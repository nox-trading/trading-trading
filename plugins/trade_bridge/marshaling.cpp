#include "marshaling.h"

#include "mt4.h"
#include "models.h"

json_t to_json(const FeedTick& tick)
{
	return json_t
	{
		{ "symbol",			tick.symbol },
		{ "bid",			tick.bid },
		{ "ask",			tick.ask },
		{ "timestamp",		tick.ctm },
	};
}

namespace mt4
{
	json_t to_json(const candle& b)
	{
		return json_t
		{
			{ "symbol",		b.symbol },
			{ "timestamp",	b.ts },
			{ "open",		b.open },
			{ "high",		b.high },
			{ "low",		b.low },
			{ "close",		b.close },
		};
	}

	NLOHMANN_JSON_SERIALIZE_ENUM(group_symbol::trade_mode, {
		{group_symbol::trade_mode::TRADE_NO, "no_trade"},
		{group_symbol::trade_mode::TRADE_CLOSE, "close_only"},
		{group_symbol::trade_mode::TRADE_FULL, "full"},
		{group_symbol::trade_mode::TRADE_LONG_ONLY, "long_only"},
	});

	json_t to_json(const group_symbol& s)
	{
		return json_t
		{
			{ "account_group",	s.account_group },
			{ "symbol",			s.symbol },
			{ "description",	s.description },
			{ "digits",			s.digits },
			{ "trade",			(group_symbol::trade_mode) s.mode },
			{ "contract_size",	s.contract_size },
			{ "tick_size",		s.tick_size },
			{ "swap_long",		s.swap_long },
			{ "swap_short",		s.swap_short },
			{ "lot_min",		s.lot_min },
			{ "lot_max",		s.lot_max },
			{ "lot_step",		s.lot_step }
		};
	}

	NLOHMANN_JSON_SERIALIZE_ENUM(trade_request::order_side, {
		{trade_request::order_side::BUY, "buy"},
		{trade_request::order_side::SELL, "sell"},
	});

	trade_request from_json(const json_t& j)
	{
		trade_request req;
		j.at("id").get_to(req.request_id);
		j.at("side").get_to(req.side);
		j.at("login").get_to(req.login);
		j.at("volume").get_to(req.volume);
		j.at("sl").get_to(req.sl);
		j.at("tp").get_to(req.tp);
		j.at("symbol").get_to(req.symbol);
		j.at("comment").get_to(req.comment);
		return req;
	}
}
