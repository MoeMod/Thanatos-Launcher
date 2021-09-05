#ifndef VGUITEXTWINDOW_H
#define VGUITEXTWINDOW_H

#ifdef _WIN32
#pragma once
#endif

#define PANEL_INFO "info"

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

enum
{
	TYPE_TEXT = 0,
	TYPE_INDEX,
	TYPE_URL,
	TYPE_FILE,
};

namespace vgui2
{
	class TextEntry;
}

class CTextWindow : public vgui2::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CTextWindow, vgui2::Frame);

public:
	CTextWindow(void);
	virtual ~CTextWindow(void);

public:
	virtual const char *GetName(void) { return PANEL_INFO; }
	virtual void SetData(KeyValues *data);
	virtual void Reset(void);
	virtual void VidInit(void);
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);
	virtual bool IsDynamic(void) { return true; }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void SetData(int type, const char *title, const char *message, const char *command);
	virtual void ShowFile(const char *filename);
	virtual void ShowText(const char *text);
	virtual void ShowURL(const char *URL);
	virtual void ShowIndex(const char *entry);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);
	virtual bool IsExited(void) { return m_bIsExited; }

protected:	
	virtual void OnCommand(const char *command);

protected:

	class CHTML : public vgui2::HTML
	{
	private:
		DECLARE_CLASS_SIMPLE(CHTML, vgui2::HTML);

	public:
		CHTML(Panel *parent, const char *pchName) : vgui2::HTML(parent, pchName) {}
	};

protected:	
	char m_szTitle[255];
	char m_szMessage[2048];
	char m_szExitCommand[255];
	int m_nContentType;
	bool m_bIsExited;

	vgui2::TextEntry *m_pTextMessage;
	vgui2::Button *m_pOK;
	vgui2::Label *m_pTitleLabel;

	CHTML *m_pHTML;
};

#endif