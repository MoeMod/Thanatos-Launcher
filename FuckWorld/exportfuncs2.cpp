#include <metahook.h>
#include <engfuncs.h>
#include "IMEInput.h"
#include "BugReport/CrashHandle.h"

cl_enginefunc_t gEngfuncs;

bool g_bUpdateVoiceState;

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	Engfuncs_InstallHook(pEnginefuncs, iVersion);

	INEIN_InstallHook();

	InitCrashHandle();

	return gExportfuncs.Initialize(pEnginefuncs, iVersion);
}

void HUD_Init(void)
{
	return gExportfuncs.HUD_Init();
}

int HUD_Redraw(float time, int intermission)
{
	static int count = 0;

	if (count < 10)
	{
		gEngfuncs.Con_Printf("Miao World!!!!! (hit:%d)\n", count + 1);
		count++;
	}

	return gExportfuncs.HUD_Redraw(time, intermission);
}

void HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	g_bUpdateVoiceState = true;

	gExportfuncs.HUD_VoiceStatus(entindex, bTalking);

	g_bUpdateVoiceState = false;
}