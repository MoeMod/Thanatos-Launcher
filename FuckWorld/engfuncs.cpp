#include <metahook.h>
#include <plugins.h>
#include <exportfuncs.h>
#include <cvardef.h>
#include <Encode.h>
#include <com_model.h>
#include "ViewPort.h"

extern cl_enginefuncs_s *engine;

model_t *g_MapSprite;

struct model_s *Engfuncs_LoadMapSprite(const char *filename)
{
	static char lastname[MAX_PATH];

	if (g_MapSprite && !strcmp(lastname, filename))
		return g_MapSprite;

	g_MapSprite = gEngfuncs.LoadMapSprite(filename);

	g_pViewPort->UpdateMapSprite();

	strcpy(lastname, filename);
	return g_MapSprite;
}

void Engfuncs_GetPlayerInfo(int ent_num, hud_player_info_t *pinfo)
{
	static hud_player_info_t info;
	g_pMetaSave->pEngineFuncs->pfnGetPlayerInfo(ent_num, &info);

	pinfo->name = info.name;
	pinfo->ping = info.ping;
	pinfo->thisplayer = info.thisplayer;
	pinfo->spectator = info.spectator;
	pinfo->packetloss = info.packetloss;
	pinfo->model = info.model;
	pinfo->topcolor = info.topcolor;
	pinfo->bottomcolor = info.bottomcolor;

	if (g_dwEngineBuildnum >= 5953)
		pinfo->m_nSteamID = info.m_nSteamID;
}

void Engfuncs_SetCrosshair(HSPRITE hspr, wrect_t rc, int r, int g, int b)
{
}

int Engfuncs_AddCommand(char *cmd_name, void(*pfnEngSrc_function)(void))
{
	return g_pMetaSave->pEngineFuncs->pfnAddCommand(cmd_name, pfnEngSrc_function);
}

int Engfuncs_GetScreenInfo(SCREENINFO *pscrinfo)
{
	static bool initialize = false;
	static SCREENINFO scrinfo;

	if (!initialize)
	{
		memset(&scrinfo, 0, sizeof(scrinfo));
		scrinfo.iSize = sizeof(SCREENINFO);

		if (g_pMetaSave->pEngineFuncs->pfnGetScreenInfo(&scrinfo))
		{
			initialize = true;
		}
	}

	if (pscrinfo->iSize == sizeof(SCREENINFO))
	{
		pscrinfo->iFlags = scrinfo.iFlags;
		pscrinfo->iCharHeight = scrinfo.iCharHeight;
		pscrinfo->iWidth = g_iVideoWidth;
		pscrinfo->iHeight = g_iVideoHeight;
		return 1;
	}

	return 0;
}

void Engfuncs_InstallHook(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	engine = pEnginefuncs;
	pEnginefuncs->LoadMapSprite = Engfuncs_LoadMapSprite;
	pEnginefuncs->pfnGetPlayerInfo = Engfuncs_GetPlayerInfo;
	pEnginefuncs->pfnSetCrosshair = Engfuncs_SetCrosshair;
	pEnginefuncs->pfnAddCommand = Engfuncs_AddCommand;
	pEnginefuncs->pfnGetScreenInfo = Engfuncs_GetScreenInfo;
}