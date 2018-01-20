#include "DialogServerPassword.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>

using namespace vgui;

CDialogServerPassword::CDialogServerPassword(vgui::Panel *parent) : Frame(parent, "DialogServerPassword")
{
	
}

CDialogServerPassword::~CDialogServerPassword(void)
{
}

void CDialogServerPassword::Activate(const char *serverName, unsigned int serverID)
{
	
	BaseClass::Activate();
}

void CDialogServerPassword::OnCommand(const char *command)
{
	bool bClose = false;

	if (!Q_stricmp(command, "Connect"))
	{
		KeyValues *msg = new KeyValues("JoinServerWithPassword");
		char buf[64];
		m_pPasswordEntry->GetText(buf, sizeof(buf)-1);
		msg->SetString("password", buf);
		msg->SetInt("serverID", m_iServerID);
		PostActionSignal(msg);

		bClose = true;
	}
	else if (!Q_stricmp(command, "Close"))
	{
		bClose = true;
	}
	else
	{
		BaseClass::OnCommand(command);
	}

	if (bClose)
		PostMessage(this, new KeyValues("Close"));
}