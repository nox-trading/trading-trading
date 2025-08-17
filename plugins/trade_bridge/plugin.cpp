#include "plugin.h"

#include <unordered_set>
#include <thread>
#include <fstream>
#include <filesystem>

#include "models.h"
#include "tools.h"
#include "ini.h"

#include "mt4.h"

namespace
{
	const auto ini_file = "./mt4api.ini";
	
	const auto chart_periods = { PERIOD_M1, PERIOD_M5, PERIOD_M15, PERIOD_M30, PERIOD_H1, PERIOD_H4, PERIOD_D1, PERIOD_W1, PERIOD_MN1 };

	static const int sleep_iteration_count = 64; // Number of iterations before sleep

	tl::expected<mt4::group_symbol, bool> make_group_symbol(const ConGroup& group, const ConSymbol& symbol, const ConGroupMargin* symbol_margin_sec)
	{
		const auto symbol_sec_group = &group.secgroups[symbol.type];
		if (symbol_sec_group->show == 0)
		{
			return tl::unexpected{ false };
		}

		auto symbol_trade_mode = static_cast<mt4::group_symbol::trade_mode>(symbol.trade);
		if (symbol.long_only)
		{
			symbol_trade_mode = mt4::group_symbol::TRADE_LONG_ONLY;
		}
		if (symbol_sec_group->trade == 0)
		{
			symbol_trade_mode = mt4::group_symbol::TRADE_NO;
		}
		return mt4::group_symbol{
			.account_group = group.group,
			.symbol = symbol.symbol,
			.description = symbol.description,
			.digits = symbol.digits,
			.mode = symbol_trade_mode,
			.contract_size = symbol.contract_size,
			.tick_size = symbol.tick_size,
			.swap_long = symbol_margin_sec != nullptr ? symbol_margin_sec->swap_long : symbol.swap_long,
			.swap_short = symbol_margin_sec != nullptr ? symbol_margin_sec->swap_short : symbol.swap_short,
			.lot_min = symbol_sec_group->lot_min,
			.lot_max = symbol_sec_group->lot_max,
			.lot_step = symbol_sec_group->lot_step
		};
	}

	std::filesystem::path get_file_path(const std::filesystem::path& path, const std::string_view symbol, int period)
	{
		return path / fmt::format("{}{}.timepoint", symbol, period);
	}

	void save_chart_point(const std::filesystem::path& path, const std::string_view symbol, time_t timestamp, int period)
	{
		auto file_path = get_file_path(path, symbol, period);
		std::ofstream out(file_path, std::ios::binary | std::ios::trunc);
		out.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
	}

	time_t load_chart_point(const std::filesystem::path& path, const std::string_view symbol, int period)
	{
		auto file_path = get_file_path(path, symbol, period);
		std::ifstream in(file_path, std::ios::binary);
		time_t value = 0;
		in.read(reinterpret_cast<char*>(&value), sizeof(value));
		return value;
	}

	struct config
	{
		std::string		server_name;
		std::string		nats_url;
		size_t			pool_size;
		time_t			last_chart_sync_time;
	};
}

namespace mt4
{
    tl::expected<plugin::uptr_t, std::string> plugin::initialize(CServerInterface* mt4server, const std::string_view plugin_name)
    {
		config cfg {};
		try
		{
			if (!ini::load_ini<config>(ini_file, cfg))
			{
				throw std::runtime_error(fmt::format("Failed to load configuration from {}", ini_file));
			}
		}
		catch (const std::exception& ex)
		{
			return tl::unexpected{ fmt::format("Failed to load configuration from {}: {}", ini_file, ex.what()) };
		}
		catch (...)
		{
			return tl::unexpected{ fmt::format("Failed to load configuration from {}: unknown error", ini_file) };
		}

		if (!cfg.nats_url.empty())
		{
			return tl::unexpected{ "NATS URL is not configured in the ini file" };
		}
		if (cfg.pool_size == 0)
		{
			cfg.pool_size = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 2;
		}
		if (cfg.server_name.empty())
		{
			return tl::unexpected{ "Server name is not configured in the ini file" };
		}

		return std::make_unique<plugin>(
			plugin_name,
			cfg.server_name,
			cfg.nats_url,
			mt4server,
			cfg.pool_size
		);
    }

	plugin::plugin(
		const std::string_view plugin_name,
		const std::string_view server_name,
		const std::string_view nats_url,
		CServerInterface* mt4server, 
		size_t pool_size
	) noexcept
		: m_plugin_name{ plugin_name }
		, m_mt4server{ mt4server }
		, m_pool{ new pool_t{ pool_size }, thread_pool_deleter{} }
		, m_logger{ plugin_name, mt4server }

		, m_topic_name_feed_tick{ std::string(server_name) + ".mt4_tick" }
		, m_topic_name_con_symbol{ std::string(server_name) + ".mt4_symbol" }
		, m_topic_name_mt4_candle{ std::string(server_name) + ".mt4_candle" }

		, m_chart_timepoint_dir{ "./charts/" }
	{
		if (auto result = connect_to_nats(nats_url); !result)
		{
			m_logger.log_error("Failed to connect to NATS: {}", result.error());
			return;
		}
		if (auto result = nats_subscribe_to_trade_request(server_name); !result)
		{
			m_logger.log_error("Failed to subscribe to trade request: {}", result.error());
			return;
		}

	}

	tl::expected<void, std::string> plugin::connect_to_nats(const std::string_view url)
	{
		return m_nats_conn.connect(url);
	}

	tl::expected<void, std::string> plugin::nats_subscribe_to_trade_request(const std::string_view server_name)
	{
		const auto topic_name = std::string(server_name) + ".trade.request";
		const auto read_next_message = m_nats_conn.subscribe_sync<trade_request>(topic_name);
		if (!read_next_message)
		{
			return tl::unexpected<std::string>(read_next_message.error());
		}
		m_pool->detach_task([this, read_next_message = *read_next_message]() mutable {
			while (true)
			{
				if (auto message = read_next_message(); message)
				{
					on_trade_request(*message);
				}
				else
				{
					auto error_wrapper = message.error();
					if (!error_wrapper)
					{
						m_logger.log_error("Failed to receive trade request: {}", error_wrapper.error());
					}
				}
			}
		}, BS::pr::high);
		return {};
	}

	void plugin::on_trade_request(trade_request& request)
	{
		m_logger.log_info("Received trade request with ID: {}", request.request_id);
	}

	void plugin::handle(const FeedTick* tick)
	{
		if (tick != nullptr)
		{
			if (auto status = m_nats_conn.publish(m_topic_name_feed_tick, *tick); !status)
			{
				m_logger.log_error("Failed to publish feed tick: {}", status.error());
			}
		}
	}

	void plugin::handle(const ConSymbol* symbol)
	{
		if (symbol != nullptr)
		{
			publish_symbol_for_groups(*symbol);
		}
	}

	void plugin::handle(const ConGroup* group)
	{
		if (group != nullptr)
		{
			publish_group_symbols(*group);
		}
	}

	void plugin::publish_chart_by_symbol(const ConSymbol& symbol, int period)
	{
		m_pool->detach_task([this, symbol_name = std::string{ symbol.symbol }, digits = symbol.digits, period]()
		{
			int count{ 0 };
			RateInfo* rates = m_mt4server->HistoryQuotes(symbol_name.data(), period, &count);
			if (rates != nullptr && count > 0)
			{
				int published_count{ 0 };
				const auto from_time = load_chart_point(m_chart_timepoint_dir, symbol_name, period);

				//m_logger.log_info("Publishing chart for symbol: {}, period: {}", symbol_name, period);

				for (int i = count - 1; i >= 0; --i)
				{
					const auto& rate = rates[i];
					if (rate.ctm < from_time)
					{
						return;
					}
					if (should_stop_task(i)) return;

					if (auto status = m_nats_conn.publish(m_topic_name_mt4_candle,
						candle{
							.symbol = symbol_name,
							.ts = rate.ctm,
							.open = tools::int_price_to_double(rate.open, digits),
							.high = tools::int_price_to_double(rate.high, digits),
							.low = tools::int_price_to_double(rate.low, digits),
							.close = tools::int_price_to_double(rate.close, digits)
						}
					); !status)
					{
						m_logger.log_error("Failed to publish chart candle for symbol: {}, period: {}: {}", symbol_name, period, status.error());
					}
					else
					{
						++published_count;
					}
				}

				save_chart_point(m_chart_timepoint_dir, symbol_name, rates[count-1].ctm, period);
			}

			//m_logger.log_info("Finished publishing chart for symbol: {} period: {} candles published: {}", symbol_name, period, published_count);
		}, BS::pr::low);
	}

	void plugin::publish_chart()
	{
		ConSymbol symbol{};
		for (int i = 0; m_mt4server->SymbolsNext(i, &symbol); ++i)
		{
			publish_chart_by_symbol(symbol, PERIOD_M1);
		}
	}

	void plugin::publish_group_symbols(const ConGroup& group)
	{
		m_pool->detach_task([this, group]()
		{
			m_logger.log_info("Publishing symbols for group: {}", group.group);

			std::unordered_set<std::string> published_symbols{};
			for (int i = 0; i < group.secmargins_total; ++i)
			{
				if (should_stop_task(i)) return;

				const auto symbol_margin_sec = &group.secmargins[i];
				ConSymbol symbol{};
				if (m_mt4server->SymbolsGet(symbol_margin_sec->symbol, &symbol))
				{
					if (auto result = make_group_symbol(group, symbol, symbol_margin_sec); result)
					{
						if (auto status = m_nats_conn.publish(m_topic_name_con_symbol, *result); !status)
						{
							m_logger.log_error("Failed to publish symbol for group '{}': {}", group.group, status.error());
						}
					}
					published_symbols.emplace(symbol.symbol);
				}
			}

			ConSymbol symbol{};
			for (int i = 0; m_mt4server->SymbolsNext(i, &symbol); ++i)
			{
				if (should_stop_task(i)) return;

				if (auto result = make_group_symbol(group, symbol, nullptr); result)
				{
					if (auto status = m_nats_conn.publish(m_topic_name_con_symbol, *result); !status)
					{
						m_logger.log_error("Failed to publish symbol for group '{}': {}", group.group, status.error());
					}
				}
			}
		}, BS::pr::low);
	}

	void plugin::publish_symbol_for_groups(const ConSymbol& symbol)
	{
		m_pool->detach_task([this, symbol]()
			{
				m_logger.log_info("Publishing symbol for groups: {}", symbol.symbol);

				ConGroup group{};
				for (int i = 0; m_mt4server->GroupsNext(i, &group); ++i)
				{
					if (should_stop_task(i)) return;

					ConGroupMargin* symbol_margin_sec { nullptr };
					for (int j = 0; j < group.secmargins_total; ++j)
					{
						const auto margin_sec = &group.secmargins[j];
						if (std::string_view{ margin_sec->symbol } == symbol.symbol) // strcmp_s ? no way
						{
							symbol_margin_sec = margin_sec;
							break;
						}
					}

					if (auto result = make_group_symbol(group, symbol, symbol_margin_sec); result)
					{
						if (auto status = m_nats_conn.publish(m_topic_name_con_symbol, *result); !status)
						{
							m_logger.log_error("Failed to publish symbol for group '{}': {}", group.group, status.error());
						}
					}
				}
			}, BS::pr::low);
	}

	void plugin::publish_all_groups_with_symbols()
	{
		ConGroup group{};
		for (int i = 0; m_mt4server->GroupsNext(i, &group); ++i)
		{
			publish_group_symbols(group);
		}
	}

	bool plugin::should_stop_task(int iteration)
	{
		if (iteration % sleep_iteration_count == 0)
		{
			if (m_pool->is_paused())
			{
				return true;
			}
			/*else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
			}*/
		}
		return false;
	}
}