#include <metahook.h>
#include "plugins.h"
#include "SteamAPI.h"

HMODULE g_hSteamAPI;
ISteamApps *(*g_pfnSteamApps)(void);
int (*g_pfnSteamAPI_Init)(void);

ISteamApps *SteamApps(void)
{
	if (!g_pfnSteamApps)
		return NULL;

	return g_pfnSteamApps();
}

int SteamAPI_Init(void)
{
	return g_pfnSteamAPI_Init();
}

bool SteamAPI_InitSteamApps(void)
{
	if (g_pfnSteamApps)
		return true;

	g_hSteamAPI = GetModuleHandle("steam_api.dll");
	g_pfnSteamApps = (ISteamApps *(*)(void))GetProcAddress(g_hSteamAPI, "SteamApps");

	if (g_pfnSteamApps)
		return true;

	return false;
}

void SteamAPI_InstallHook(void)
{
	if (!g_bIsUseSteam)
		return;

	if (SteamAPI_InitSteamApps())
		return;

	g_pMetaHookAPI->IATHook(g_hEngineModule, "steam_api.dll", "SteamAPI_Init", SteamAPI_Init, (void *&)g_pfnSteamAPI_Init);
}