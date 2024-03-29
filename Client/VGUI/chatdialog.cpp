#include "hud.h"
#include "chatdialog.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_int.h"
#include "vgui_controls/controls.h"

#include <vgui/IInputInternal.h>

inline wchar_t *CloneWString(const wchar_t *str)
{
	wchar_t *cloneStr = new wchar_t [wcslen(str) + 1];
	wcscpy(cloneStr, str);
	return cloneStr;
}

char *RemoveColorMarkup(char *str)
{
	char *out = str;

	for (char *in = str; *in != 0; ++in)
	{
		if (*in > 0 && *in < COLOR_MAX)
			continue;

		*out = *in;
		++out;
	}

	*out = 0;
	return str;
}

char *ConvertCRtoNL(char *str)
{
	for (char *ch = str; *ch != 0; ch++)
	{
		if (*ch == '\r')
			*ch = '\n';
	}

	return str;
}

wchar_t *ConvertCRtoNL(wchar_t *str)
{
	for (wchar_t *ch = str; *ch != 0; ch++)
	{
		if (*ch == L'\r')
			*ch = L'\n';
	}

	return str;
}

void StripEndNewlineFromString(char *str)
{
	int s = strlen(str) - 1;

	if (s >= 0)
	{
		if (str[s] == '\n' || str[s] == '\r')
			str[s] = 0;
	}
}

void StripEndNewlineFromString(wchar_t *str)
{
	int s = wcslen(str) - 1;

	if (s >= 0)
	{
		if (str[s] == L'\n' || str[s] == L'\r')
			str[s] = 0;
	}
}

char *LookupString(const char *msg, int *msg_dest)
{
	if (!msg)
		return "";

	if (msg[0] == '#')
	{
		client_textmessage_t *clmsg = TextMessageGet(msg + 1);

		if (!clmsg || !(clmsg->pMessage))
			return (char *)msg;

		if (msg_dest)
		{
			if (clmsg->effect < 0)
				*msg_dest = -clmsg->effect;
		}

		return (char *)clmsg->pMessage;
	}
	else
		return (char *)msg;
}

wchar_t *ReadLocalizedString(wchar_t *pOut, int outSize, bool bStripNewline, char *originalString, int originalSize)
{
	char szString[2048];
	szString[0] = 0;
	strcpy_s(szString, sizeof(szString), READ_STRING());

	if (originalString)
		Q_strncpy(originalString, szString, originalSize);

	const wchar_t *pBuf = vgui2::localize()->Find(szString);

	if (pBuf)
	{
		wcsncpy(pOut, pBuf, outSize / sizeof(wchar_t));
		pOut[outSize / sizeof(wchar_t) - 1] = 0;
	}
	else
	{
		char *str = LookupString(szString);

		if (!str[0])
			vgui2::localize()->ConvertANSIToUnicode(szString, pOut, outSize);
		else
			vgui2::localize()->ConvertANSIToUnicode(str, pOut, outSize);
	}

	if (bStripNewline)
		StripEndNewlineFromString(pOut);

	return pOut;
}

wchar_t *ReadChatTextString(wchar_t *pOut, int outSize)
{
	char szString[2048];
	szString[0] = 0;
	strcpy_s(szString, sizeof(szString), READ_STRING());

	vgui2::localize()->ConvertANSIToUnicode(szString, pOut, outSize);

	StripEndNewlineFromString(pOut);

	for (wchar_t *test = pOut; test && *test; ++test)
	{
		if (*test && (*test < COLOR_MAX))
			*test = COLOR_NORMAL;
	}

	return pOut;
}

CChatDialogLine::CChatDialogLine(vgui2::Panel *parent, const char *panelName) : vgui2::RichText(parent, panelName)
{
	m_hFont = m_hFontMarlett = 0;
	m_flExpireTime = 0.0f;
	m_flStartTime = 0.0f;
	m_iNameLength = 0;
	m_text = NULL;

	SetPaintBackgroundEnabled(true);
	SetVerticalScrollbar(false);
}

CChatDialogLine::~CChatDialogLine(void)
{
	if (m_text)
		delete [] m_text;
}

void CChatDialogLine::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("ChatFont");
	m_hFontMarlett = pScheme->GetFont("Marlett");
	m_clrText = pScheme->GetColor("FgColor", GetFgColor());

	SetFont(m_hFont);
	SetBgColor(Color(0, 0, 0, 100));
}

void CChatDialogLine::PerformFadeout(void)
{
	float curtime = g_pViewPort->GetCurrentTime();

	int lr = m_clrText[0];
	int lg = m_clrText[1];
	int lb = m_clrText[2];

	if (curtime >= m_flStartTime && curtime < m_flStartTime + CHATLINE_FLASH_TIME)
	{
		float frac1 = (curtime - m_flStartTime) / CHATLINE_FLASH_TIME;
		float frac = frac1;

		frac *= CHATLINE_NUM_FLASHES;
		frac *= 2 * M_PI;
		frac = cos(frac);
		frac = clamp(frac, 0.0f, 1.0f);
		frac *= (1.0f - frac1);

		int r = lr, g = lg, b = lb;

		r = r + (255 - r) * frac;
		g = g + (255 - g) * frac;
		b = b + (255 - b) * frac;

		int alpha = 63 + 192 * (1.0f - frac1);
		alpha = clamp(alpha, 0, 255);

		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));
		SetText("");

		InsertColorChange(Color(r, g, b, 255));
		InsertString(wbuf);
	}
	else if (curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME)
	{
		float frac = (m_flExpireTime - curtime) / CHATLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp(alpha, 0, 255);

		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));
		SetText("");

		InsertColorChange(Color(lr * frac, lg * frac, lb * frac, alpha));
		InsertString(wbuf);
	}
	else
	{
		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));
		SetText("");

		InsertColorChange(Color(lr, lg, lb, 255));
		InsertString(wbuf);
	}

	OnThink();
}

void CChatDialogLine::SetExpireTime(void)
{
	m_flStartTime = g_pViewPort->GetCurrentTime();
	m_flExpireTime = m_flStartTime + hud_saytext_time->value;
	m_nCount = CChatDialog::m_nLineCounter++;
}

int CChatDialogLine::GetCount(void)
{
	return m_nCount;
}

bool CChatDialogLine::IsReadyToExpire(void)
{
	if (!g_pViewPort->IsInMultiplayer())
		return true;

	if (g_pViewPort->GetCurrentTime() >= m_flExpireTime)
		return true;

	return false;
}

float CChatDialogLine::GetStartTime(void)
{
	return m_flStartTime;
}

void CChatDialogLine::Expire(void)
{
	SetVisible(false);
}

CChatDialogInputLine::CChatDialogInputLine(CChatDialog *parent, char const *panelName) : vgui2::Panel(parent, panelName)
{
	SetMouseInputEnabled(false);

	m_pPrompt = new vgui2::TextEntry(this, "ChatInputPrompt");
	m_pInput = new CChatDialogEntry(this, "ChatInput", parent);
	m_pInput->SetMaximumCharCount(127);
}

void CChatDialogInputLine::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui2::HFont hFont = pScheme->GetFont("ChatFont");

	m_pPrompt->SetFont(hFont);
	m_pInput->SetFont(hFont);
	m_pInput->SetFgColor(pScheme->GetColor("Chat.TypingText", pScheme->GetColor("Panel.FgColor", Color(255, 255, 255, 255))));
	m_pInput->SetMouseInputEnabled(true);

	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundEnabled(true);
}

void CChatDialogInputLine::SetPrompt(const char *prompt)
{
	Assert(m_pPrompt);

	m_pPrompt->SetText(prompt);

	InvalidateLayout();
}

void CChatDialogInputLine::ClearEntry(void)
{
	Assert(m_pInput);

	SetEntry(L"");
}

void CChatDialogInputLine::SetEntry(const wchar_t *entry)
{
	Assert(m_pInput);
	Assert(entry);

	m_pInput->SetText(entry);
}

void CChatDialogInputLine::GetMessageText(wchar_t *buffer, int buffersizebytes)
{
	m_pInput->GetText(buffer, buffersizebytes);
}

void CChatDialogInputLine::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);

	int w, h;
	m_pPrompt->GetSize(w, h);

	wchar_t text[128];
	m_pPrompt->GetText(text, sizeof(text));

	int width = 0;
	
	vgui2::HFont font = vgui2::scheme()->GetIScheme(GetScheme())->GetFont("Default", IsProportional());
	
	for (size_t i = 0; i < wcslen(text); i++)
		width += vgui2::surface()->GetCharacterWidth(font, text[i]) * 1.35;

	m_pPrompt->SetBounds(0, 0, width, tall);
	m_pInput->SetBounds(width + 2, 0, wide - width - 2, tall);
}

vgui2::Panel *CChatDialogInputLine::GetInputPanel(void)
{
	return m_pInput;
}

CChatDialogHistory::CChatDialogHistory(vgui2::Panel *pParent, const char *panelName) : BaseClass(pParent, "ChatHistory")
{
	InsertFade(-1, -1);
}

void CChatDialogHistory::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui2::HFont font = pScheme->GetFont("ChatHistoryFont");

	if (font == vgui2::INVALID_FONT)
		font = pScheme->GetFont("ChatFont");

	SetFont(font);
	SetAlpha(255);
}

void CChatDialogHistory::Paint(void)
{
	if (g_pViewPort->AllowedToPrintText())
		BaseClass::Paint();
}

int CChatDialog::m_nLineCounter = 1;

CChatDialog::CChatDialog(void) : BaseClass(NULL, PANEL_CHAT)
{
	SetZPos(-30);

	m_nMessageMode = MM_NONE;
	m_pChatHistory = new CChatDialogHistory(this, "ChatHistory");

	CreateChatLines();
	CreateChatInputLine();
}

void CChatDialog::CreateChatInputLine(void)
{
	m_pChatInput = new CChatDialogInputLine(this, "ChatInputLine");
	m_pChatInput->SetVisible(false);

	if (GetChatHistory())
	{
		GetChatHistory()->SetMaximumCharCount(127 * 100);
		GetChatHistory()->SetVisible(true);
	}
}

void CChatDialog::CreateChatLines(void)
{
	m_ChatLine = new CChatDialogLine(this, "ChatLine");
	m_ChatLine->SetVisible(false);
}

void CChatDialog::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundType(2);
	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	GetChatHistory()->SetVerticalScrollbar(false);

	m_iHistoryAlpha = GetChatHistory()->GetBgColor().a();
	m_iAlpha = CHAT_HISTORY_ALPHA;

	Color cColor = pScheme->GetColor("DullWhite", GetBgColor());

	if (m_iAlpha == 0)
		m_iAlpha = cColor.a();

	SetBgColor(Color(cColor.r(), cColor.g(), cColor.b(), m_iAlpha));
	LoadControlSettings("Resource/UI/ChatDialog.res", "GAME");
}

void CChatDialog::Reset(void)
{
	Clear();
}

void CChatDialog::Paint(void)
{
}

CChatDialogHistory *CChatDialog::GetChatHistory(void)
{
	return m_pChatHistory;
}

void CChatDialog::Init(void)
{
}

void CChatDialog::VidInit(void)
{
	Clear();
	SetVisible(true);
	MakeReadyForUse();
	SetPaintBackgroundEnabled(false);

	if (GetChatHistory())
	{
		GetChatHistory()->SetPaintBorderEnabled(false);
		GetChatHistory()->SetBgColor(Color(GetChatHistory()->GetBgColor().r(), GetChatHistory()->GetBgColor().g(), GetChatHistory()->GetBgColor().b(), 0));
		GetChatHistory()->SetMouseInputEnabled(false);
		GetChatHistory()->SetVerticalScrollbar(false);
	}

	m_pChatInput->SetVisible(false);
}

void CChatDialog::Update(void)
{
}

void CChatDialog::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

int CChatDialog::GetChatInputOffset(void)
{
	return m_iFontHeight;
}

void CChatDialog::OnThink(void)
{
	if (m_ChatLine)
	{
		vgui2::HFont font = m_ChatLine->GetFont();
		m_iFontHeight = vgui2::surface()->GetFontTall(font) * 1.35;

		int iChatX, iChatY, iChatW, iChatH;
		int iInputX, iInputY, iInputW, iInputH;

		GetBounds(iChatX, iChatY, iChatW, iChatH);

		m_pChatInput->GetBounds(iInputX, iInputY, iInputW, iInputH);
		m_pChatInput->SetBounds(iInputX, iChatH - (m_iFontHeight * 1.75), iInputW, m_iFontHeight);

		int iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH;

		GetChatHistory()->GetBounds(iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH);

		iChatHistoryH = (iChatH - (m_iFontHeight * 2.25)) - iChatHistoryY;

		GetChatHistory()->SetBounds(iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH);
	}

	SetAlpha(255);

	if (CHAT_HISTORY_FADE_TIME > 0.0)
		FadeChatHistory();
}

#pragma optimize("", off)

int CChatDialog::ComputeBreakChar(int width, const char *text, int textlen)
{
	CChatDialogLine *line = m_ChatLine;
	vgui2::HFont font = line->GetFont();

	int currentlen = 0;
	int lastbreak = textlen;

	for (int i = 0; i < textlen; i++)
	{
		char ch = text[i];

		if (ch <= 32)
			lastbreak = i;

		wchar_t wch[2];
		vgui2::localize()->ConvertANSIToUnicode(&ch, wch, sizeof(wch));

		int a, b, c;
		vgui2::surface()->GetCharABCwide(font, wch[0], a, b, c);
		currentlen += a + b + c;

		if (currentlen >= width)
		{
			if (lastbreak == textlen)
				lastbreak = max(0, i - 1);

			break;
		}
	}

	if (currentlen >= width)
		return lastbreak;

	return textlen;
}

#pragma warning(push)
#pragma warning(disable: 4748)

void CChatDialog::Printf(const char *fmt, ...)
{
	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, marker);
	va_end(marker);

	ChatPrintf(0, "%s", msg);
}

#pragma warning(pop)
#pragma optimize("", on)

void CChatDialog::StartMessageMode(int iMessageModeType)
{
	if (!IsVisible())
		SetVisible(true);

	m_nMessageMode = iMessageModeType;
	m_pChatInput->ClearEntry();
	m_pChatInput->SetVisible(true);

	if (m_nMessageMode == MM_SAY)
		m_pChatInput->SetPrompt("Say");
	else
		m_pChatInput->SetPrompt("Team");

	if (GetChatHistory())
	{
		GetChatHistory()->SetMouseInputEnabled(true);
		GetChatHistory()->SetKeyBoardInputEnabled(false);
		GetChatHistory()->SetVerticalScrollbar(true);
		GetChatHistory()->ResetAllFades(true);
		GetChatHistory()->SetPaintBorderEnabled(true);
		GetChatHistory()->SetVisible(true);
		GetChatHistory()->SetCursor(vgui2::dc_arrow);
	}

	vgui2::SETUP_PANEL(this);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	vgui2::input()->ClearCompositionString();
	vgui2::surface()->CalculateMouseVisible();

	m_pChatInput->RequestFocus();
	m_pChatInput->SetPaintBorderEnabled(true);
	m_pChatInput->SetMouseInputEnabled(true);

	if (CHAT_HISTORY_FADE_TIME <= 0)
	{
		SetPaintBackgroundEnabled(true);

		if (GetChatHistory())
			GetChatHistory()->SetBgColor(Color(GetChatHistory()->GetBgColor().r(), GetChatHistory()->GetBgColor().g(), GetChatHistory()->GetBgColor().b(), m_iHistoryAlpha));
	}
	else
	{
		m_flHistoryFadeTime = gEngfuncs.GetAbsoluteTime() + CHAT_HISTORY_FADE_TIME;
	}
}

void CChatDialog::StopMessageMode(void)
{
	m_nMessageMode = MM_NONE;

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	if (GetChatHistory())
	{
		GetChatHistory()->SetPaintBorderEnabled(false);
		GetChatHistory()->GotoTextEnd();
		GetChatHistory()->SetMouseInputEnabled(false);
		GetChatHistory()->SetVerticalScrollbar(false);
		GetChatHistory()->ResetAllFades(false, true, CHAT_HISTORY_FADE_TIME);
		GetChatHistory()->SelectNoText();
		GetChatHistory()->SetCursor(vgui2::dc_none);
	}

	m_pChatInput->ClearEntry();
	m_pChatInput->SetVisible(false);

	if (CHAT_HISTORY_FADE_TIME <= 0)
	{
		SetPaintBackgroundEnabled(false);

		if (GetChatHistory())
			GetChatHistory()->SetBgColor(Color(GetChatHistory()->GetBgColor().r(), GetChatHistory()->GetBgColor().g(), GetChatHistory()->GetBgColor().b(), 0));
	}
	else
	{
		m_flHistoryFadeTime = gEngfuncs.GetAbsoluteTime() + CHAT_HISTORY_FADE_TIME;
	}
}

#pragma warning(push)
#pragma warning(disable: 4723)

void CChatDialog::FadeChatHistory(void)
{
	float frac = (m_flHistoryFadeTime - g_pViewPort->GetCurrentTime()) / CHAT_HISTORY_FADE_TIME;

	int alpha = frac * m_iAlpha;
	alpha = clamp(alpha, 0, m_iAlpha);

	if (alpha >= 0)
	{
		if (GetChatHistory())
		{
			if (IsMouseInputEnabled())
			{
				SetPaintBackgroundEnabled(true);
				GetChatHistory()->SetBgColor(Color(GetChatHistory()->GetBgColor().r(), GetChatHistory()->GetBgColor().g(), GetChatHistory()->GetBgColor().b(), m_iAlpha - alpha));

				m_pChatInput->GetPrompt()->SetBgColor(Color(m_pChatInput->GetPrompt()->GetBgColor().r(), m_pChatInput->GetPrompt()->GetBgColor().g(), m_pChatInput->GetPrompt()->GetBgColor().b(), m_iAlpha - alpha));
				m_pChatInput->GetInputPanel()->SetBgColor(Color(m_pChatInput->GetInputPanel()->GetBgColor().r(), m_pChatInput->GetInputPanel()->GetBgColor().g(), m_pChatInput->GetInputPanel()->GetBgColor().b(), m_iAlpha - alpha));
				m_pChatInput->GetPrompt()->SetAlpha((m_iAlpha * 2) - alpha);
				m_pChatInput->GetInputPanel()->SetAlpha((m_iAlpha * 2) - alpha);

				SetBgColor(Color(GetBgColor().r(), GetBgColor().g(), GetBgColor().b(), m_iAlpha - alpha));
			}
			else
			{
				SetPaintBackgroundEnabled(false);
				GetChatHistory()->SetBgColor(Color(GetChatHistory()->GetBgColor().r(), GetChatHistory()->GetBgColor().g(), GetChatHistory()->GetBgColor().b(), alpha));
				SetBgColor(Color(GetBgColor().r(), GetBgColor().g(), GetBgColor().b(), alpha));

				m_pChatInput->GetPrompt()->SetBgColor(Color(m_pChatInput->GetPrompt()->GetBgColor().r(), m_pChatInput->GetPrompt()->GetBgColor().g(), m_pChatInput->GetPrompt()->GetBgColor().b(), alpha));
				m_pChatInput->GetInputPanel()->SetBgColor(Color(m_pChatInput->GetInputPanel()->GetBgColor().r(), m_pChatInput->GetInputPanel()->GetBgColor().g(), m_pChatInput->GetInputPanel()->GetBgColor().b(), alpha));
				m_pChatInput->GetPrompt()->SetAlpha(alpha);
				m_pChatInput->GetInputPanel()->SetAlpha(alpha);
			}
		}
	}
}

#pragma warning(pop)

Color CChatDialog::GetTextColorForClient(TextColor colorNum, int clientIndex)
{
	Color c;

	switch (colorNum)
	{
		case COLOR_PLAYERNAME:
		{
			c = GetClientColor(clientIndex);
			break;
		}

		case COLOR_LOCATION:
		{
			c = g_ColorDarkGreen;
			break;
		}

		default:
		{
			c = g_ColorYellow;
		}
	}

	return Color(c[0], c[1], c[2], 255);
}

Color CChatDialog::GetClientColor(int clientIndex)
{
	if (clientIndex == 0)
		return g_ColorGreen;

	return g_ColorYellow;
}

void CChatDialogLine::InsertAndColorizeText(wchar_t *buf, int clientIndex)
{
	if (m_text)
	{
		delete [] m_text;
		m_text = NULL;
	}

	m_textRanges.RemoveAll();
	m_text = CloneWString(buf);

	CChatDialog *pChat = dynamic_cast<CChatDialog *>(GetParent());

	if (pChat == NULL)
		return;

	wchar_t *txt = m_text;
	int lineLen = wcslen(m_text);

	if (m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION || m_text[0] == COLOR_NORMAL)
	{
		while (txt && *txt)
		{
			TextRange range;

			switch (*txt)
			{
				case COLOR_PLAYERNAME:
				case COLOR_LOCATION:
				case COLOR_NORMAL:
				{
					range.start = (txt - m_text) + 1;
					range.color = pChat->GetTextColorForClient((TextColor)(*txt), clientIndex);
					range.end = lineLen;

					int count = m_textRanges.Count();

					if (count)
						m_textRanges[count - 1].end = range.start - 1;

					m_textRanges.AddToTail(range);

					++txt;
					break;
				}

				default: ++txt;
			}
		}
	}

	if (!m_textRanges.Count() && m_iNameLength > 0 && m_text[0] == COLOR_USEOLDCOLORS)
	{
		TextRange range;
		range.start = 0;
		range.end = m_iNameStart;
		range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
		m_textRanges.AddToTail(range);

		range.start = m_iNameStart;
		range.end = m_iNameStart + m_iNameLength;
		range.color = pChat->GetTextColorForClient(COLOR_PLAYERNAME, clientIndex);
		m_textRanges.AddToTail(range);

		range.start = range.end;
		range.end = wcslen(m_text);
		range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
		m_textRanges.AddToTail(range);
	}

	if (!m_textRanges.Count())
	{
		TextRange range;
		range.start = 0;
		range.end = wcslen(m_text);
		range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
		m_textRanges.AddToTail(range);
	}

	for (int i = 0; i < m_textRanges.Count(); ++i)
	{
		wchar_t * start = m_text + m_textRanges[i].start;

		if (*start > 0 && *start < COLOR_MAX)
			m_textRanges[i].start += 1;
	}

	Colorize();
}

void CChatDialogLine::Colorize(int alpha)
{
	CChatDialog *pChat = dynamic_cast<CChatDialog *>(GetParent());

	if (pChat && pChat->GetChatHistory())
		pChat->GetChatHistory()->InsertString("\n");

	wchar_t wText[4096];
	Color color;

	for (int i = 0; i < m_textRanges.Count(); ++i)
	{
		wchar_t * start = m_text + m_textRanges[i].start;
		int len = m_textRanges[i].end - m_textRanges[i].start + 1;

		if (len > 1)
		{
			wcsncpy(wText, start, len);
			wText[len - 1] = 0;
			color = m_textRanges[i].color;
			color[3] = alpha;
			InsertColorChange(color);
			InsertString(wText);

			CChatDialog *pChat = dynamic_cast<CChatDialog *>(GetParent());

			if (pChat && pChat->GetChatHistory())
			{
				pChat->GetChatHistory()->InsertColorChange(color);
				pChat->GetChatHistory()->InsertString(wText);
				pChat->GetChatHistory()->InsertFade(hud_saytext_time->value, CHAT_HISTORY_IDLE_FADE_TIME);

				if (i == m_textRanges.Count() - 1)
					pChat->GetChatHistory()->InsertFade(-1, -1);
			}
		}
	}

	InvalidateLayout(true);
}

CChatDialogLine *CChatDialog::FindUnusedChatLine(void)
{
	return m_ChatLine;
}

void CChatDialog::Send(void)
{
	wchar_t szTextbuf[128];
	m_pChatInput->GetMessageText(szTextbuf, sizeof(szTextbuf));

	char ansi[128];
	vgui2::localize()->ConvertUnicodeToANSI(szTextbuf, ansi, sizeof(ansi));

	int len = Q_strlen(ansi);

	if (len > 0 && ansi[len - 1] == '\n')
		ansi[len - 1] = '\0';

	if (len > 0)
	{
		char szbuf[144];

		switch (m_nMessageMode)
		{
			case MM_SAY:
			{
				Q_snprintf(szbuf, sizeof(szbuf), "say \"%s\"", ansi);
				engine->pfnClientCmd(szbuf);
				break;
			}

			case MM_SAY_TEAM:
			{
				Q_snprintf(szbuf, sizeof(szbuf), "say_team \"%s\"", ansi);
				engine->pfnClientCmd(szbuf);
				break;
			}
		}
	}

	m_pChatInput->ClearEntry();
}

vgui2::Panel *CChatDialog::GetInputPanel(void)
{
	return m_pChatInput->GetInputPanel();
}

void CChatDialog::Clear(void)
{
	m_pChatInput->ClearEntry();
	m_pChatHistory->SetText("");
}

void CChatDialog::ChatPrintf(int iPlayerIndex, const char *fmt, ...)
{
	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, marker);
	va_end(marker);

	if (strlen(msg) > 0 && msg[strlen(msg) - 1] == '\n')
		msg[strlen(msg) - 1] = 0;

	char *pmsg = msg;

	while (*pmsg && (*pmsg == '\n' || (*pmsg > 0 && *pmsg < COLOR_MAX)))
		pmsg++;

	if (!*pmsg)
		return;

	pmsg = msg;

	while (*pmsg && (*pmsg == '\n'))
		pmsg++;

	if (!*pmsg)
		return;

	CChatDialogLine *line = (CChatDialogLine *)FindUnusedChatLine();

	if (!line)
		line = (CChatDialogLine *)FindUnusedChatLine();

	if (!line)
		return;

	line->SetText("");

	int iNameStart = 0;
	int iNameLength = 0;

	hud_player_info_t sPlayerInfo;

	if (iPlayerIndex == 0)
	{
		Q_memset(&sPlayerInfo, 0, sizeof(hud_player_info_t));
		sPlayerInfo.name = "Console";
	}
	else
		engine->pfnGetPlayerInfo(iPlayerIndex, &sPlayerInfo);

	int bufSize = (strlen(pmsg) + 1) * sizeof(wchar_t);
	wchar_t *wbuf = static_cast<wchar_t *>(_alloca(bufSize));

	if (wbuf)
	{
		line->SetExpireTime();
		vgui2::localize()->ConvertANSIToUnicode(pmsg, wbuf, bufSize);

		if (sPlayerInfo.name)
		{
			wchar_t wideName[MAX_PLAYER_NAME_LENGTH];
			vgui2::localize()->ConvertANSIToUnicode(sPlayerInfo.name, wideName, sizeof(wideName));

			const wchar_t *nameInString = wcsstr(wbuf, wideName);

			if (nameInString)
			{
				iNameStart = (nameInString - wbuf);
				iNameLength = wcslen(wideName);
			}
		}

		Color clrNameColor = GetClientColor(iPlayerIndex);

		line->SetVisible(false);
		line->SetNameStart(iNameStart);
		line->SetNameLength(iNameLength);
		line->SetNameColor(clrNameColor);
		line->InsertAndColorizeText(wbuf, iPlayerIndex);
	}

	if (g_pViewPort->AllowedToPrintText())
		SetVisible(true);
}

void CChatDialog::ChatPrintf(int iPlayerIndex, const wchar_t *string)
{
	wchar_t msg[4096];
	wcscpy(msg, string);

	if (wcslen(msg) > 0 && msg[wcslen(msg) - 1] == '\n')
		msg[wcslen(msg) - 1] = 0;

	wchar_t *pmsg = msg;

	while (*pmsg && (*pmsg == L'\n' || (*pmsg > 0 && *pmsg < COLOR_MAX)))
		pmsg++;

	if (!*pmsg)
		return;

	pmsg = msg;

	while (*pmsg && (*pmsg == L'\n'))
		pmsg++;

	if (!*pmsg)
		return;

	CChatDialogLine *line = (CChatDialogLine *)FindUnusedChatLine();

	if (!line)
		line = (CChatDialogLine *)FindUnusedChatLine();

	if (!line)
		return;

	line->SetText("");

	int iNameStart = 0;
	int iNameLength = 0;

	hud_player_info_t sPlayerInfo;

	if (iPlayerIndex == 0)
	{
		Q_memset(&sPlayerInfo, 0, sizeof(hud_player_info_t));
		sPlayerInfo.name = "Console";
	}
	else
		engine->pfnGetPlayerInfo(iPlayerIndex, &sPlayerInfo);

	line->SetExpireTime();

	if (sPlayerInfo.name)
	{
		wchar_t wideName[MAX_PLAYER_NAME_LENGTH];
		vgui2::localize()->ConvertANSIToUnicode(sPlayerInfo.name, wideName, sizeof(wideName));

		const wchar_t *nameInString = wcsstr(msg, wideName);

		if (nameInString)
		{
			iNameStart = (nameInString - msg);
			iNameLength = wcslen(wideName);
		}
	}

	Color clrNameColor = GetClientColor(iPlayerIndex);

	line->SetVisible(false);
	line->SetNameStart(iNameStart);
	line->SetNameLength(iNameLength);
	line->SetNameColor(clrNameColor);
	line->InsertAndColorizeText(msg, iPlayerIndex);

	if (g_pViewPort->AllowedToPrintText())
		SetVisible(true);
}

void CChatDialog::MsgFunc_SayText(void)
{
}

void CChatDialog::MsgFunc_TextMsg(void)
{
}