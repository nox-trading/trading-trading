#include <nlohmann/json.hpp>

struct FeedTick
{
	char              symbol[16];                 // символ
	time_t            ctm;                        // время
	char              bank[32];                   // источник цены
	double            bid, ask;                    // цены bid и ask
	int               feeder;                     // источник данных
	char              reserved[8];                // зарезервированное поле
};

struct ConSymbol
{
	//--- общие настройки
	char              symbol[12];                  // имя символа
	char              description[64];             // описание символа
	char              source[12];                  // символ-источник котировок
	char              currency[12];                // расчетная валюта
	int               type;                        // группа символов
	int               digits;                      // количество знаков после запятой в котировках
	int               trade;                       // режим торговли
	//--- параметры представления
	int          background_color;            // цвет фона
	int               count;                       // позиция символа
	int               count_original;              // позиция символа в Обзоре рынка
	int               external_unused[7];          // неиспользуемое поле
	//--- сессии
	int               realtime;                    // разрешение котировок в реальном времени
	time_t            starting;                    // дата начала торговли по символу
	time_t            expiration;                  // дата окончания торговли по символу
	int       sessions[7];                 // котировочные и торговые сессии
	//--- расчет прибыли
	int               profit_mode;                 // режим расчета прибыли
	int               profit_reserved;             // зарезервированное поле
	//--- фильтрация котировок
	int               filter;                      // уровень фильтрации
	int               filter_counter;              // количество котировок для подтверждения нового уровня
	double            filter_limit;                // максимальное допустимое отклонение от предыдущей цены
	int               filter_smoothing;            // сглаживание цен в тиках
	float             filter_reserved;             // зарезервированное поле
	int               logging;                     // разрешить запись тиков в файл
	//--- спреды и свопы
	int               spread;                      // спред
	int               spread_balance;              // баланс спреда
	int               exemode;                     // режим исполнения
	int               swap_enable;                 // включение свопов
	int               swap_type;                   // тип начисления свопов
	double            swap_long, swap_short;        // размер свопа для длинных и коротких позиций
	int               swap_rollover3days;          // день тройного свопа
	double            contract_size;               // размер контракта
	double            tick_value;                  // стоимость тика
	double            tick_size;                   // размер тика
	int               stops_level;                 // уровень стопов
	int               gtc_pendings;                // режим истечения
	//--- расчет маржи
	int               margin_mode;                 // режим расчета маржи
	double            margin_initial;              // начальная маржа
	double            margin_maintenance;          // поддерживающая маржа
	double            margin_hedged;               // хеджированная маржа
	double            margin_divider;              // коэффициент маржи
	//--- расчетные данные (для внутреннего использования)
	double            point;                       // размер пункта
	double            multiply;                    // множитель для получения количества знаков из размера пункта
	double            bid_tickvalue;               // стоимость тика для длинной позиции в валюте депозита
	double            ask_tickvalue;               // стоимость тика для короткой позиции в валюте депозита
	//---
	int               long_only;                   // разрешение только длинных позиций
	int               instant_max_volume;          // максимальный объем для немедленного исполнения
	//---
	char              margin_currency[12];         // валюта расчета маржи
	int               freeze_level;                // уровень заморозки
	int               margin_hedged_strong;        // строгая проверка маржи для хеджирующих позиций
	time_t            value_date;                  // дата валютирования, не используется
	int               quotes_delay;                // задержка котировок после начала сессии
	int               swap_openprice;              // использовать цену переоткрытия при расчете стоимости позиций при SWAP_BY_INTEREST
	int               swap_variation_margin;       // начислять вариационную маржу при ролловере
	//---
	int               unused[21];                  // зарезервированное поле
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
