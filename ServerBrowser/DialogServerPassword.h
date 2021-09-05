#ifndef DIALOGSERVERPASSWORD_H
#define DIALOGSERVERPASSWORD_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

class CDialogServerPassword : public vgui2::Frame
{
public:
	typedef vgui2::Frame BaseClass;

public:
	CDialogServerPassword(vgui2::Panel *parent);
	~CDialogServerPassword(void);

public:
	void Activate(const char *serverName, unsigned int serverID);

private:
	virtual void OnCommand(const char *command);

private:
	vgui2::Label *m_pInfoLabel;
	vgui2::Label *m_pGameLabel;
	vgui2::TextEntry *m_pPasswordEntry;
	vgui2::Button *m_pConnectButton;

	int m_iServerID;
};

#endif