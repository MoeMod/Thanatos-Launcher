#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#define PANEL_CHAT "chat"

enum
{
	MM_NONE = 0,
	MM_SAY,
	MM_SAY_TEAM,
};

enum TextColor
{
	COLOR_NORMAL = 1,
	COLOR_USEOLDCOLORS = 2,
	COLOR_PLAYERNAME = 3,
	COLOR_LOCATION = 4,
	COLOR_MAX
};

struct TextRange
{
	int start;
	int end;
	Color color;
};

void StripEndNewlineFromString(char *str);
void StripEndNewlineFromString(wchar_t *str);
char *LookupString(const char *msg, int *msg_dest = NULL);

char *ConvertCRtoNL(char *str);
wchar_t *ConvertCRtoNL(wchar_t *str);
wchar_t *ReadLocalizedString(wchar_t *pOut, int outSize, bool bStripNewline, char *originalString = NULL, int originalSize = 0);
wchar_t *ReadChatTextString(wchar_t *pOut, int outSize);
char *RemoveColorMarkup(char *str);

extern Color g_ColorBlue;
extern Color g_ColorRed;
extern Color g_ColorGreen;
extern Color g_ColorDarkGreen;
extern Color g_ColorYellow;
extern Color g_ColorGrey;

#define CHATLINE_NUM_FLASHES 0.01f
#define CHATLINE_FLASH_TIME 0.01f
#define CHATLINE_FADE_TIME 0.01f

#define CHAT_HISTORY_FADE_TIME 0.0f
#define CHAT_HISTORY_IDLE_TIME 0.01f
#define CHAT_HISTORY_IDLE_FADE_TIME 0.01f
#define CHAT_HISTORY_ALPHA 0

#include "vgui_controls/RichText.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/TextEntry.h"

class CChatDialogLine : public vgui::RichText
{
	typedef vgui::RichText BaseClass;

public:
	CChatDialogLine(vgui::Panel *parent, const char *panelNam);
	~CChatDialogLine(void);

public:
	void SetExpireTime(void);
	bool IsReadyToExpire(void);
	void Expire(void);
	float GetStartTime(void);
	int GetCount(void);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformFadeout(void);
	virtual void InsertAndColorizeText(wchar_t *buf, int clientIndex);
	virtual void Colorize(int alpha = 255);

public:
	vgui::HFont GetFont(void) { return m_hFont; }
	Color GetTextColor(void) { return m_clrText; }
	void SetNameLength(int iLength) { m_iNameLength = iLength; }
	void SetNameColor(Color cColor) { m_clrNameColor = cColor; }
	void SetNameStart(int iStart) { m_iNameStart = iStart; }

protected:
	int m_iNameLength;
	vgui::HFont m_hFont;

	Color m_clrText;
	Color m_clrNameColor;

	float m_flExpireTime;

	CUtlVector<TextRange> m_textRanges;
	wchar_t *m_text;

	int m_iNameStart;

private:
	float m_flStartTime;
	int m_nCount;
	vgui::HFont m_hFontMarlett;

private:
	CChatDialogLine(const CChatDialogLine &);
};

class CChatDialogHistory : public vgui::RichText
{
	DECLARE_CLASS_SIMPLE(CChatDialogHistory, vgui::RichText);

public:
	CChatDialogHistory(vgui::Panel *pParent, const char *panelName);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint(void);
};

class CChatDialogInputLine;

class CChatDialog : public vgui::Frame, public CViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CChatDialog, vgui::Frame);

public:
	enum
	{
		CHAT_INTERFACE_LINES = 6,
		MAX_CHARS_PER_LINE = 128
	};

public:
	CChatDialog(void);

public:
	virtual void CreateChatInputLine(void);
	virtual void CreateChatLines(void);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Printf(const char *fmt, ...);
	virtual void ChatPrintf(int iPlayerIndex, const char *fmt, ...);
	virtual void ChatPrintf(int iPlayerIndex, const wchar_t *string);
	virtual int GetChatInputOffset(void);

public:
	virtual void MsgFunc_SayText(void);
	virtual void MsgFunc_TextMsg(void);

public:
	void StartMessageMode(int iMessageModeType);
	void StopMessageMode(void);
	void Send(void);
	void Clear(void);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint(void);
	virtual void OnThink(void);
	virtual void Reset(void);

public:
	virtual const char *GetName(void) { return PANEL_CHAT; }
	virtual void SetData(KeyValues *data) {}
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);
	virtual bool IsDynamic(void) { return true; }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	static int m_nLineCounter;

public:
	vgui::Panel *GetInputPanel(void);
	CChatDialogHistory *GetChatHistory(void);
	void FadeChatHistory(void);

public:
	int m_iAlpha;
	float m_flHistoryFadeTime;
	float m_flHistoryIdleTime;
	int m_iHistoryAlpha;

public:
	CChatDialogInputLine *GetChatInput(void) { return m_pChatInput; }

public:
	virtual Color GetTextColorForClient(TextColor colorNum, int clientIndex);
	virtual Color GetClientColor(int clientIndex);

protected:
	CChatDialogLine *FindUnusedChatLine(void);

protected:
	CChatDialogInputLine *m_pChatInput;
	CChatDialogLine *m_ChatLine;
	int m_iFontHeight;
	CChatDialogHistory *m_pChatHistory;

private:
	int ComputeBreakChar(int width, const char *text, int textlen);

private:
	int m_nMessageMode;
	vgui::HFont m_hChatFont;
};

class CChatDialogEntry : public vgui::TextEntry
{
	typedef vgui::TextEntry BaseClass;

public:
	CChatDialogEntry(vgui::Panel *parent, char const *panelName, CChatDialog *pChat) : BaseClass(parent, panelName)
	{
		SetCatchEnterKey(true);
		SetAllowNonAsciiCharacters(true);

		m_pChatDialog = pChat;
	}

	virtual void OnKeyCodeTyped(vgui::KeyCode code)
	{
		if (code == vgui::KEY_ENTER || code == vgui::KEY_ESCAPE)
		{
			if (code != vgui::KEY_ESCAPE)
			{
				if (m_pChatDialog)
					m_pChatDialog->Send();
			}

			if (m_pChatDialog)
				m_pChatDialog->StopMessageMode();
		}
		else if (code == vgui::KEY_TAB)
		{
			return;
		}
		else
			BaseClass::OnKeyCodeTyped(code);
	}

private:
	CChatDialog *m_pChatDialog;
};

class CChatDialogInputLine : public vgui::Panel
{
	typedef vgui::Panel BaseClass;

public:
	CChatDialogInputLine(CChatDialog *parent, char const *panelName);

public:
	void SetPrompt(const char *prompt);
	void ClearEntry(void);
	void SetEntry(const wchar_t *entry);
	void GetMessageText(wchar_t *buffer, int buffersizebytes);

public:
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	vgui::Panel *GetInputPanel(void);
	virtual vgui::VPANEL GetCurrentKeyFocus(void) { return m_pInput->GetVPanel(); }
	vgui::TextEntry *GetPrompt(void) { return m_pPrompt; }

public:
	vgui::TextEntry *m_pPrompt;
	CChatDialogEntry *m_pInput;
};

#endif