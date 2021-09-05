#ifndef CSTEXTWINDOW_H
#define CSTEXTWINDOW_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\vguitextwindow.h"

class CCSTextWindow : public CTextWindow
{
private:
	DECLARE_CLASS_SIMPLE(CCSTextWindow, CTextWindow);

public:
	CCSTextWindow(void);
	virtual ~CCSTextWindow(void);

public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Update(void);
	virtual void SetVisible(bool state);
	virtual void ShowPanel(bool bShow);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);

protected:
	vgui2::KeyCode m_iScoreBoardKey;

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

public:
	void ShowMapBriefing(void);
	void ShowSpectateHelp(void);

public:
	void MsgFunc_MOTD(void);

public:
	char m_szMOTD[2048];
};

#endif