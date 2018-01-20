#include "hud.h"
#include "parsemsg.h"
#include "cdll_dll.h"
#include "player.h"
#include "vgui_int.h"
#include "demo_api.h"
#include "game_shared/voice_status.h"

#include "CSBackGroundPanel.h"
#include "CounterStrikeViewport.h"

#include "vgui/hud_layer.h"

#include "game_controls/commandmenu.h"
#include "game_controls/NavProgress.h"

#include "vgui/cstriketeammenu.h"
#include "vgui/cstrikeclassmenu.h"
#include "vgui/cstrikeclientscoreboard.h"
#include "vgui/cstriketextwindow.h"
#include "vgui/cstrikespectatorgui.h"
#include "vgui/cstrikebuymenu.h"
#include "vgui/cstrikechatdialog.h"

Panel *g_lastPanel = NULL;
Button *g_lastButton = NULL;

using namespace vgui;

CViewport *g_pViewPort = NULL;

CViewport::CViewport(void) : Panel(NULL, "NewClientViewport")
{
	m_bInitialied = false;

	int swide, stall;
	surface()->GetScreenSize(swide, stall);

	MakePopup(false, true);
	SetScheme("ClientScheme");
	SetBounds(0, 0, swide, stall);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	m_bInitialied = false;
	m_bShowBackGround = false;

	m_pHudLayer = new CHudLayer(this);
	m_pActivePanel = NULL;
	m_pLastActivePanel = NULL;
	m_pPanelShow = NULL;

	m_Panels.RemoveAll();

	m_TeamColors[0] = Color(0, 0, 0, 128);
	m_TeamColors[1] = COLOR_RED;
	m_TeamColors[2] = COLOR_BLUE;
	m_TeamColors[3] = COLOR_GREY;

	m_iTeamScores[0] = 0;
	m_iTeamScores[1] = 0;
	m_iTeamScores[2] = 0;
	m_iTeamScores[3] = 0;
}

CViewport::~CViewport(void)
{
	RemoveAllPanels();

	delete m_pHudLayer;
}

void CViewport::Start(void)
{
	CreateBackGround();

	m_pCommandMenu = (CommandMenu *)AddNewPanel(new CommandMenu);
	m_pNavProgress = (CNavProgress *)AddNewPanel(new CNavProgress);
	m_pTeamMenu = (CCSTeamMenu *)AddNewPanel(new CCSTeamMenu);
	m_pClassMenu_TER = (CCSClassMenu_TER *)AddNewPanel(new CCSClassMenu_TER);
	m_pClassMenu_CT = (CCSClassMenu_CT *)AddNewPanel(new CCSClassMenu_CT);
	m_pTextWindow = (CCSTextWindow *)AddNewPanel(new CCSTextWindow);
	m_pScoreBoard = (CCSClientScoreBoardDialog *)AddNewPanel(new CCSClientScoreBoardDialog);
	m_pSpectatorGUI = (CCSSpectatorGUI *)AddNewPanel(new CCSSpectatorGUI);
	m_pSpectatorMenu = (CCSSpectatorMenu *)AddNewPanel(new CCSSpectatorMenu);
	m_pBuyMenu_TER = (CCSBuyMenu_TER *)AddNewPanel(new CCSBuyMenu_TER);
	m_pBuyMenu_CT = (CCSBuyMenu_CT *)AddNewPanel(new CCSBuyMenu_CT);
	m_pChatDialog = (CCSChatDialog *)AddNewPanel(new CCSChatDialog);
	
	m_pCommandMenu->LoadFromFile("Resource/CommandMenu.res");
	m_pCommandMenu->SetCloseKey(KEY_H);
	m_pCommandMenu->SetProportional(true);
	
	m_bInitialied = true;
	m_pHudLayer->Start();

	SetVisible(false);
}

void CViewport::SetParent(VPANEL parent)
{
	Panel::SetParent(parent);

	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->SetParent(this);

	m_pHudLayer->SetParent(GetVPanel());
}

void CViewport::SetVisible(bool state)
{
	Panel::SetVisible(state);

	if (state)
	{
		if (m_pActivePanel)
		{
			if (m_pActivePanel->HasInputElements() && m_pActivePanel->IsMouseInputEnabled())
			{
				m_pActivePanel->SetMouseInputEnabled(false);
				m_pActivePanel->SetMouseInputEnabled(true);
			}
		}

		if (!m_pTextWindow->IsExited())
		{
			m_pTextWindow->SetMouseInputEnabled(false);
			m_pTextWindow->SetMouseInputEnabled(true);
			m_pTextWindow->RequestFocus();
		}
	}

	m_pHudLayer->SetVisible(state);
}

int CViewport::GetViewPortScheme(void)
{
	return m_pBackGround->GetScheme();
}

VPANEL CViewport::GetViewPortPanel(void)
{
	return m_pBackGround->GetVParent();
}

void CViewport::CreateBackGround(void)
{
	m_pBackGround = new CCSBackGroundPanel(this);
	m_pBackGround->SetZPos(-20);
	m_pBackGround->SetVisible(false);
}

void CViewport::ShowBackGround(bool bShow)
{
	m_bShowBackGround = bShow;
	m_pBackGround->SetVisible(bShow);
}

void CViewport::HideVoiceLabels(void)
{
	GetClientVoiceHud()->HideLabels();
}

void CViewport::Init(void)
{
	m_pHudLayer->Init();

	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->Init();

	HideAllVGUIMenu();
}

void CViewport::VidInit(void)
{
	m_pHudLayer->VidInit();

	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->VidInit();

	StopProgressBar();
	HideAllVGUIMenu();
}

void CViewport::Reset(void)
{
	m_pHudLayer->Reset();

	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->Reset();
}

void CViewport::Think(void)
{
	m_flCurrentTime = gEngfuncs.GetAbsoluteTime();

	if (!m_pTextWindow->IsVisible() && m_pTextWindow->IsExited())
	{
		if (m_pPanelShow)
		{
			ShowPanel(m_pPanelShow, true);
			m_pPanelShow = NULL;
		}
	}

	if (m_pScoreBoard->IsVisible() && m_pScoreBoard->NeedsUpdate())
		m_pScoreBoard->Update();

	if (m_pSpectatorGUI->IsVisible() && m_pSpectatorGUI->NeedsUpdate())
		m_pSpectatorGUI->Update();
}

void CViewport::HideAllVGUIMenu(void)
{
	ShowPanel(PANEL_ALL, false);
	ShowPanel(m_pTextWindow, false);
}

bool CViewport::ShowVGUIMenu(int iMenu)
{
	CViewPortPanel *panel = NULL;

	switch (iMenu)
	{
		case MENU_CLASS_T: panel = m_pClassMenu_TER; break;
		case MENU_CLASS_CT: panel = m_pClassMenu_CT; break;
		case MENU_TEAM: panel = m_pTeamMenu; break;

		case MENU_MAPBRIEFING:
		{
			m_pTextWindow->ShowMapBriefing();
			return true;
		}

		case MENU_SPECHELP:
		{
			m_pTextWindow->ShowSpectateHelp();
			return true;
		}

		case MENU_BUY:
		case MENU_BUY_PISTOL:
		case MENU_BUY_SHOTGUN:
		case MENU_BUY_RIFLE:
		case MENU_BUY_SUBMACHINEGUN:
		case MENU_BUY_MACHINEGUN:
		case MENU_BUY_ITEM:
		{
			CCSBaseBuyMenu *buyMenu = NULL;

			if (g_iTeamNumber == TEAM_CT)
				buyMenu = m_pBuyMenu_CT;
			else
				buyMenu = m_pBuyMenu_TER;

			buyMenu->ActivateMenu(iMenu);
			return true;
		}
	}

	if (panel)
	{
		ShowPanel(panel, true);
		return true;
	}

	return false;
}

bool CViewport::HideVGUIMenu(int iMenu)
{
	CViewPortPanel *panel = NULL;

	switch (iMenu)
	{
		case MENU_CLASS_T: panel = m_pClassMenu_TER; break;
		case MENU_CLASS_CT: panel = m_pClassMenu_CT; break;
		case MENU_TEAM: panel = m_pTeamMenu; break;

		case MENU_MAPBRIEFING:
		case MENU_SPECHELP:
		{
			ShowPanel(m_pTextWindow, false);
			return true;
		}

		case MENU_BUY:
		case MENU_BUY_PISTOL:
		case MENU_BUY_SHOTGUN:
		case MENU_BUY_RIFLE:
		case MENU_BUY_SUBMACHINEGUN:
		case MENU_BUY_MACHINEGUN:
		case MENU_BUY_ITEM:
		{
			if (g_iTeamNumber == TEAM_CT)
				panel = m_pBuyMenu_CT;
			else
				panel = m_pBuyMenu_TER;

			break;
		}
	}

	if (panel)
	{
		ShowPanel(panel, false);
		return true;
	}

	return false;
}

void CViewport::ActivateClientUI(void)
{
	SetVisible(true);
}

void CViewport::HideClientUI(void)
{
	SetVisible(false);
}

bool CViewport::IsInLevel(void)
{
	const char *levelName = gEngfuncs.pfnGetLevelName();

	if (levelName && levelName[0])
		return true;

	return false;
}

bool CViewport::IsInMultiplayer(void)
{
	return (IsInLevel() && gEngfuncs.GetMaxClients() > 1);
}

bool CViewport::IsVIPMap(void)
{
	const char *mapName = engine->pfnGetLevelName();

	if (!mapName)
		return false;

	if (!Q_strncmp(mapName, "maps/as_", 8))
		return true;

	return false;
}

bool CViewport::IsBombDefuseMap(void)
{
	const char *mapName = engine->pfnGetLevelName();

	if (!mapName)
		return false;

	if (!Q_strncmp(mapName, "maps/de_", 8))
		return true;

	return false;
}

char *CViewport::GetServerName(void)
{
	return m_szServerName;
}

float CViewport::GetCurrentTime(void)
{
	return m_flCurrentTime;
}

void CViewport::ShowSpectatorGUI(void)
{
	if (m_pSpectatorGUI->NeedsUpdate())
		m_pSpectatorGUI->Update();

	m_pSpectatorGUI->ShowPanel(true);
}

void CViewport::ShowSpectatorGUIBar(void)
{
	if (m_pSpectatorMenu->NeedsUpdate())
		m_pSpectatorMenu->Update();

	m_pSpectatorMenu->ShowPanel(true);
}

void CViewport::HideSpectatorGUI(void)
{
	if (gViewPortInterface->GetClientDllInterface()->IsSpectator())
		DuckMessage("#Spec_Duck");

	m_pSpectatorGUI->ShowPanel(false);
}

void CViewport::UpdateSpectatorGUI(void)
{
	if (m_pSpectatorGUI->IsVisible() && m_pSpectatorGUI->NeedsUpdate())
		m_pSpectatorGUI->Update();
}

void CViewport::UpdateSpectatorGUIBar(void)
{
	if (m_pSpectatorMenu->IsVisible() && m_pSpectatorMenu->NeedsUpdate())
		m_pSpectatorMenu->Update();
}

void CViewport::UpdateSpectatorPanel(void)
{
	UpdateSpectatorGUI();
	UpdateSpectatorGUIBar();
}

void CViewport::DeactivateSpectatorGUI(void)
{
	DuckMessage("#Spec_Duck");

	m_pSpectatorMenu->ShowPanel(false);
}

bool CViewport::IsSpectatorGUIVisible(void)
{
	return m_pSpectatorGUI->IsVisible();
}

bool CViewport::IsSpectatorBarVisible(void)
{
	return m_pSpectatorMenu->IsVisible();
}

void CViewport::SetSpectatorBanner(const char *image)
{
	m_pSpectatorGUI->SetLogoImage(image);
}

int CViewport::GetSpectatorBottomBarHeight(void)
{
	return m_pSpectatorGUI->GetBottomBarHeight();
}

int CViewport::GetSpectatorTopBarHeight(void)
{
	return m_pSpectatorGUI->GetTopBarHeight();
}

bool CViewport::IsScoreBoardVisible(void)
{
	return m_pScoreBoard->IsVisible();
}

void CViewport::ShowScoreBoard(void)
{
	if (!IsInLevel())
		return;

	if (m_pScoreBoard->IsVisible())
		return;

	ShowPanel(m_pScoreBoard, true);
}

void CViewport::HideScoreBoard(void)
{
	if (!IsInLevel())
		return;

	if (!m_pScoreBoard->IsVisible())
		return;

	ShowPanel(m_pScoreBoard, false);
}

void CViewport::HideVGUIMenu(void)
{
	if (m_pActivePanel)
	{
		if (m_pLastActivePanel)
		{
			m_pActivePanel = m_pLastActivePanel;
			m_pLastActivePanel = NULL;

			if (m_pActivePanel->NeedsUpdate())
				m_pActivePanel->Update();

			m_pActivePanel->ShowPanel(true);
		}

		m_pActivePanel->ShowPanel(false);
		m_pActivePanel = NULL;

		UpdateAllPanels();
	}
}

CViewPortPanel *CViewport::AddNewPanel(CViewPortPanel *pPanel, char const *pchDebugName)
{
	if (!pPanel)
	{
		if (pchDebugName)
			DevMsg("CViewport::AddNewPanel(%s): NULL panel.\n", pchDebugName);
		else
			Assert(0);

		delete pPanel;
		return NULL;
	}

	if (FindPanelByName(pPanel->GetName()) != NULL)
	{
		DevMsg("CViewport::AddNewPanel: panel with name '%s' already exists.\n", pPanel->GetName());

		delete pPanel;
		return NULL;
	}

	m_Panels.AddToTail(pPanel);

	ipanel()->SetVisible(pPanel->GetVPanel(), false);
	pPanel->SetParent(this);
	return pPanel;
}

CViewPortPanel *CViewport::FindPanelByName(const char *szPanelName)
{
	int count = m_Panels.Count();

	for (int i = 0; i < count; i++)
	{
		if (Q_strcmp(m_Panels[i]->GetName(), szPanelName) == 0)
			return m_Panels[i];
	}

	return NULL;
}

void CViewport::PostMessageToPanel(CViewPortPanel *pPanel, KeyValues *pKeyValues)
{
	PostMessage(pPanel->GetVPanel(), pKeyValues);
}

void CViewport::PostMessageToPanel(const char *pName, KeyValues *pKeyValues)
{
	if (Q_strcmp(pName, PANEL_ALL) == 0)
	{
		for (int i = 0; i < m_Panels.Count(); i++)
		{
			if (m_Panels[i]->IsDynamic())
				PostMessageToPanel(m_Panels[i], pKeyValues);
		}

		return;
	}

	CViewPortPanel *panel = NULL;

	if (Q_strcmp(pName, PANEL_ACTIVE) == 0)
		panel = m_pActivePanel;
	else
		panel = FindPanelByName(pName);

	if (!panel)
		return;

	PostMessageToPanel(panel, pKeyValues);
}

CViewPortPanel *CViewport::GetActivePanel(void)
{
	return m_pActivePanel;
}

void CViewport::ShowPanel(const char *pName, bool state)
{
	if (Q_strcmp(pName, PANEL_ALL) == 0)
	{
		if (!state)
			m_pPanelShow = NULL;

		for (int i = 0; i < m_Panels.Count(); i++)
		{
			if (m_Panels[i]->IsDynamic())
				ShowPanel(m_Panels[i], state);
		}

		return;
	}

	CViewPortPanel *panel = NULL;

	if (Q_strcmp(pName, PANEL_ACTIVE) == 0)
		panel = m_pActivePanel;
	else
		panel = FindPanelByName(pName);

	if (!panel)
		return;

	ShowPanel(panel, state);
}

void CViewport::ShowPanel(CViewPortPanel *pPanel, bool state)
{
	if (m_pTextWindow->IsVisible())
	{
		if (pPanel == m_pScoreBoard && state)
		{
			m_pTextWindow->ShowPanel(false);
		}

		if (pPanel != m_pTextWindow && pPanel != m_pScoreBoard)
		{
			if (m_pPanelShow && m_pPanelShow != pPanel)
				ShowPanel(m_pPanelShow, true);

			if (state)
				m_pPanelShow = pPanel;
			else if (m_pPanelShow == pPanel)
				m_pPanelShow = NULL;

			return;
		}
	}
	else
	{
		if (pPanel == m_pScoreBoard && !state)
		{
			if (!m_pTextWindow->IsExited())
				m_pTextWindow->ShowPanel(true);
		}
	}

	if (state)
	{
		if (pPanel->HasInputElements())
		{
			if (gEngfuncs.IsSpectateOnly())
				return;

			if (gEngfuncs.pDemoAPI->IsPlayingback())
				return;

			if ((m_pActivePanel != NULL) && (m_pActivePanel != pPanel) && (m_pActivePanel->IsVisible()))
			{
				m_pLastActivePanel = m_pActivePanel;
				m_pActivePanel->ShowPanel(false);
			}

			m_pActivePanel = pPanel;
		}
	}
	else
	{
		if (m_pActivePanel == pPanel)
			m_pActivePanel = NULL;

		if (m_pLastActivePanel)
		{
			m_pActivePanel = m_pLastActivePanel;
			m_pLastActivePanel = NULL;
			m_pActivePanel->ShowPanel(true);
		}
	}

	if (state)
		HideVoiceLabels();

	if (state && pPanel->NeedsUpdate())
		pPanel->Update();

	pPanel->ShowPanel(state);
}

void CViewport::UpdateAllPanels(void)
{
	int count = m_Panels.Count();

	for (int i = 0; i < count; i++)
	{
		CViewPortPanel *p = m_Panels[i];

		if (p->IsVisible() && p->NeedsUpdate())
			p->Update();
	}
}

void CViewport::RemoveAllPanels(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
	{
		VPANEL vPanel = m_Panels[i]->GetVPanel();
		ipanel()->DeletePanel(vPanel);
	}

	if (m_pBackGround)
	{
		m_pBackGround->MarkForDeletion();
		m_pBackGround = NULL;
	}

	m_Panels.Purge();
	m_pActivePanel = NULL;
	m_pLastActivePanel = NULL;
}

void CViewport::StartMessageMode(void)
{
	m_pChatDialog->StartMessageMode(MM_SAY);
}

void CViewport::StartMessageMode2(void)
{
	m_pChatDialog->StartMessageMode(MM_SAY_TEAM);
}

bool CViewport::SelectMenuItem(int menu_item)
{
	return m_pHudLayer->SelectMenuItem(menu_item);
}

void CViewport::UpdateMapSprite(void)
{
	m_pHudLayer->UpdateMapSprite();
}

void CViewport::RenderMapSprite(void)
{
	m_pHudLayer->RenderMapSprite();
}

void CViewport::CalcRefdef(struct ref_params_s *pparams)
{
	m_pHudLayer->CalcRefdef(pparams);
}

void CViewport::ShowCommandMenu(void)
{
	int x, y, w, h, sw, sh;

	surface()->GetScreenSize(sw, sh);

	m_pCommandMenu->UpdateMenu();
	m_pCommandMenu->GetSize(w, h);

	x = 0;
	y = (sh - h) / 2;

	m_pCommandMenu->SetPos(x, y);
	m_pCommandMenu->SetVisible(true);
}

void CViewport::UpdateCommandMenu(void)
{
	m_pCommandMenu->UpdateMenu();
}

void CViewport::HideCommandMenu(void)
{
	m_pCommandMenu->SetVisible(false);
}

int CViewport::IsCommandMenuVisible(void)
{
	return m_pCommandMenu->IsVisible();
}

Color CViewport::GetTeamColor(int index)
{
	return m_TeamColors[index];
}

char *CViewport::GetTeamName(int index)
{
	return gViewPortInterface->GetTeamName(index);
}

int CViewport::GetTeamScore(int index)
{
	return m_iTeamScores[index];
}

void CViewport::StartProgressBar(const char *title, int numTicks, int startTicks, bool isTimeBased)
{
	m_pNavProgress->ShowPanel(true);
	m_pNavProgress->Init(title, numTicks, startTicks);
}

void CViewport::UpdateProgressBar(const char *statusText, int tick)
{
	m_pNavProgress->Update(statusText, tick);
}

void CViewport::StopProgressBar(void)
{
	m_pNavProgress->ShowPanel(false);
}

void CViewport::PrintText(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4)
{
	m_pChatDialog->Print(msg_dest, msg_name, param1, param2, param3, param4);
}

bool CViewport::AllowedToPrintText(void)
{
	if (!gViewPortInterface->AllowedToPrintText())
		return false;

	if (m_pTextWindow->IsVisible())
		return false;

	if (m_pActivePanel)
	{
		if (m_pActivePanel->IsVisible() && m_pActivePanel->HasInputElements())
			return false;
	}

	return true;
}

int CViewport::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	if (m_pHudLayer->FireMessage(pszName, iSize, pbuf))
		return 1;

	if (!strcmp(pszName, "SayText"))
	{
		m_pChatDialog->MsgFunc_SayText();
		return 1;
	}

	if (!strcmp(pszName, "TextMsg"))
	{
		m_pChatDialog->MsgFunc_TextMsg();
		return 1;
	}

	if (!strcmp(pszName, "VGUIMenu"))
	{
		int iMenu = READ_BYTE();

		return ShowVGUIMenu(iMenu) ? 1 : 0;
	}

	if (!strcmp(pszName, "MOTD"))
	{
		m_pTextWindow->MsgFunc_MOTD();
		return 1;
	}

	if (!strcmp(pszName, "TimeLeft"))
	{
		m_pScoreBoard->MsgFunc_TimeLeft();
		return 1;
	}

	if (!strcmp(pszName, "TeamScore"))
	{
		char *teamname = READ_STRING();
		short score = READ_SHORT();

		if (teamname[0] == 'T')
		{
			m_iTeamScores[TEAM_TERRORIST] = score;
		}
		else if (teamname[0] == 'C')
		{
			m_iTeamScores[TEAM_CT] = score;
		}

		return 1;
	}

	if (!strcmp(pszName, "ServerName"))
	{
		strncpy(m_szServerName, READ_STRING(), MAX_SERVERNAME_LENGTH - 1);
		m_szServerName[MAX_SERVERNAME_LENGTH - 1] = 0;
		m_pScoreBoard->SetServerName(m_szServerName);
		return 0;
	}

	return 0;
}