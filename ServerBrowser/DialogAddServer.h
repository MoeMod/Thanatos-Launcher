#ifndef DIALOGADDSERVER_H
#define DIALOGADDSERVER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include "ServerList.h"
#include <ServerBrowser/IServerRefreshResponse.h>

class CBaseGamesPage;
class IGameList;

class CAddServerGameList : public vgui2::ListPanel
{
public:
	DECLARE_CLASS_SIMPLE(CAddServerGameList, vgui2::ListPanel);

public:
	CAddServerGameList(vgui2::Panel *parent, const char *panelName);

public:
	virtual void OnKeyCodeTyped(vgui2::KeyCode code);
};

class CDialogAddServer : public vgui2::Frame, public IServerRefreshResponse
{
public:
	DECLARE_CLASS_SIMPLE(CDialogAddServer, vgui2::Frame);

public:
	friend class CAddServerGameList;

public:
	CDialogAddServer(vgui2::Panel *parent);
	~CDialogAddServer(void);

public:
	void StartRefresh(void);
	void StopRefresh(void);
	bool IsRefreshing(void);
	void AddNewServer(netadr_t address);
	void AddNewServer(serveritem_t &server);
	void ServerResponded(serveritem_t &server);
	void ServerFailedToRespond(serveritem_t &server);
	void RefreshComplete(void);
	void TestServers(void);

public:
	void ApplySchemeSettings(vgui2::IScheme *pScheme);

public:
	MESSAGE_FUNC(OnItemSelected, "ItemSelected");

private:
	void OnCommand(const char *command);
	void OnOK(void);
	void OnTick(void);
	void OnKeyCodeTyped(vgui2::KeyCode code);

	MESSAGE_FUNC(OnTextChanged, "TextChanged");

private:
	vgui2::Button *m_pTestServersButton;
	vgui2::Button *m_pAddServerButton;
	vgui2::Button *m_pAddSelectedServerButton;

	vgui2::PropertySheet *m_pTabPanel;
	vgui2::TextEntry *m_pTextEntry;
	CAddServerGameList *m_pDiscoveredGames;
	int m_OriginalHeight;
	CServerList m_Servers;
	CUtlVector<serveritem_t> m_DiscoveredServers;
};

#endif