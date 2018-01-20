#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "CounterStrikeViewport.h"

extern void *g_pgHUD_m_Menu;
extern void (__fastcall *g_pfnCHudMenu_SelectMenuItem)(void *, int, int menu_item);

#define MAX_MENU_STRING 512

char g_szMenuString[MAX_MENU_STRING];
char g_szPrelocalisedMenuString[MAX_MENU_STRING];

pfnUserMsgHook g_pfnShowMenu;
pfnUserMsgHook g_pfnVGUIMenu;

xcommand_t g_pfnSizeDown;
xcommand_t g_pfnSizeUp;

int KB_ConvertString(char *in, char **ppout);

DECLARE_MESSAGE(m_Menu, ShowMenu);
DECLARE_MESSAGE(m_Menu, VGUIMenu);

DECLARE_COMMAND(m_Menu, SizeDown);
DECLARE_COMMAND(m_Menu, SizeUp);

int CHudMenu::Init(void)
{
	gHUD.AddHudElem(this);

	g_pfnShowMenu = HOOK_MESSAGE(ShowMenu);
	g_pfnVGUIMenu = HOOK_MESSAGE(VGUIMenu);

	g_pfnSizeUp   = HOOK_COMMAND("sizeup",   SizeUp);
	g_pfnSizeDown = HOOK_COMMAND("sizedown", SizeDown);

	InitHUDData();

	return TRUE;
}

void CHudMenu::InitHUDData(void)
{
	m_fMenuDisplayed = FALSE;
	m_bitsValidSlots = FALSE;

	Reset();
}

void CHudMenu::Reset(void)
{
	g_szPrelocalisedMenuString[0] = NULL;
	m_fWaitingForMore = FALSE;
}

int CHudMenu::VidInit(void)
{
	return TRUE;
}

int CHudMenu::Draw(float flTime)
{
	return TRUE;
}

bool CHudMenu::SelectMenuItem(int menu_item)
{
	char szbuf[32];

	if (g_pViewPort->SelectMenuItem(menu_item))
	{
		m_fMenuDisplayed = FALSE;
		return true;
	}

	return false;

	if ((menu_item == '-' && (m_bitsValidSlots & MENU_KEY_PREV)) || (menu_item == '+' && (m_bitsValidSlots & MENU_KEY_NEXT)))
	{
		sprintf(szbuf, "menuselect %c\n", menu_item);
		ClientCmd(szbuf);

		m_fMenuDisplayed = FALSE;
		m_iFlags &= ~HUD_ACTIVE;
		return true;
	}

	if ((menu_item > 0) && (m_bitsValidSlots & (1 << (menu_item - 1))))
	{
		sprintf(szbuf, "menuselect %d\n", menu_item);
		ClientCmd(szbuf);

		m_fMenuDisplayed = FALSE;
		m_iFlags &= ~HUD_ACTIVE;
		return true;
	}

	return false;
}

int CHudMenu::MsgFunc_ShowMenu(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_bitsValidSlots = READ_SHORT();

	if (!cl_newmenu->value)
		return g_pfnShowMenu(pszName, iSize, pbuf);

	char *temp = NULL;
	int DisplayTime = READ_CHAR();
	int NeedMore = READ_BYTE();

	if (DisplayTime > 0)
		m_flShutoffTime = DisplayTime + gHUD.m_flTime;
	else
		m_flShutoffTime = -1;

	if (m_bitsValidSlots)
	{
		if (!m_fWaitingForMore)
		{
			strncpy(g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING);
		}
		else
			strncat(g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING - strlen(g_szPrelocalisedMenuString));

		g_szPrelocalisedMenuString[MAX_MENU_STRING - 1] = NULL;

		if (!NeedMore)
		{
			strcpy(g_szMenuString, gHUD.m_TextMessage.BufferedLocaliseTextString(g_szPrelocalisedMenuString));

			if (KB_ConvertString(g_szMenuString, &temp))
			{
				strcpy(g_szMenuString, temp);
				free(temp);
			}
		}

		m_fMenuDisplayed = TRUE;
		m_iFlags |= HUD_ACTIVE;
	}
	else
	{
		m_fMenuDisplayed = FALSE;
		m_iFlags &= ~HUD_ACTIVE;
	}

	m_fWaitingForMore = NeedMore;

	if (!g_pViewPort->FireMessage(pszName, iSize, pbuf))
		return g_pfnShowMenu(pszName, iSize, pbuf);

	return TRUE;
}

int CHudMenu::MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (!g_pViewPort->FireMessage(pszName, iSize, pbuf))
		return g_pfnVGUIMenu(pszName, iSize, pbuf);

	return TRUE;
}

void CHudMenu::UserCmd_SizeDown(void)
{
	if (SelectMenuItem('-'))
		m_fMenuDisplayed = FALSE;
	else
		g_pfnSizeDown();
}

void CHudMenu::UserCmd_SizeUp(void)
{
	if (SelectMenuItem('+'))
	{
		m_fMenuDisplayed = FALSE;
	}
	else
		g_pfnSizeUp();
}