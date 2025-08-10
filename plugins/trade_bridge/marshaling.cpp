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
			{ "period",		b.period },

			{ "timestamp",	b.timestamp },
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
	})

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
}
