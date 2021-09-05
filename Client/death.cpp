#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include <triangleapi.h>

#include "CounterStrikeViewport.h"

extern int g_iUser1;

DECLARE_MESSAGE(m_DeathNotice, DeathMsg);

struct DeathNoticeItem
{
	char szKiller[MAX_PLAYER_NAME_LENGTH * 2];
	char szVictim[MAX_PLAYER_NAME_LENGTH * 2];
	int iId;
	int iHeadShotId;
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
	vgui2::IImage **DrawBg;
};

#define MAX_DEATHNOTICES 8
#define MAX_DRAWDEATHNOTICES 4

static int DEATHNOTICE_DISPLAY_TIME = 6;
static int KILLEFFECT_DISPLAY_TIME = 1;

#define DEATHNOTICE_TOP 32

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

float *GetClientColor(int clientIndex)
{
	return g_pfnGetClientColor(clientIndex);
}

int CHudDeathNotice::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(DeathMsg);

	return 1;
}

void CHudDeathNotice::Reset(void)
{
	m_showKill = false;
	m_killEffectTime = 0;
}

void CHudDeathNotice::InitHUDData(void)
{
	memset(rgDeathNoticeList, 0, sizeof(rgDeathNoticeList));
}

int CHudDeathNotice::VidInit(void)
{
	m_HUD_d_skull = gHUD.GetSpriteIndex("d_skull");
	m_headSprite = gHUD.GetSpriteIndex("d_headshot");
	m_headWidth = gHUD.GetSpriteRect(m_headSprite).right - gHUD.GetSpriteRect(m_headSprite).left;

	m_killBg[0] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_left", true);
	m_killBg[1] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_center", true);
	m_killBg[2] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_right", true);
	m_deathBg[0] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_left", true);
	m_deathBg[1] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_center", true);
	m_deathBg[2] = vgui2::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_right", true);

	int w, t;
	m_killBg[0]->GetContentSize(w, t);
	m_iDrawBgWidth = w;

	gEngfuncs.pfnDrawConsoleStringLen("N", &w, &t);

	m_iFontHeight = t;
	m_iDrawBgHeight = m_iFontHeight * 1.15;

	if (m_iDrawBgHeight < 24)
		m_iDrawBgHeight = 24;

	m_killBg[0]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_killBg[2]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_deathBg[0]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_deathBg[2]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);

	m_KillerMsgSprite = SPR_Load("sprites/z4b/os_skull.spr");
	return 1;
}

int CHudDeathNotice::GetDeathNoticeY(void)
{
	if (g_iUser1 || gEngfuncs.IsSpectateOnly())
		return gViewPortInterface->GetSpectatorTopBarHeight() + (YRES(DEATHNOTICE_TOP) / 4);

	return YRES(DEATHNOTICE_TOP);
}

int CHudDeathNotice::Draw(float flTime)
{
	int x, y, r, g, b;
	int noticeY = GetDeathNoticeY();

	int c = 0;

	for (; c < MAX_DEATHNOTICES; c++)
	{
		if (rgDeathNoticeList[c].iId == 0)
			break;
	}

	c = max(c, MAX_DRAWDEATHNOTICES);

	for (int i = c - 4; i < c; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;

		if (rgDeathNoticeList[i].flDisplayTime < flTime)
		{
			memmove(&rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i));
			i--;
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min(rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME);

		if (gViewPortInterface && gViewPortInterface->AllowedToPrintText() != FALSE)
		{
			int xMin, id;
			int iconWidth;
			int killerWidth, victimWidth;
			int xOffset;
			int yOffset;

			y = noticeY + ((m_iDrawBgHeight * 1.15) * (i - (c - 4)));
			id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			iconWidth = gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left;
			killerWidth = ConsoleStringLen(rgDeathNoticeList[i].szKiller);
			victimWidth = ConsoleStringLen(rgDeathNoticeList[i].szVictim);
			xOffset = 3;
			yOffset = (m_iDrawBgHeight - m_iFontHeight);
			y = y - yOffset;

			if (cl_killeffect->value && rgDeathNoticeList[i].DrawBg)
			{
				x = ScreenWidth - victimWidth - ((iconWidth + YRES(5)) + ((rgDeathNoticeList[i].iHeadShotId != -1) ? (m_headWidth + YRES(5)) : 0)) - (YRES(5) * 3);
				xMin = x;

				if (!rgDeathNoticeList[i].iSuicide)
				{
					x -= YRES(5) + killerWidth;
					xMin = x;
					x += YRES(5) + killerWidth;
				}

				x += iconWidth + YRES(5);

				if (rgDeathNoticeList[i].iHeadShotId != -1)
					x += m_headWidth + YRES(5);

				if (rgDeathNoticeList[i].iNonPlayerKill == FALSE)
					x += victimWidth;

				gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);

				rgDeathNoticeList[i].DrawBg[0]->SetPos(xMin - 3 - xOffset, y);
				rgDeathNoticeList[i].DrawBg[0]->Paint();

				rgDeathNoticeList[i].DrawBg[1]->SetPos(xMin - 3 - xOffset + m_iDrawBgWidth, y);
				rgDeathNoticeList[i].DrawBg[1]->SetSize(x + xOffset - (xMin - 3 - xOffset + m_iDrawBgWidth), m_iDrawBgHeight);
				rgDeathNoticeList[i].DrawBg[1]->Paint();

				rgDeathNoticeList[i].DrawBg[2]->SetPos(x + xOffset, y);
				rgDeathNoticeList[i].DrawBg[2]->Paint();
			}

			x = ScreenWidth - victimWidth - ((iconWidth + YRES(5)) + ((rgDeathNoticeList[i].iHeadShotId != -1) ? (m_headWidth + YRES(5)) : 0)) - (YRES(5) * 3);

			if (!rgDeathNoticeList[i].iSuicide)
			{
				x -= (YRES(5) + killerWidth);

				if (rgDeathNoticeList[i].KillerColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2]);

				x = YRES(5) + DrawConsoleString(x, y + ((m_iDrawBgHeight - m_iFontHeight) / 2), rgDeathNoticeList[i].szKiller);
			}

			if (rgDeathNoticeList[i].iTeamKill)
			{
				r = 10;
				g = 240;
				b = 10;
			}
			else
			{
				r = 255;
				g = 80;
				b = 0;
			}

			SPR_Set(gHUD.GetSprite(id), r, g, b);
			SPR_DrawAdditive(0, x, y + (abs(m_iDrawBgHeight - (gHUD.GetSpriteRect(id).bottom - gHUD.GetSpriteRect(id).top)) / 2), &gHUD.GetSpriteRect(id));

			x += iconWidth + YRES(5);

			if (rgDeathNoticeList[i].iHeadShotId != -1)
			{
				SPR_Set(gHUD.GetSprite(rgDeathNoticeList[i].iHeadShotId), r, g, b);
				SPR_DrawAdditive(0, x, y + (abs(m_iDrawBgHeight - (gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId).bottom - gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId).top)) / 2), &gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId));

				x += m_headWidth + YRES(5);
			}

			if (rgDeathNoticeList[i].iNonPlayerKill == FALSE)
			{
				if (rgDeathNoticeList[i].VictimColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2]);

				x = DrawConsoleString(x, y + ((m_iDrawBgHeight - m_iFontHeight) / 2), rgDeathNoticeList[i].szVictim);
			}
		}
	}

	if (m_showKill)
	{
		m_killEffectTime = min(m_killEffectTime, gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME);

		if (gHUD.m_flTime < m_killEffectTime)
		{
			float alpha = (m_killEffectTime - gHUD.m_flTime) / KILLEFFECT_DISPLAY_TIME;
			float scale = (m_killEffectTime - gHUD.m_flTime) / KILLEFFECT_DISPLAY_TIME / 2.0f + 0.5f;

			alpha = sqrtf(alpha);
			scale = sqrtf(scale);
			scale /= 3;

			int w = gEngfuncs.pfnSPR_Width(m_KillerMsgSprite, 0);
			int h = gEngfuncs.pfnSPR_Height(m_KillerMsgSprite, 0);

			w *= scale;
			h *= scale;

			x = ScreenWidth / 2 - w / 2;
			y = ScreenHeight / 2 - h / 2;
			
			auto hSpriteModel = const_cast<struct model_s *>(gEngfuncs.GetSpritePointer(m_KillerMsgSprite));
			gEngfuncs.pTriAPI->SpriteTexture(hSpriteModel, 0);

			gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
			gEngfuncs.pTriAPI->Color4f(1, 1, 1, alpha);

			gEngfuncs.pTriAPI->Begin(TRI_QUADS);
			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			gEngfuncs.pTriAPI->Vertex3f(x, y + h, 0);
			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			gEngfuncs.pTriAPI->Vertex3f(x + w, y + h, 0);
			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			gEngfuncs.pTriAPI->Vertex3f(x + w, y, 0);
			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			gEngfuncs.pTriAPI->Vertex3f(x, y, 0);
			gEngfuncs.pTriAPI->End();
		}
		else
		{
			m_showKill = false;
		}
	}

	return 1;
}

int CHudDeathNotice::MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);

	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int headshot = READ_BYTE();
	int multiKills = 0;

	char killedwith[32];
	strcpy(killedwith, "d_");
	strncat(killedwith, READ_STRING(), 32);

	if (gViewPortInterface)
		gViewPortInterface->DeathMsg(killer, victim);

	gHUD.m_Spectator.DeathMessage(victim);

	for (int j = 0; j < MAX_DEATHNOTICES; j++)
	{
		if (rgDeathNoticeList[j].iId == 0)
			break;

		if (rgDeathNoticeList[j].DrawBg == m_killBg)
			multiKills++;
	}

	if (cl_killmessage->value)
	{
		if (killer == gHUD.m_iPlayerNum && victim != gHUD.m_iPlayerNum)
		{
			gEngfuncs.pfnClientCmd("speak \"fvox/bell2\"\n");
			m_showKill = true;
			m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
		}
	}

	int i = 0;

	for (; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;
	}

	if (i == MAX_DEATHNOTICES)
	{
		memmove(rgDeathNoticeList, rgDeathNoticeList + 1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES);
		i = MAX_DEATHNOTICES - 1;
	}

	if (gViewPortInterface)
		gViewPortInterface->GetAllPlayersInfo();

	char *killer_name = g_PlayerInfoList[killer].name;

	if (!killer_name)
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor(killer);
		strncpy(rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	char *victim_name = NULL;

	if (((char)victim) != -1)
		victim_name = g_PlayerInfoList[victim].name;

	if (!victim_name)
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor(victim);
		strncpy(rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	if (((char)victim) == -1)
	{
		rgDeathNoticeList[i].iNonPlayerKill = TRUE;
		strcpy(rgDeathNoticeList[i].szVictim, killedwith + 2);
	}
	else
	{
		if (killer == victim || killer == 0)
			rgDeathNoticeList[i].iSuicide = TRUE;

		if (!strcmp(killedwith, "d_teammate"))
			rgDeathNoticeList[i].iTeamKill = TRUE;
	}

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	rgDeathNoticeList[i].iId = gHUD.GetSpriteIndex(killedwith);
	rgDeathNoticeList[i].iHeadShotId = headshot ? m_headSprite : -1;
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	if (victim == gHUD.m_iPlayerNum)
		rgDeathNoticeList[i].DrawBg = m_deathBg;
	else if (killer == gHUD.m_iPlayerNum)
		rgDeathNoticeList[i].DrawBg = m_killBg;
	else
		rgDeathNoticeList[i].DrawBg = NULL;

	if (rgDeathNoticeList[i].iNonPlayerKill)
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" killed a ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
		ConsolePrint("\n");
	}
	else
	{
		if (rgDeathNoticeList[i].iSuicide)
		{
			ConsolePrint(rgDeathNoticeList[i].szVictim);

			if (!strcmp(killedwith, "d_world"))
				ConsolePrint(" died");
			else
				ConsolePrint(" killed self");
		}
		else if (rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed his teammate ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}
		else
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}

		if (killedwith && *killedwith && (*killedwith > 13) && strcmp(killedwith, "d_world") && !rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(" with ");
			ConsolePrint(killedwith + 2);
		}

		ConsolePrint("\n");
	}

	return 1;
}