#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "cstrikechatdialog.h"

Color g_ColorBlue(153, 204, 255, 255);
Color g_ColorRed(255, 63.75, 63.75, 255);
Color g_ColorGreen(153, 255, 153, 255);
Color g_ColorDarkGreen(64, 255, 64, 255);
Color g_ColorYellow(255, 178.5, 0.0, 255);
Color g_ColorGrey(204, 204, 204, 255);

using namespace vgui;

CCSChatDialog::CCSChatDialog(void) : BaseClass()
{
	SetProportional(true);
	SetSizeable(false);
}

void CCSChatDialog::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("Resource/UI/ChatDialog.res", "GAME");
	GetPos(m_iSaveX, m_iSaveY);
	SetVisible(false);
}

void CCSChatDialog::CreateChatInputLine(void)
{
	m_pChatInput = new CChatDialogInputLine(this, "ChatInputLine");
	m_pChatInput->SetVisible(false);
}

void CCSChatDialog::CreateChatLines(void)
{
	m_ChatLine = new CChatDialogLine(this, "ChatLine");
	m_ChatLine->SetVisible(false);
}

void CCSChatDialog::Init(void)
{
	BaseClass::Init();
}

void CCSChatDialog::VidInit(void)
{
	BaseClass::VidInit();
}

void CCSChatDialog::Reset(void)
{
}

void CCSChatDialog::Print(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4)
{
	char szString[2048];
	wchar_t outputBuf[256];
	wchar_t szBuf[5][256];

	if (strlen(msg_name) > 0)
		g_pVGuiLocalize->ConvertANSIToUnicode(msg_name, szBuf[0], sizeof(szBuf[0]));

	if (strlen(param1) > 0)
		g_pVGuiLocalize->ConvertANSIToUnicode(param1, szBuf[1], sizeof(szBuf[1]));

	if (strlen(param2) > 0)
		g_pVGuiLocalize->ConvertANSIToUnicode(param2, szBuf[2], sizeof(szBuf[2]));

	if (strlen(param3) > 0)
		g_pVGuiLocalize->ConvertANSIToUnicode(param3, szBuf[3], sizeof(szBuf[30]));

	if (strlen(param4) > 0)
		g_pVGuiLocalize->ConvertANSIToUnicode(param4, szBuf[4], sizeof(szBuf[4]));

	switch (msg_dest)
	{
		case HUD_PRINTCENTER:
		{
			g_pVGuiLocalize->ConstructString(outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
			g_pVGuiLocalize->ConvertUnicodeToANSI(outputBuf, szString, sizeof(szString));
			CenterPrint(ConvertCRtoNL(szString));
			break;
		}

		case HUD_PRINTNOTIFY:
		{
			szString[0] = 1;
			g_pVGuiLocalize->ConstructString(outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
			g_pVGuiLocalize->ConvertUnicodeToANSI(outputBuf, szString + 1, sizeof(szString) - 1);
			ConsolePrint(ConvertCRtoNL(szString));
			break;
		}

		case HUD_PRINTTALK:
		{
			swprintf(outputBuf, szBuf[0], szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
			g_pVGuiLocalize->ConvertUnicodeToANSI(outputBuf, szString, sizeof(szString));

			int len = strlen(szString);

			if (len && szString[len - 1] != '\n' && szString[len - 1] != '\r')
				Q_strncat(szString, "\n", sizeof(szString), 1);

			Printf("%s", ConvertCRtoNL(szString));
			Msg("%s\n", ConvertCRtoNL(szString));
			break;
		}

		case HUD_PRINTCONSOLE:
		{
			g_pVGuiLocalize->ConstructString(outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
			g_pVGuiLocalize->ConvertUnicodeToANSI(outputBuf, szString, sizeof(szString));
			ConsolePrint(ConvertCRtoNL(szString));
			break;
		}
	}

	if (g_pViewPort->AllowedToPrintText())
		SetVisible(true);
}

void CCSChatDialog::OnThink(void)
{
	if (g_iUser1 || gEngfuncs.IsSpectateOnly())
	{
		int bottomBarHeight = g_pViewPort->GetSpectatorBottomBarHeight();
		SetPos(m_iSaveX, ScreenHeight - bottomBarHeight - GetWide() + (ScreenHeight - m_iSaveY));
	}
	else
		SetPos(m_iSaveX, m_iSaveY);

	if (gHUD.m_iIntermission && IsMouseInputEnabled())
		StopMessageMode();

	BaseClass::OnThink();
}

void CCSChatDialog::MsgFunc_SayText(void)
{
	int client = READ_BYTE();

	wchar_t szBuf[6][256];
	char untranslated_msg_text[256];
	wchar_t *msg_text = ReadLocalizedString(szBuf[0], sizeof(szBuf[0]), false, untranslated_msg_text, sizeof(untranslated_msg_text));

	ReadChatTextString(szBuf[1], sizeof(szBuf[1]));
	ReadChatTextString(szBuf[2], sizeof(szBuf[2]));
	ReadLocalizedString(szBuf[3], sizeof(szBuf[3]), true);
	ReadLocalizedString(szBuf[4], sizeof(szBuf[4]), true);

	if (client > 0 && szBuf[1][0] == '\0')
		g_pVGuiLocalize->ConvertANSIToUnicode(g_PlayerInfoList[client].name, szBuf[1], sizeof(szBuf[1]));

	g_pVGuiLocalize->ConstructString(szBuf[5], sizeof(szBuf[5]), msg_text, 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
	ChatPrintf(client, ConvertCRtoNL(szBuf[5]));
#ifdef _DEBUG
	char ansiString[512];
	g_pVGuiLocalize->ConvertUnicodeToANSI(ConvertCRtoNL(szBuf[5]), ansiString, sizeof(ansiString));
	Msg("%s\n", RemoveColorMarkup(ansiString));
#endif
}

void CCSChatDialog::MsgFunc_TextMsg(void)
{
	char szString[2048];
	int msg_dest = READ_BYTE();
	int client = -1;

	if (msg_dest == HUD_PRINTRADIO)
	{
		client = atoi(READ_STRING());

		wchar_t szBuf[6][256];
		char untranslated_msg_text[256];
		wchar_t *msg_text = ReadLocalizedString(szBuf[0], sizeof(szBuf[0]), false, untranslated_msg_text, sizeof(untranslated_msg_text));

		ReadChatTextString(szBuf[1], sizeof(szBuf[1]));
		sprintf_s(szString, sizeof(szString), "#%s", READ_STRING());

		const wchar_t *pBuf = g_pVGuiLocalize->Find(szString);

		if (pBuf)
		{
			wcsncpy(szBuf[2], pBuf, sizeof(szBuf[3]) / sizeof(wchar_t));
			szBuf[2][sizeof(szBuf[3]) / sizeof(wchar_t) - 1] = 0;
		}
		else
		{
			char *str = LookupString(&szString[1]);

			if (!str[0])
				g_pVGuiLocalize->ConvertANSIToUnicode(&szString[1], szBuf[2], sizeof(szBuf[3]));
			else
				g_pVGuiLocalize->ConvertANSIToUnicode(str, szBuf[2], sizeof(szBuf[3]));
		}

		ReadLocalizedString(szBuf[3], sizeof(szBuf[3]), true);
		ReadLocalizedString(szBuf[4], sizeof(szBuf[4]), true);

		g_pVGuiLocalize->ConstructString(szBuf[5], sizeof(szBuf[5]), msg_text, 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4]);
		ChatPrintf(client, ConvertCRtoNL(szBuf[5]));
#ifdef _DEBUG
		char ansiString[512];
		g_pVGuiLocalize->ConvertUnicodeToANSI(ConvertCRtoNL(szBuf[5]), ansiString, sizeof(ansiString));
		Msg("%s\n", RemoveColorMarkup(ansiString));
#endif
	}
	else
	{
		wchar_t szBuf[5][256];
		char szBuf2[5][256];

		for (int i = 0; i < 5; ++i)
		{
			strcpy_s(szString, sizeof(szString), READ_STRING());

			char *tmpStr = LookupString(szString, &msg_dest);
			const wchar_t *pBuf = g_pVGuiLocalize->Find(tmpStr);

			if (!pBuf && tmpStr[0] == '#')
			{
				StripEndNewlineFromString(tmpStr);
				pBuf = g_pVGuiLocalize->Find(tmpStr);
			}

			if (pBuf)
			{
				int nMaxChars = sizeof(szBuf[i]) / sizeof(wchar_t);
				wcsncpy(szBuf[i], pBuf, nMaxChars);
				szBuf[i][nMaxChars - 1] = 0;
			}
			else
			{
				if (i)
					StripEndNewlineFromString(tmpStr);

				strcpy(szBuf2[i], tmpStr);
			}
		}

		Print(msg_dest, szBuf2[0], szBuf2[1], szBuf2[2], szBuf2[3], szBuf2[4]);
	}
}

int CCSChatDialog::GetChatInputOffset(void)
{
	if (m_pChatInput->IsVisible())
		return m_iFontHeight;

	return 0;
}

void CCSChatDialog::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

Color CCSChatDialog::GetClientColor(int clientIndex)
{
	if (clientIndex == 0)
		return g_ColorGreen;

	switch (g_PlayerExtraInfo[clientIndex].teamnumber)
	{
		case 1: return g_ColorRed;
		case 2: return g_ColorBlue;
		default: return g_ColorGrey;
	}

	return g_ColorYellow;
}

Color CCSChatDialog::GetTextColorForClient(TextColor colorNum, int clientIndex)
{
	Color c;

	switch (colorNum)
	{
		case COLOR_PLAYERNAME:
		{
			c = GetClientColor(clientIndex);
			break;
		}

		case COLOR_LOCATION:
		{
			c = g_ColorDarkGreen;
			break;
		}

		default:
		{
			c = g_ColorYellow;
		}
	}

	return Color(c[0], c[1], c[2], 255);
}