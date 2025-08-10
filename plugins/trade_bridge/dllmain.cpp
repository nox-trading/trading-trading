#include <memory>

#include "plugin.h"
#include "logger.h"

#include "mt4.h"

const PluginInfo plugin_info { "mt4api", 100, "", {0}, };
CServerInterface* mt4server{ nullptr };

mt4::logger::uptr_t mt4logger { nullptr };
mt4::plugin::uptr_t mt4plugin { nullptr };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    char tmp[256], * cp;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 1);
        if ((cp = strrchr(tmp, '.')) != NULL) *cp = 0;
        strcat(tmp, ".ini");
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void APIENTRY MtSrvAbout(PluginInfo* info)
{
    if (info != NULL)
    {
        memcpy(info, &plugin_info, sizeof(PluginInfo));
    }
}

int APIENTRY MtSrvStartup(CServerInterface* server)
{
//#ifdef _DEBUG
//	std::this_thread::sleep_for(std::chrono::seconds{ 5 });
//#endif

    if (server != nullptr)
    {
        mt4server = server;

		mt4logger = std::make_unique<mt4::logger>(plugin_info.name, mt4server);
        if (server->Version() != ServerApiVersion)
        {
            mt4logger->log_error("Server API version mismatch: expected {}, got {}", ServerApiVersion, server->Version());
            return(FALSE);
        }

        mt4plugin = std::make_unique<mt4::plugin>(
            plugin_info.name,
            "test1",
            "nats://localhost:4222",
            mt4server,
            2 // thread pool size
        );
		//mt4plugin->publish_chart(0);

        return(TRUE);
    }
	return(FALSE);
}

void APIENTRY MtSrvCleanup()
{
	mt4logger->log_info("Unloading plugin, cleaning up resources");
    mt4plugin.reset();
    mt4logger.reset();
	mt4server = nullptr;
}

int APIENTRY MtSrvGroupsAdd(const ConGroup* group)
{
    if (mt4plugin)
    {
        mt4plugin->handle(group);
    }
    return (TRUE);
}

int APIENTRY MtSrvSymbolsAdd(const ConSymbol* symbol)
{
    if (mt4plugin)
    {
        mt4plugin->handle(symbol);
    }
    return (TRUE);
}

void APIENTRY MtSrvHistoryTickApply(const ConSymbol*, FeedTick* tick)
{
    if (mt4plugin)
    {
        mt4plugin->handle(tick);
    }
}