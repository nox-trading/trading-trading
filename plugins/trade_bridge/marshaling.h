#pragma once

#include <nlohmann/json.hpp>

using json_t = nlohmann::json;

struct ConSymbol;
struct FeedTick;

namespace mt4
{
	struct candle;
	json_t to_json(const candle&);

	struct group_symbol;
	json_t to_json(const group_symbol&);
}

json_t to_json(const FeedTick&);
