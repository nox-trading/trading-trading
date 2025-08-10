#pragma once

#include <memory>
#include <string>
#include <tl/expected.hpp>

#include <fmt/core.h>

struct CServerInterface;

template <class T, class E>
std::ostream& operator<< (std::ostream& os, tl::expected<T, E> const& ex)
{
    return os << ex.error();
}

namespace mt4
{
    class logger
    {
    public:
		using uptr_t = std::unique_ptr<logger>;

        logger(const std::string_view plugin_name, CServerInterface* server);

        template <typename... Args>
        std::string log_info(fmt::format_string<Args...> fmt_str, Args&&... args)
        {
			const std::string message = fmt::format(fmt_str, std::forward<Args>(args)...);
            log_info_impl(message);
			return message;
        }

        template<typename ... Args >
        std::string log_error(fmt::format_string<Args...> fmt_str, Args&&... args)
        {
			const std::string message = fmt::format(fmt_str, std::forward<Args>(args)...);
            log_error_impl(message);
			return message;
        }

    private:
        void log_info_impl(const std::string_view message);
        void log_error_impl(const std::string_view message);

        CServerInterface*   m_mt4server;
        std::string         m_plugin_name;
    };
}