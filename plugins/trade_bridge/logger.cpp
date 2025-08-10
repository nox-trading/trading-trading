#include "logger.h"

#include "mt4.h"

namespace mt4
{
    logger::logger(const std::string_view plugin_name, CServerInterface* server)
        : m_mt4server(server)
        , m_plugin_name(plugin_name)
    {
    }

    void logger::log_info_impl(const std::string_view message)
    {
        if (m_mt4server != nullptr)
        {
            m_mt4server->LogsOut(CmdOK, m_plugin_name.data(), message.data());
        }
    }

    void logger::log_error_impl(const std::string_view message)
    {
        if (m_mt4server != nullptr)
        {
            m_mt4server->LogsOut(CmdErr, m_plugin_name.data(), message.data());
        }
    }
} 