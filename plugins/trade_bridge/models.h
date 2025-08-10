#pragma once

#include <string>

namespace mt4
{
	struct candle
	{
		std::string symbol;
		int			period;

		int32_t timestamp;
		double open;
		double high;
		double low;
		double close;
	};

	struct group_symbol
	{
		enum trade_mode
		{
			TRADE_NO,
			TRADE_CLOSE,
			TRADE_FULL,
			TRADE_LONG_ONLY
		};

		std::string		account_group;
		std::string		symbol;
		std::string		description;
		int				digits;
		trade_mode		mode;
		double			contract_size;
		double			tick_size;
		double			swap_long;
		double			swap_short;
		int             lot_min;
		int				lot_max;
		int             lot_step;
	};
}