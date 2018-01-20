#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "tri.h"

extern pfnUserMsgHook g_pfnMSG_ResetHUD;
extern pfnUserMsgHook g_pfnMSG_InitHUD;
extern pfnUserMsgHook g_pfnMSG_ViewMode;

int CHud::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	int result = g_pfnMSG_ResetHUD(pszName, iSize, pbuf);

	for (auto p : m_HudList)
	{
		p->Reset();
	}

	m_flMouseSensitivity = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
		g_PlayerScoreAttrib[i] = 0;

	g_pViewPort->Reset();

	return result;
}

void CAM_ToFirstPerson(void);

int CHud::MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	CAM_ToFirstPerson();
	return 1;
}

int CHud::MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	int result = g_pfnMSG_InitHUD(pszName, iSize, pbuf);

	for (auto p : m_HudList)
	{
		p->InitHUDData();
	}

	g_iFreezeTimeOver = 0;

	g_FogParameters.density = 0;
	g_FogParameters.affectsSkyBox = false;
	g_FogParameters.color[0] = 0;
	g_FogParameters.color[1] = 0;
	g_FogParameters.color[2] = 0;

	if (cl_fog_skybox)
		gEngfuncs.Cvar_SetValue(cl_fog_skybox->name, g_FogParameters.affectsSkyBox);

	if (cl_fog_density)
		gEngfuncs.Cvar_SetValue(cl_fog_density->name, g_FogParameters.density);

	if (cl_fog_r)
		gEngfuncs.Cvar_SetValue(cl_fog_r->name, g_FogParameters.color[0]);

	if (cl_fog_g)
		gEngfuncs.Cvar_SetValue(cl_fog_g->name, g_FogParameters.color[1]);

	if (cl_fog_b)
		gEngfuncs.Cvar_SetValue(cl_fog_b->name, g_FogParameters.color[2]);

	return 1;
}