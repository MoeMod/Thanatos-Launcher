#include "hud.h"
#include "player.h"
#include "com_model.h"
#include "CVARs.h"
#include "Encode.h"
#include "cl_util.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <IEngineSurface.h>
#include <VGUI/VGUI2.h>
#include <VGUI/ISurface.h>
#include <VGUI/IScheme.h>
#include "plugins.h"
#include "calcscreen.h"
#include "client.h"
#include "parsemsg.h"

int CHudHeadName::Init(void)
{
	gHUD.AddHudElem(this);

	return TRUE;
}

int CHudHeadName::VidInit(void)
{
	m_iFlags |= HUD_ACTIVE;

	if (!m_hHeadFont)
	{
		vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("ClientScheme"));

		if (pScheme)
		{
			m_hHeadFont = pScheme->GetFont("CreditsText");
			pScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetDefaultScheme());

			if (!m_hHeadFont)
				m_hHeadFont = pScheme->GetFont("CreditsFont");
		}
	}

	return TRUE;
}

void CHudHeadName::Reset(void)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_sUnicodes[i][0] = 0;
}

void CHudHeadName::BuildUnicodeList(void)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0])
		{
			m_sUnicodes[i][0] = 0;
			continue;
		}

		wcscpy(m_sUnicodes[i], UTF8ToUnicode(g_PlayerInfoList[i].name));
	}
}

BOOL CHudHeadName::IsValidEntity(cl_entity_s *pEntity)
{
	bool bNotInPVS = (abs(gEngfuncs.GetLocalPlayer()->curstate.messagenum - pEntity->curstate.messagenum) > 15);

	if (pEntity && pEntity->model && pEntity->model->name && !bNotInPVS)
		return TRUE;

	return FALSE;
}

int CHudHeadName::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL) || g_iUser1 || !cl_headname->value)
		return TRUE;

	if (gHUD.m_flTime > m_flNextBuild)
	{
		BuildUnicodeList();
		m_flNextBuild = gHUD.m_flTime + 1.0;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_sUnicodes[i][0])
		{
			if (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0])
				continue;

			wcscpy(m_sUnicodes[i], UTF8ToUnicode(g_PlayerInfoList[i].name));
		}

		if (g_PlayerExtraInfo[i].dead)
			continue;

		if (g_PlayerExtraInfo[i].teamnumber != g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamnumber)
			continue;

		if (i != gHUD.m_iPlayerNum)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);

			if (!IsValidEntity(ent))
				continue;

			model_t *model = ent->model;
			vec3_t origin = ent->origin;

			if (model)
				origin.z += max(model->maxs.z, 35.0);
			else
				origin.z += 35.0;

			float screenPos[2];

			int iResult = gEngfuncs.pTriAPI->WorldToScreen(origin, screenPos);

			if (!(screenPos[0] < 1 && screenPos[1] < 1 && screenPos[0] > -1 && screenPos [1] > -1 && !iResult))
				continue;

			int w, t;
			vgui2::surface()->GetScreenSize(w, t);
			w /= 2;
			t /= 2;
			screenPos[0] = screenPos[0] * w + w;
			screenPos[1] = -screenPos[1] * t + t;

			int textWide, textTall;
			vgui2::surface()->GetTextSize(m_hHeadFont, m_sUnicodes[i], textWide, textTall);
			vgui2::surface()->DrawSetTextPos(screenPos[0] - ((textWide) / 2), screenPos[1] - ((textTall) / 2));
			vgui2::surface()->DrawSetTextColor(255, 255, 255, 128);

			for (size_t j = 0; j < wcslen(m_sUnicodes[i]); j++)
				vgui2::surface()->DrawUnicodeCharAdd(m_sUnicodes[i][j]);
		}
	}

	return TRUE;
}