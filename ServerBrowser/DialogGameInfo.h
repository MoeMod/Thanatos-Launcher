#ifndef DIALOGGAMEINFO_H
#define DIALOGGAMEINFO_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include "ServerList.h"
#include "getchallenge.h"
#include "playerinfo.h"
#include <ServerBrowser/IServerRefreshResponse.h>
#include <ServerBrowser/IServerGetChallengeResponse.h>
#include <ServerBrowser/IServerPlayersResponse.h>

class CDialogGameInfo : public vgui2::Frame, public IServerRefreshResponse, public IServerGetChallengeResponse, public IServerPlayersResponse
{
	DECLARE_CLASS_SIMPLE(CDialogGameInfo, vgui2::Frame); 

public:
	CDialogGameInfo(vgui2::Panel *parent, serveritem_t &server);
	~CDialogGameInfo(void);

public:
	void Run(const char *titleName);
	void ChangeGame(int serverIP, int serverPort);
	void Connect(void);
	void ConnectToServer(void);

public:
	virtual void ServerResponded(serveritem_t &server);
	virtual void ServerFailedToRespond(serveritem_t &server);
	virtual void GetChallengeFailedToRespond(void);
	virtual void GetChallengeRefreshComplete(int challenge);
	virtual void AddPlayerToList(const char *playerName, int score, float timePlayedSeconds);
	virtual void PlayersFailedToRespond(void);
	virtual void PlayersRefreshComplete(void);
	virtual void RefreshComplete(void);
	virtual void ClearPlayerList(void);
	virtual void SendPlayerQuery(void);

protected:
	MESSAGE_FUNC(OnConnect, "Connect");
	MESSAGE_FUNC(OnRefresh, "Refresh");
	MESSAGE_FUNC_PTR(OnButtonToggled, "ButtonToggled", panel);
	MESSAGE_FUNC_PTR(OnRadioButtonChecked, "RadioButtonChecked", panel)
	{
		OnButtonToggled(panel);
	}

	MESSAGE_FUNC_CHARPTR(OnJoinServerWithPassword, "JoinServerWithPassword", password);
	MESSAGE_FUNC_INT_INT(OnConnectToGame, "ConnectedToGame", ip, port);

protected:
	virtual void OnKeyCodeTyped(vgui2::KeyCode code);
	virtual void OnTick(void);
	virtual void PerformLayout(void);

private:
	long m_iRequestRetry;

private:
	static int PlayerTimeColumnSortFunc(vgui2::ListPanel *pPanel, const vgui2::ListPanelItem &p1, const vgui2::ListPanelItem &p2);

private:
	void RequestInfo(void);
	void ShowAutoRetryOptions(bool state);
	void ConstructConnectArgs(char *pchOptions, int cchOptions, const serveritem_t &server);
	void ApplyConnectCommand(const serveritem_t &server);

private:
	vgui2::Button *m_pConnectButton;
	vgui2::Button *m_pCloseButton;
	vgui2::Button *m_pRefreshButton;
	vgui2::Label *m_pInfoLabel;
	vgui2::ToggleButton *m_pAutoRetry;
	vgui2::RadioButton *m_pAutoRetryAlert;
	vgui2::RadioButton *m_pAutoRetryJoin;
	vgui2::ListPanel *m_pPlayerList;

	enum { PING_TIMES_MAX = 4 };

	unsigned int m_iServerID;

	CServerList m_Servers;
	CGetChallenge m_GetChallenge;
	CPlayerInfo m_PlayerInfo;

	bool m_bConnecting;
	char m_szPassword[64];
	int m_iChallenge;

	bool m_bServerNotResponding;
	bool m_bServerFull;
	bool m_bShowAutoRetryToggle;
	bool m_bShowingExtendedOptions;
	bool m_bQueryPlayerInfo;

	bool m_bPlayerListUpdatePending;
};

#endif