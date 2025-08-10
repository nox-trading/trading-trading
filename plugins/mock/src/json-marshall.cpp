#include <nlohmann/json.hpp>

struct FeedTick
{
	char              symbol[16];                 // ������
	time_t            ctm;                        // �����
	char              bank[32];                   // �������� ����
	double            bid, ask;                    // ���� bid � ask
	int               feeder;                     // �������� ������
	char              reserved[8];                // ����������������� ����
};

struct ConSymbol
{
	//--- ����� ���������
	char              symbol[12];                  // ��� �������
	char              description[64];             // �������� �������
	char              source[12];                  // ������-�������� ���������
	char              currency[12];                // ��������� ������
	int               type;                        // ������ ��������
	int               digits;                      // ���������� ������ ����� ������� � ����������
	int               trade;                       // ����� ��������
	//--- ��������� �������������
	int          background_color;            // ���� ����
	int               count;                       // ������� �������
	int               count_original;              // ������� ������� � ������ �����
	int               external_unused[7];          // �������������� ����
	//--- ������
	int               realtime;                    // ���������� ��������� � �������� �������
	time_t            starting;                    // ���� ������ �������� �� �������
	time_t            expiration;                  // ���� ��������� �������� �� �������
	int       sessions[7];                 // ������������ � �������� ������
	//--- ������ �������
	int               profit_mode;                 // ����� ������� �������
	int               profit_reserved;             // ����������������� ����
	//--- ���������� ���������
	int               filter;                      // ������� ����������
	int               filter_counter;              // ���������� ��������� ��� ������������� ������ ������
	double            filter_limit;                // ������������ ���������� ���������� �� ���������� ����
	int               filter_smoothing;            // ����������� ��� � �����
	float             filter_reserved;             // ����������������� ����
	int               logging;                     // ��������� ������ ����� � ����
	//--- ������ � �����
	int               spread;                      // �����
	int               spread_balance;              // ������ ������
	int               exemode;                     // ����� ����������
	int               swap_enable;                 // ��������� ������
	int               swap_type;                   // ��� ���������� ������
	double            swap_long, swap_short;        // ������ ����� ��� ������� � �������� �������
	int               swap_rollover3days;          // ���� �������� �����
	double            contract_size;               // ������ ���������
	double            tick_value;                  // ��������� ����
	double            tick_size;                   // ������ ����
	int               stops_level;                 // ������� ������
	int               gtc_pendings;                // ����� ���������
	//--- ������ �����
	int               margin_mode;                 // ����� ������� �����
	double            margin_initial;              // ��������� �����
	double            margin_maintenance;          // �������������� �����
	double            margin_hedged;               // ������������� �����
	double            margin_divider;              // ����������� �����
	//--- ��������� ������ (��� ����������� �������������)
	double            point;                       // ������ ������
	double            multiply;                    // ��������� ��� ��������� ���������� ������ �� ������� ������
	double            bid_tickvalue;               // ��������� ���� ��� ������� ������� � ������ ��������
	double            ask_tickvalue;               // ��������� ���� ��� �������� ������� � ������ ��������
	//---
	int               long_only;                   // ���������� ������ ������� �������
	int               instant_max_volume;          // ������������ ����� ��� ������������ ����������
	//---
	char              margin_currency[12];         // ������ ������� �����
	int               freeze_level;                // ������� ���������
	int               margin_hedged_strong;        // ������� �������� ����� ��� ����������� �������
	time_t            value_date;                  // ���� �������������, �� ������������
	int               quotes_delay;                // �������� ��������� ����� ������ ������
	int               swap_openprice;              // ������������ ���� ������������ ��� ������� ��������� ������� ��� SWAP_BY_INTEREST
	int               swap_variation_margin;       // ��������� ������������ ����� ��� ���������
	//---
	int               unused[21];                  // ����������������� ����
};

void to_json(nlohmann::json& j, const FeedTick& t)
{
	j = nlohmann::json{
		{"symbol", t.symbol},
		{"ts", t.ctm},
		{"bid", t.bid},
		{"ask", t.ask},
	};
}

void to_json(nlohmann::json& j, const ConSymbol& symbol)
{
    j = nlohmann::json{
        {"symbol", symbol.symbol},
        {"description", symbol.description},
        {"source", symbol.source},
        {"currency", symbol.currency},
        {"type", symbol.type},
        {"digits", symbol.digits},
        {"trade", symbol.trade},
        {"background_color", symbol.background_color},
        {"count", symbol.count},
        {"count_original", symbol.count_original},
        {"external_unused", std::vector<int>(symbol.external_unused, symbol.external_unused + 7)},
        {"realtime", symbol.realtime},
        {"starting", symbol.starting},
        {"expiration", symbol.expiration},
        {"sessions", std::vector<int>(symbol.sessions, symbol.sessions + 7)},
        {"profit_mode", symbol.profit_mode},
        {"profit_reserved", symbol.profit_reserved},
        {"filter", symbol.filter},
        {"filter_counter", symbol.filter_counter},
        {"filter_limit", symbol.filter_limit},
        {"filter_smoothing", symbol.filter_smoothing},
        {"filter_reserved", symbol.filter_reserved},
        {"logging", symbol.logging},
        {"spread", symbol.spread},
        {"spread_balance", symbol.spread_balance},
        {"exemode", symbol.exemode},
        {"swap_enable", symbol.swap_enable},
        {"swap_type", symbol.swap_type},
        {"swap_long", symbol.swap_long},
        {"swap_short", symbol.swap_short},
        {"swap_rollover3days", symbol.swap_rollover3days},
        {"contract_size", symbol.contract_size},
        {"tick_value", symbol.tick_value},
        {"tick_size", symbol.tick_size},
        {"stops_level", symbol.stops_level},
        {"gtc_pendings", symbol.gtc_pendings},
        {"margin_mode", symbol.margin_mode},
        {"margin_initial", symbol.margin_initial},
        {"margin_maintenance", symbol.margin_maintenance},
        {"margin_hedged", symbol.margin_hedged},
        {"margin_divider", symbol.margin_divider},
        {"point", symbol.point},
        {"multiply", symbol.multiply},
        {"bid_tickvalue", symbol.bid_tickvalue},
        {"ask_tickvalue", symbol.ask_tickvalue},
        {"long_only", symbol.long_only},
        {"instant_max_volume", symbol.instant_max_volume},
        {"margin_currency", symbol.margin_currency},
        {"freeze_level", symbol.freeze_level},
        {"margin_hedged_strong", symbol.margin_hedged_strong},
        {"value_date", symbol.value_date},
        {"quotes_delay", symbol.quotes_delay},
        {"swap_openprice", symbol.swap_openprice},
        {"swap_variation_margin", symbol.swap_variation_margin},
        {"unused", std::vector<int>(symbol.unused, symbol.unused + 21)}
    };
}
