#pragma once

#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

namespace json
{
	using type = nlohmann::json;

	class marshaler
	{
	public:
		template<typename T>
		static std::string marshal(T&& obj)
		{
			auto j = to_json(std::forward<T>(obj));
			return j.dump();
		}

        template<typename T>
        static tl::expected<std::decay_t<T>, std::string>unmarshal(std::string_view sv)
        {
            type j = type::parse(sv, nullptr, false);
            if (j.is_discarded())
            {
                return tl::unexpected{ "invalid json" };
            }

            try
            {
                using U = std::decay_t<T>;
                return j.template get<U>();
            }
            catch (...)
            {
                return tl::unexpected{ "invalid message" };
            }
        }
	};
}