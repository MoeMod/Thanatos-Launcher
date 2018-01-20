#include <metahook.h>
#include <cvardef.h>
#include <common.h>
#include <cmd.h>

char g_szVersion[] = __DATE__;

cvar_t *g_pcvarVersion;

void Version_Response()
{
}

void Version_Init()
{
	g_pcvarVersion = gEngfuncs.pfnRegisterVariable("tz_version", g_szVersion, 0);

	gEngfuncs.pfnAddCommand("tz_versionsync", Version_Response);

	gEngfuncs.PlayerInfo_SetValueForKey("tz_version", g_szVersion);
	gEngfuncs.pfnClientCmd(va("setinfo \"tz_version\" \"%s\"", g_szVersion));
}

void Version_Shutdown()
{
	gEngfuncs.PlayerInfo_SetValueForKey("tz_version", "");
	gEngfuncs.pfnClientCmd("setinfo \"tz_version\" \"\"");
}