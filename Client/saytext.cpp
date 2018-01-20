#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "CounterStrikeViewport.h"

extern float *GetClientColor(int clientIndex);

#define MAX_LINES 5
#define MAX_CHARS_PER_LINE 256

#define MAX_LINE_WIDTH (ScreenWidth - 40)
#define LINE_START 10

static float SCROLL_SPEED = 5;

static char g_szLineBuffer[MAX_LINES + 1][MAX_CHARS_PER_LINE];
static float *g_pflNameColors[MAX_LINES + 1];
static int g_iNameLengths[MAX_LINES + 1];
static float flScrollTime = 0;

static int Y_START = 0;
static int line_height = 0;

DECLARE_MESSAGE(m_SayText, SayText);
DECLARE_MESSAGE(m_SayText, SendAudio);
DECLARE_MESSAGE(m_SayText, SendRadio);
DECLARE_COMMAND(m_SayText, MessageMode);
DECLARE_COMMAND(m_SayText, MessageMode2);

pfnUserMsgHook g_pfnSayText;
pfnUserMsgHook g_pfnSendAudio;
pfnUserMsgHook g_pfnSendRadio;

xcommand_t g_pfnMessageMode;
xcommand_t g_pfnMessageMode2;

int CHudSayText::Init(void)
{
	g_pfnSendAudio = HOOK_MESSAGE(SendAudio);
	g_pfnSendRadio = HOOK_MESSAGE(SendRadio);
	g_pfnSayText   = HOOK_MESSAGE(SayText);

	g_pfnMessageMode  = HOOK_COMMAND("messagemode",  MessageMode);
	g_pfnMessageMode2 = HOOK_COMMAND("messagemode2", MessageMode2);

	InitHUDData();

	m_HUD_saytext	   = gEngfuncs.pfnGetCvarPointer("hud_saytext_internal");
	m_HUD_saytext_time = gEngfuncs.pfnGetCvarPointer("hud_saytext_time");

	m_iFlags |= HUD_INTERMISSION;

	gHUD.AddHudElem(this);

	return TRUE;
}

void CHudSayText::InitHUDData(void)
{
}

int CHudSayText::VidInit(void)
{
	return TRUE;
}

void CHudSayText::Reset(void)
{
}

void CHudSayText::UserCmd_MessageMode(void)
{
	if (!gHUD.m_pCvarDraw->value || g_pViewPort->IsScoreBoardVisible())
		return g_pfnMessageMode();

	if (!gHUD.m_iIntermission && gEngfuncs.Cmd_Argc() == 1 && cl_newchat->value)
		return g_pViewPort->StartMessageMode();

	return g_pfnMessageMode();
}

void CHudSayText::UserCmd_MessageMode2(void)
{
	if (!gHUD.m_pCvarDraw->value || g_pViewPort->IsScoreBoardVisible())
		return g_pfnMessageMode2();

	if (!gHUD.m_iIntermission && gEngfuncs.Cmd_Argc() == 1 && cl_newchat->value)
		return g_pViewPort->StartMessageMode2();

	return g_pfnMessageMode2();
}

int ScrollTextUp(void)
{
	ConsolePrint(g_szLineBuffer[0]);
	g_szLineBuffer[MAX_LINES][0] = 0;
	memmove(g_szLineBuffer[0], g_szLineBuffer[1], sizeof(g_szLineBuffer) - sizeof(g_szLineBuffer[0]));
	memmove(&g_pflNameColors[0], &g_pflNameColors[1], sizeof(g_pflNameColors) - sizeof(g_pflNameColors[0]));
	memmove(&g_iNameLengths[0], &g_iNameLengths[1], sizeof(g_iNameLengths) - sizeof(g_iNameLengths[0]));
	g_szLineBuffer[MAX_LINES-1][0] = 0;

	if (g_szLineBuffer[0][0] == ' ')
	{
		g_szLineBuffer[0][0] = 2;
		return 1 + ScrollTextUp();
	}

	return 1;
}

int CHudSayText::Draw(float flTime)
{
	return TRUE;
}

int CHudSayText::MsgFunc_SayText(const char *pszName, int iSize, void *pbuf)
{
	if (!cl_newchat->value)
		return g_pfnSayText(pszName, iSize, pbuf);

	if ((gViewPortInterface && !gViewPortInterface->AllowedToPrintText()) || !m_HUD_saytext->value)
		return g_pfnSayText(pszName, iSize, pbuf);

	if (!g_pViewPort->FireMessage(pszName, iSize, pbuf))
		return g_pfnSayText(pszName, iSize, pbuf);

	return TRUE;
}

int CHudSayText::MsgFunc_SendAudio(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client = READ_BYTE();
	char *code = READ_STRING();
	int pitch = READ_SHORT();

	char name[64];
	sprintf(name, "misc/talk.wav");

	if (strstr(code, "%!"))
	{
		char *pName = name;
		char *pCode = &code[1];

		while (*pCode && *pCode <= 'z')
			*pName++ = *pCode++;

		*pName = '\0';
		gEngfuncs.pfnPlaySoundByNameAtPitch(name, 1.0, pitch);
	}
	else
		gEngfuncs.pfnPlaySoundVoiceByName(code, 1.0, pitch);

	g_PlayerExtraInfo[client].radarflashes = 22;
	g_PlayerExtraInfo[client].radarflash = gHUD.m_flTime;
	g_PlayerExtraInfo[client].radarflashon = 1;

	return TRUE;
}

int CHudSayText::MsgFunc_SendRadio(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client = READ_BYTE();
	char *file = READ_STRING();
	int pitch = READ_SHORT();

	char name[64];
	sprintf(name, "radio/%s.wav", file);
	gEngfuncs.pfnPlaySoundVoiceByName(name, 1.0, pitch);

	g_PlayerExtraInfo[client].radarflashes = 22;
	g_PlayerExtraInfo[client].radarflash = gHUD.m_flTime;
	g_PlayerExtraInfo[client].radarflashon = 1;

	return TRUE;
}

void CHudSayText::SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4)
{
	if (gViewPortInterface && !gViewPortInterface->AllowedToPrintText())
	{
		ConsolePrint(pszBuf);

		return;
	}
}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo(int line)
{
	int line_width = 0;
	GetConsoleStringSize(g_szLineBuffer[line], &line_width, &line_height);

	if ((line_width + LINE_START) > MAX_LINE_WIDTH)
	{
		int length = LINE_START;
		int tmp_len = 0;
		char *last_break = NULL;

		for (char *x = g_szLineBuffer[line]; *x != 0; x++)
		{
			if (x[0] == '/' && x[1] == '(')
			{
				x += 2;

				while (*x != 0 && *x != ')')
					x++;

				if (*x != 0)
					x++;

				if (*x == 0)
					break;
			}

			char buf[2];
			buf[1] = 0;

			if (*x == ' ' && x != g_szLineBuffer[line])
				last_break = x;

			buf[0] = *x;
			GetConsoleStringSize(buf, &tmp_len, &line_height);
			length += tmp_len;

			if (length > MAX_LINE_WIDTH)
			{
				if (!last_break)
					last_break = x - 1;

				x = last_break;

				int j;

				do
				{
					for (j = 0; j < MAX_LINES; j++)
					{
						if (!*g_szLineBuffer[j])
							break;
					}

					if (j == MAX_LINES)
					{
						int linesmoved = ScrollTextUp();
						line -= linesmoved;
						last_break = last_break - (sizeof(g_szLineBuffer[0]) * linesmoved);
					}
				}
				while (j == MAX_LINES);

				if ((char)*last_break == (char)' ')
				{
					int linelen = strlen(g_szLineBuffer[j]);
					int remaininglen = strlen(last_break);

					if ((linelen - remaininglen) <= MAX_CHARS_PER_LINE)
						strcat(g_szLineBuffer[j], last_break);
				}
				else
				{
					if ((strlen(g_szLineBuffer[j]) - strlen(last_break) - 2) < MAX_CHARS_PER_LINE)
					{
						strcat(g_szLineBuffer[j], " ");
						strcat(g_szLineBuffer[j], last_break);
					}
				}

				*last_break = 0;

				EnsureTextFitsInOneLineAndWrapIfHaveTo(j);
				break;
			}
		}
	}
}