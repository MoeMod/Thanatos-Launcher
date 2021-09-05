#ifndef TEAMMENU_H
#define TEAMMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <UtlVector.h>
#include "mouseoverpanelbutton.h"

#define PANEL_TEAM "team"

namespace vgui2
{
	class RichText;
	class HTML;
}

class TeamFortressViewport;

class CTeamMenu : public vgui2::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CTeamMenu, vgui2::Frame);

public:
	CTeamMenu(void);
	virtual ~CTeamMenu(void);

public:
	virtual void Init(void) {}
	virtual void VidInit(void) {}
	virtual void Reset(void);

public:
	virtual const char *GetName(void) { return PANEL_TEAM; }
	virtual void SetData(KeyValues *data) {}
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);
	virtual bool IsDynamic(void) { return true; }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	void AutoAssign(void);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);
	virtual void SetLabelText(const char *textEntryName, const char *text);
	virtual void LoadMapPage(const char *mapName);

protected:
	virtual vgui2::Panel *CreateControlByName(const char *controlName);
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel);

protected:
	vgui2::RichText *m_pMapInfo;
	vgui2::HTML *m_pMapInfoHTML;
	vgui2::KeyCode m_iJumpKey;
	vgui2::KeyCode m_iScoreBoardKey;
	char m_szMapName[MAX_PATH];
	vgui2::EditablePanel *m_pPanel;
	CUtlVector<MouseOverPanelButton *> m_mouseoverButtons;
};

#endif