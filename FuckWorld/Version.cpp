#include <metahook.h>
#include <cvardef.h>
#include <common.h>
#include <cmd.h>

char g_szVersion[] = __DATE__;

void Version_Init()
{
	gEngfuncs.PlayerInfo_SetValueForKey("tz_version", g_szVersion);
	gEngfuncs.pfnClientCmd(va("setinfo \"tz_version\" \"%s\"", g_szVersion));
}

void Version_Shutdown()
{
	gEngfuncs.PlayerInfo_SetValueForKey("tz_version", "");
	gEngfuncs.pfnClientCmd("setinfo \"tz_version\" \"\"");
}