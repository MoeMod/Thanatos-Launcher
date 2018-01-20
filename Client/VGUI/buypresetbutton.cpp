#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "buypresetbutton.h"
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>

using namespace vgui;

class BuyPresetImagePanel : public EditablePanel
{
	typedef EditablePanel BaseClass;

public:
	BuyPresetImagePanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);

		m_pPrimaryWeapon = new ImagePanel(this, "primary");
		m_pSecondaryWeapon = new ImagePanel(this, "secondary");
		m_pKnifeWeapon = new ImagePanel(this, "knife");
		m_pPrimaryWeapon->SetShouldScaleImage(true);
	}

	void ClearWeapons(void)
	{
		m_pPrimaryWeapon->SetImage((IImage *)NULL);
		m_pSecondaryWeapon->SetImage((IImage *)NULL);
		m_pKnifeWeapon->SetImage((IImage *)NULL);
	}

	void SetPrimaryWeapon(IImage *image)
	{
		m_pPrimaryWeapon->SetImage(image);
	}

	void SetSecondaryWeapon(IImage *image)
	{
		m_pSecondaryWeapon->SetImage(image);
	}

	void SetKnifeWeapon(IImage *image)
	{
		m_pKnifeWeapon->SetImage(image);
	}

private:
	ImagePanel *m_pPrimaryWeapon;
	ImagePanel *m_pSecondaryWeapon;
	ImagePanel *m_pKnifeWeapon;
};

BuyPresetButton::BuyPresetButton(Panel *parent, const char *panelName) : BaseClass(parent, panelName, "BuyPresetButton")
{
	m_pImagePanel = new BuyPresetImagePanel(this, "ImagePanel");
	m_pImagePanel->LoadControlSettings("Resource/UI/BuyPresetButton.res", "GAME");

	m_pKeyboard = NULL;
	m_pBlankSlot = NULL;
	m_pSelect = NULL;

	m_pFullText = NULL;
	m_pText = NULL;
	m_cHotkey = 0;
	m_iKeyOffset = -1;
	m_iKeySize = -1;

	m_pSetCommand = NULL;
	m_iMousePressed = (MouseCode)-1;
}

BuyPresetButton::~BuyPresetButton(void)
{
	delete m_pImagePanel;

	if (m_pFullText)
		delete [] m_pFullText;

	if (m_pSetCommand)
		delete [] m_pSetCommand;
}

void BuyPresetButton::ApplySettings(KeyValues *resourceData)
{
	BaseClass::ApplySettings(resourceData);

	m_pKeyboard = scheme()->GetImage(resourceData->GetString("image_keyboard", ""), true);
	m_pBlankSlot = scheme()->GetImage(resourceData->GetString("image_blankslot", ""), true);
	m_pSelect = scheme()->GetImage(resourceData->GetString("image_select", ""), true);

	m_pSetCommand = CloneString(resourceData->GetString("command_set", ""));

	if (m_pSetCommand[0])
		SetMouseClickEnabled(MOUSE_RIGHT, true);
}

void BuyPresetButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (!m_pKeyboard)
		m_pKeyboard = scheme()->GetImage(pScheme->GetResourceString("BuyPresetButton.Keyboard"), true);

	if (!m_pBlankSlot)
		m_pBlankSlot = scheme()->GetImage(pScheme->GetResourceString("BuyPresetButton.BlankSlot"), true);

	if (!m_pSelect)
		m_pSelect = scheme()->GetImage(pScheme->GetResourceString("BuyPresetButton.Select"), true);

	if (m_iKeyOffset == -1)
	{
		m_iKeyOffset = atoi(pScheme->GetResourceString("BuyPresetButton.KeyboardOffset"));

		if (IsProportional())
			m_iKeyOffset = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeyOffset);
	}

	if (m_iKeySize == -1)
	{
		m_iKeySize = atoi(pScheme->GetResourceString("BuyPresetButton.KeyboardSize"));

		if (IsProportional())
			m_iKeySize = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeySize);
	}

	if (m_pFullText)
	{
		if (m_pKeyboard)
		{
			SetText(m_pText);
			SetHotkey(m_cHotkey);
		}
	}
}

void BuyPresetButton::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void BuyPresetButton::DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, bool hollow)
{
	if (!ShouldPaint())
		return;

	if (m_pBlankSlot && m_pSelect)
	{
		if (IsArmed())
		{
			m_pSelect->SetSize(wide, tall);
			m_pSelect->Paint();
		}
		else
		{
			m_pBlankSlot->SetSize(wide, tall);
			m_pBlankSlot->Paint();
		}
	}

	if (m_pKeyboard)
	{
		wchar_t key = m_cHotkey;

		if (key)
		{
			if (m_iKeyOffset > 0 && m_iKeySize > 0)
			{
				int keyWide = m_iKeySize;
				int keyTall = m_iKeySize;

				int keyX = m_iKeyOffset;
				int keyY = (tall - keyTall) / 2;

				m_pKeyboard->SetPos(keyX, keyY);
				m_pKeyboard->SetSize(keyWide, keyTall);
				m_pKeyboard->Paint();

				SetTextInset(keyX + keyWide + keyX, 0);
			}
		}
	}
}

void BuyPresetButton::Paint(void)
{
	BaseClass::Paint();

	if (m_pKeyboard)
	{
		int wide, tall;
		GetSize(wide, tall);

		wchar_t key = m_cHotkey;

		if (key)
		{
			if (m_iKeyOffset > 0 && m_iKeySize > 0)
			{
				int keyWide = m_iKeySize;
				int keyTall = m_iKeySize;

				int keyX = m_iKeyOffset;
				int keyY = (tall - keyTall) / 2;

				HFont font = GetFont();

				int charWide, charTall;
				charTall = surface()->GetFontTall(font);

				int a, b, c;
				surface()->GetCharABCwide(font, key, a, b, c);
				charWide = a + b + c;

				surface()->DrawSetTextColor(GetFgColor());
				surface()->DrawSetTextFont(font);
				surface()->DrawSetTextPos(keyX + (keyWide - charWide) / 2, keyY + (keyTall - charTall) / 2);
				surface()->DrawUnicodeChar(key);
				surface()->DrawFlushText();
			}
		}
	}
}

void BuyPresetButton::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void BuyPresetButton::FireActionSignal(void)
{
	if (m_iMousePressed == MOUSE_RIGHT)
	{
		PostActionSignal(new KeyValues("Command", "command", m_pSetCommand));
		return;
	}

	BaseClass::FireActionSignal();
}

void BuyPresetButton::OnMousePressed(MouseCode code)
{
	BaseClass::OnMousePressed(code);

	m_iMousePressed = code;
}

void BuyPresetButton::OnMouseReleased(MouseCode code)
{
	BaseClass::OnMouseReleased(code);

	m_iMousePressed = (MouseCode)-1;
}

void BuyPresetButton::GetText(char *textOut, int bufferLen)
{
	g_pVGuiLocalize->ConvertUnicodeToANSI(m_pFullText, textOut, bufferLen);
}

void BuyPresetButton::GetText(wchar_t *textOut, int bufLenInBytes)
{
	wcsncpy(textOut, m_pFullText, bufLenInBytes / sizeof(wchar_t));
}

void BuyPresetButton::SetHotkey(wchar_t ch)
{
	BaseClass::SetHotkey(towlower(ch));
}

void BuyPresetButton::SetText(const wchar_t *wtext)
{
	BaseClass::SetText(wtext);
}

void BuyPresetButton::SetText(const char *text)
{
	const wchar_t *wtext;

	if (text[0] == '#')
	{
		wtext = g_pVGuiLocalize->Find(text);
	}
	else
	{
		wchar_t unicodeVar[256];
		g_pVGuiLocalize->ConvertANSIToUnicode(text, unicodeVar, sizeof(unicodeVar));
		wtext = unicodeVar;
	}

	if (!wtext)
	{
		BaseClass::SetText("");
		return;
	}

	wchar_t hotkey = 0;
	const wchar_t *saveptr = wtext;

	for (const wchar_t *ch = wtext; *ch != 0; ch++)
	{
		if (*ch == '&')
		{
			ch++;

			if (*ch == '&')
			{
				continue;
			}
			else if (*ch == 0)
			{
				break;
			}
			else if (iswalnum(*ch))
			{
				hotkey = *ch;

				if (ch <= (wtext + 1))
				{
					ch++;

					while (*ch == ' ')
						ch++;

					wtext = ch;
					break;
				}
			}
		}
	}

	if (m_pFullText)
		delete [] m_pFullText;

	m_pFullText = CloneWString(saveptr);
	m_pText = m_pFullText + (wtext - saveptr);
	m_cHotkey = hotkey;

	if (m_pKeyboard)
	{
		SetText(wtext);
		SetHotkey(hotkey);
	}
	else
	{
		BaseClass::SetText(text);
	}
}

void BuyPresetButton::ClearWeapons(void)
{
	m_pImagePanel->ClearWeapons();
}

void BuyPresetButton::SetPrimaryWeapon(IImage *image)
{
	m_pImagePanel->SetPrimaryWeapon(image);
}

void BuyPresetButton::SetSecondaryWeapon(IImage *image)
{
	m_pImagePanel->SetSecondaryWeapon(image);
}

void BuyPresetButton::SetKnifeWeapon(IImage *image)
{
	m_pImagePanel->SetKnifeWeapon(image);
}