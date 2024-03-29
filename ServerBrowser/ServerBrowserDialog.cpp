
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISystem.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include "FileSystem.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/FocusNavGroup.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/AnimationController.h>

#include "inetapi.h"
#include "msgbuffer.h"
#include "proto_oob.h"
#include "ServerContextMenu.h"
#include "socket.h"
#include "ServerBrowserDialog.h"
#include "DialogGameInfo.h"

#include "InternetGames.h"
#include "FavoriteGames.h"
#include "SpectateGames.h"
#include "LanGames.h"
#include "HistoryGames.h"

using namespace vgui2;

static CServerBrowserDialog *s_InternetDlg = NULL;

CServerBrowserDialog &ServerBrowserDialog(void)
{
	return *CServerBrowserDialog::GetInstance();
}

void GetMostCommonQueryPorts(CUtlVector<uint16> &ports)
{
	for (int i = 0; i <= 5; i++)
	{
		ports.AddToTail(27015 + i);
		ports.AddToTail(26900 + i);
	}
#ifdef _DEBUG
	ports.AddToTail(4242);
#endif
}

CServerBrowserDialog::CServerBrowserDialog(vgui2::Panel *parent) : Frame(parent, "CServerBrowserDialog")
{
	s_InternetDlg = this;

	m_szGameName[0] = 0;
	m_szModDir[0] = 0;
	m_pSavedData = NULL;
	m_pFilterData = NULL;
	m_pFavorites = NULL;
	m_pHistory = NULL;

	LoadUserData();

	m_pFavorites = new CFavoriteGames(this);
	m_pHistory = new CHistoryGames(this);
	m_pLanGames = new CLanGames(this);

	SetMinimumSize(640, 384);
	SetSize(640, 384);
	SetVisible(false);

	m_pGameList = m_pFavorites;
	m_pContextMenu = new CServerContextMenu(this);
	m_pContextMenu->SetVisible(false);

	m_pTabPanel = new PropertySheet(this, "GameTabs");
	m_pTabPanel->SetTabWidth(72);
	m_pTabPanel->AddPage(m_pFavorites, "#ServerBrowser_FavoritesTab");
	m_pTabPanel->AddPage(m_pHistory, "#ServerBrowser_HistoryTab");
	m_pTabPanel->AddPage(m_pLanGames, "#ServerBrowser_LanTab");
	m_pTabPanel->AddActionSignalTarget(this);

	m_pStatusLabel = new Label(this, "StatusLabel", "");

	LoadControlSettings("Servers/DialogServerBrowser.res");

	m_pStatusLabel->SetText("");

	const char *gameList = m_pSavedData->GetString("GameList");

	if (!Q_stricmp(gameList, "favorites"))
		m_pTabPanel->SetActivePage(m_pFavorites);
	else if (!Q_stricmp(gameList, "history"))
		m_pTabPanel->SetActivePage(m_pHistory);
	else
		m_pTabPanel->SetActivePage(m_pLanGames);
	
	
	m_pInternetGames = new CInternetGames(this);
	m_pSpectateGames = new CSpectateGames(this);

	m_pTabPanel->AddPage(m_pInternetGames, "#ServerBrowser_InternetTab");
	m_pTabPanel->AddPage(m_pSpectateGames, "#ServerBrowser_SpectateTab");

	if (!Q_stricmp(gameList, "spectate"))
		m_pTabPanel->SetActivePage(m_pSpectateGames);
}

CServerBrowserDialog::~CServerBrowserDialog(void)
{
	delete m_pContextMenu;

	SaveUserData();

	if (m_pSavedData)
		m_pSavedData->deleteThis();
}

void CServerBrowserDialog::Initialize(void)
{
	SetTitle("#ServerBrowser_Servers", true);
	SetVisible(false);
}

serveritem_t &CServerBrowserDialog::GetServer(unsigned int serverID)
{
	return m_pGameList->GetServer(serverID);
}

void CServerBrowserDialog::Open(void)
{
	Activate();

	m_pTabPanel->RequestFocus();

	ivgui()->PostMessage(m_pTabPanel->GetActivePage()->GetVPanel(), new KeyValues("PageShow"), GetVPanel());
}

void CServerBrowserDialog::LoadUserData(void)
{
	if (m_pSavedData)
		m_pSavedData->deleteThis();

	m_pSavedData = new KeyValues("ServerBrowser");

	if (!m_pSavedData->LoadFromFile(vgui2::filesystem(), "config/ServerBrowser.vdf"))
	{
	}

	KeyValues *filters = m_pSavedData->FindKey("Filters", false);

	if (filters)
	{
		m_pFilterData = filters->MakeCopy();
		m_pSavedData->RemoveSubKey(filters);
	}
	else
		m_pFilterData = new KeyValues("Filters");

	if (m_pHistory)
	{
		KeyValues *historys = m_pSavedData->FindKey("Historys", true);
		m_pHistory->LoadHistorysList(historys);
	}

	if (m_pFavorites)
	{
		KeyValues *favorites = m_pSavedData->FindKey("Favorites", true);
		m_pFavorites->LoadFavoritesList(favorites);

		ReloadFilterSettings();
	}

	InvalidateLayout();
	Repaint();
}

void CServerBrowserDialog::SaveUserData(void)
{
	m_pSavedData->Clear();
	m_pSavedData->LoadFromFile(vgui2::filesystem(), "config/ServerBrowser.vdf");

	if (m_pGameList == m_pSpectateGames)
		m_pSavedData->SetString("GameList", "spectate");
	else if (m_pGameList == m_pFavorites)
		m_pSavedData->SetString("GameList", "favorites");
	else if (m_pGameList == m_pLanGames)
		m_pSavedData->SetString("GameList", "lan");
	else if (m_pGameList == m_pHistory)
		m_pSavedData->SetString("GameList", "history");
	else
		m_pSavedData->SetString("GameList", "internet");

	KeyValues *favorites = m_pSavedData->FindKey("Favorites", true);
	m_pFavorites->SaveFavoritesList(favorites);

	KeyValues *historys = m_pSavedData->FindKey("Historys", true);
	m_pHistory->SaveHistorysList(historys);

	m_pSavedData->RemoveSubKey(m_pSavedData->FindKey("Filters"));
	m_pSavedData->AddSubKey(m_pFilterData->MakeCopy());
	m_pSavedData->SaveToFile(vgui2::filesystem(), "config/ServerBrowser.vdf");
}

void CServerBrowserDialog::RefreshCurrentPage(void)
{
	if (m_pGameList)
		m_pGameList->StartRefresh();
}

void CServerBrowserDialog::UpdateStatusText(const char *fmt, ...)
{
	if (!m_pStatusLabel)
		return;

	if (fmt && strlen(fmt) > 0)
	{
		char str[1024];
		va_list argptr;
		va_start(argptr, fmt);
		_vsnprintf(str, sizeof(str), fmt, argptr);
		va_end(argptr);

		m_pStatusLabel->SetText(str);
	}
	else
		m_pStatusLabel->SetText("");
}

void CServerBrowserDialog::UpdateStatusText(wchar_t *unicode)
{
	if (!m_pStatusLabel)
		return;

	if (unicode && wcslen(unicode) > 0)
		m_pStatusLabel->SetText(unicode);
	else
		m_pStatusLabel->SetText("");
}

void CServerBrowserDialog::OnGameListChanged(void)
{
	m_pGameList = dynamic_cast<IGameList *>(m_pTabPanel->GetActivePage());

	UpdateStatusText("");
	InvalidateLayout();
	Repaint();
}

Panel *CServerBrowserDialog::GetActivePage(void)
{
	return m_pTabPanel->GetActivePage();
}

CServerBrowserDialog *CServerBrowserDialog::GetInstance(void)
{
	return s_InternetDlg;
}

void CServerBrowserDialog::AddServerToFavorites(serveritem_t &server)
{
	m_pFavorites->AddNewServer(server);
	m_pFavorites->StartRefresh();
}

CServerContextMenu *CServerBrowserDialog::GetContextMenu(vgui2::Panel *pPanel)
{
	if (m_pContextMenu)
		delete m_pContextMenu;

	m_pContextMenu = new CServerContextMenu(this);
	m_pContextMenu->SetAutoDelete(false);

	if (!pPanel)
		m_pContextMenu->SetParent(this);
	else
		m_pContextMenu->SetParent(pPanel);

	m_pContextMenu->SetVisible(false);
	return m_pContextMenu;
}

CDialogGameInfo *CServerBrowserDialog::JoinGame(IGameList *gameList, unsigned int serverIndex)
{
	CDialogGameInfo *gameDialog = OpenGameInfoDialog(gameList, serverIndex);

	gameDialog->ConnectToServer();
	gameDialog->Close();
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::JoinGame(int serverIP, int serverPort, const char *titleName)
{
	CDialogGameInfo *gameDialog = OpenGameInfoDialog(serverIP, serverPort, titleName);

	gameDialog->ConnectToServer();
	gameDialog->Close();
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::OpenGameInfoDialog(IGameList *gameList, unsigned int serverIndex)
{
	serveritem_t &server = gameList->GetServer(serverIndex);

	CDialogGameInfo *gameDialog = new CDialogGameInfo(&ServerBrowserDialog(), server);
	gameDialog->AddActionSignalTarget(this);
	gameDialog->Run(server.name);
	gameDialog->MoveToCenterOfScreen();
	gameDialog->DoModal();

	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::OpenGameInfoDialog(int serverIP, int serverPort, const char *titleName)
{
	serveritem_t server;
	memset(&server, 0, sizeof(server));
	*((int *)server.ip) = serverIP;
	server.port = serverPort;

	CDialogGameInfo *gameDialog = new CDialogGameInfo(&ServerBrowserDialog(), server);
	gameDialog->AddActionSignalTarget(this);
	gameDialog->Run(titleName);
	gameDialog->MoveToCenterOfScreen();
	gameDialog->DoModal();

	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

void CServerBrowserDialog::CloseAllGameInfoDialogs(void)
{
	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui2::Panel *dlg = m_GameInfoDialogs[i];

		if (dlg)
			vgui2::ivgui()->PostMessage(dlg->GetVPanel(), new KeyValues("Close"), NULL);
	}
}

KeyValues *CServerBrowserDialog::GetFilterSaveData(const char *filterSet)
{
	return m_pFilterData->FindKey(filterSet, true);
}

const char *CServerBrowserDialog::GetActiveModName(void)
{
	return m_szModDir[0] ? m_szModDir : NULL;
}

const char *CServerBrowserDialog::GetActiveGameName(void)
{
	return m_szGameName[0] ? m_szGameName : NULL;
}

void CServerBrowserDialog::OnActiveGameName(KeyValues *pKV)
{
	Q_strncpy(m_szModDir, pKV->GetString("name"), sizeof(m_szModDir));
	Q_strncpy(m_szGameName, pKV->GetString("game"), sizeof(m_szGameName));

	ReloadFilterSettings();
}

void CServerBrowserDialog::ReloadFilterSettings(void)
{
	//m_pInternetGames->LoadFilterSettings();
	//m_pSpectateGames->LoadFilterSettings();
	m_pFavorites->LoadFilterSettings();
	m_pLanGames->LoadFilterSettings();
	m_pHistory->LoadFilterSettings();
}

void CServerBrowserDialog::OnConnectToGame(KeyValues *pMessageValues)
{
	int ip = pMessageValues->GetInt("ip");
	int connectionPort = pMessageValues->GetInt("connectionport");

	if (!ip)
		return;

	memset(&m_CurrentConnection, NULL, sizeof(serveritem_t));
	*(int *)m_CurrentConnection.ip = ip;
	m_CurrentConnection.port = connectionPort;

	if (m_pHistory)
	{
		m_pHistory->AddNewServer(m_CurrentConnection);
		m_pHistory->SetRefreshOnReload();
	}

	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui2::Panel *dlg = m_GameInfoDialogs[i];

		if (dlg)
		{
			KeyValues *kv = new KeyValues("ConnectedToGame", "ip", ip, "connectionport", connectionPort);
			vgui2::ivgui()->PostMessage(dlg->GetVPanel(), kv, NULL);
		}
	}

	m_pFavorites->OnConnectToGame();
	m_bCurrentlyConnected = true;
}

void CServerBrowserDialog::OnDisconnectFromGame(void)
{
	m_bCurrentlyConnected = false;
	memset(&m_CurrentConnection, NULL, sizeof(serveritem_t));

	m_pFavorites->OnDisconnectFromGame();
}

void CServerBrowserDialog::ActivateBuildMode(void)
{
	EditablePanel *panel = dynamic_cast<EditablePanel *>(m_pTabPanel->GetActivePage());

	if (!panel)
		return;

	panel->ActivateBuildMode();
}

bool CServerBrowserDialog::GetDefaultScreenPosition(int &x, int &y, int &wide, int &tall)
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);

	x = wx + (int)(ww * 0.05);
	y = wy + (int)(wt * 0.4);
	wide = (int)(ww * 0.5);
	tall = (int)(wt * 0.55);

	return true;
}