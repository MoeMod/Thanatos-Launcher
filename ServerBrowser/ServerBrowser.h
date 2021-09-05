#ifndef SERVERBROWSER_H
#define SERVERBROWSER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI2.h>
#include <ServerBrowser/IServerBrowser.h>

#include <vgui_controls/PHandle.h>

class CServerBrowserDialog;

class CServerBrowser : public IServerBrowser
{
public:
	CServerBrowser(void);
	~CServerBrowser(void);

public:
	virtual bool Initialize(CreateInterfaceFn *factorylist, int numFactories);
	virtual vgui2::VPANEL GetPanel(void);
	virtual void ActiveGameName(const char *szGameName, const char *szGameDir);
	virtual bool Activate(void);
	virtual void ConnectToGame(int ip, int connectionport);
	virtual void DisconnectFromGame(void);
	virtual void Shutdown(void);
	virtual void Deactivate(void);
	virtual void Reactivate(void);
	virtual void SetParent(vgui2::VPANEL parent);
	virtual bool JoinGame(unsigned int gameIP, unsigned int gamePort, const char *userName);
	virtual void CloseAllGameInfoDialogs(void);
	virtual void CreateDialog(void);

private:
	vgui2::DHANDLE<CServerBrowserDialog> m_hInternetDlg;
};

CServerBrowser &ServerBrowser(void);

#endif