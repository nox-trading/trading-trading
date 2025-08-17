#pragma once

#include <memory>
#include <filesystem>

#include <BS_thread_pool.hpp>
#include <tl/expected.hpp>

#include "logger.h"
#include "json.h"
#include "nats.h"
#include "marshaling.h"

struct CServerInterface;
struct ConGroup;
struct ConGroupMargin;
struct ConSymbol;
struct FeedTick;

namespace mt4
{
	class plugin
	{
		using pool_t = BS::thread_pool<BS::tp::priority | BS::tp::pause>;
		struct thread_pool_deleter
		{
			void operator() (pool_t* pool) const noexcept
			{
				if (pool)
				{
					pool->pause();
					pool->wait_for(std::chrono::seconds{ 1 });
					delete pool;
				}
			}
		};
		using pool_ptr_t = std::unique_ptr<pool_t, thread_pool_deleter>;

	public:
		using uptr_t = std::unique_ptr<plugin>;

		static tl::expected<plugin::uptr_t, std::string> initialize(CServerInterface* mt4server, const std::string_view plugin_name);

		void handle(const FeedTick* tick);
		void handle(const ConSymbol* symbol);
		void handle(const ConGroup* group);

		void publish_chart();
		void publish_all_groups_with_symbols();

	private:
		plugin(
			const std::string_view plugin_name,
			const std::string_view server_name,
			const std::string_view nats_url,
			CServerInterface* mt4server,
			size_t pool_size
		) noexcept;

		tl::expected<void, std::string> connect_to_nats(const std::string_view nats_url);
		tl::expected<void, std::string> nats_subscribe_to_trade_request(const std::string_view server_name);

		void on_trade_request(trade_request& request);

		void publish_chart_by_symbol(const ConSymbol& symbol, int period);

		void publish_group_symbols(const ConGroup& group);
		void publish_symbol_for_groups(const ConSymbol& symbol);

		bool should_stop_task(int iteration);

		std::string 					m_plugin_name;
		CServerInterface*				m_mt4server;
		logger 							m_logger;

		nats::server<json::marshaler>	m_nats_conn;
		pool_ptr_t						m_pool;

		const std::string				m_topic_name_feed_tick;
		const std::string				m_topic_name_con_symbol;
		const std::string				m_topic_name_mt4_candle;

		const std::filesystem::path		m_chart_timepoint_dir;
	};
}