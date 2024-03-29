//#define ENABLE_HTMLWINDOW

#include "hud.h"
#include "game_controls\vguitextwindow.h"

#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/TextEntry.h>

#include "plugins.h"

using namespace vgui2;

#define TEMP_HTML_FILE "textwindow_temp.html"

CTextWindow::CTextWindow(void) : Frame(NULL, PANEL_INFO)
{
	m_szTitle[0] = '\0';
	m_szMessage[0] = '\0';
	m_szExitCommand[0] = '\0';

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pTextMessage = new TextEntry(this, "TextMessage");
#if defined (ENABLE_HTMLWINDOW)
	m_pHTML = new CHTML(this, "HTMLMessage");
#endif
	m_pTitleLabel = new Label(this, "MessageTitle", "Message Title");
	m_pOK = new Button(this, "ok", "#PropertyDialog_OK");

	m_pOK->SetCommand("okay");
	m_pTextMessage->SetMultiline(true);
	m_nContentType = TYPE_TEXT;
}

void CTextWindow::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

CTextWindow::~CTextWindow(void)
{
	filesystem()->RemoveFile(TEMP_HTML_FILE, "DEFAULT_WRITE_PATH");
}

void CTextWindow::Reset(void)
{
}

void CTextWindow::VidInit(void)
{
	Q_strcpy(m_szTitle, "This could be your Title.");
	Q_strcpy(m_szMessage, "Just for 10 Euros a week!");

	m_szExitCommand[0] = 0;
	m_nContentType = TYPE_TEXT;

	Update();
}

void CTextWindow::ShowText(const char *text)
{
	m_pTextMessage->SetVisible(true);
	m_pTextMessage->SetText(text);
	m_pTextMessage->GotoTextStart();
}

void CTextWindow::ShowURL(const char *URL)
{
#if defined (ENABLE_HTMLWINDOW)
	
	m_pHTML->SetVisible(true);
	m_pHTML->OpenURL(URL);
	
#endif
}

void CTextWindow::ShowIndex(const char *entry)
{
	const char *data = NULL;
	int length = 0;
#if 0
	if (NULL == g_pStringTableInfoPanel)
		return;

	int index = g_pStringTableInfoPanel->FindStringIndex(m_szMessage);

	if (index != ::INVALID_STRING_INDEX)
		data = (const char *)g_pStringTableInfoPanel->GetStringUserData(index, &length);
#endif
	if (!data)
	{
		data = entry;
		length = strlen(data);
	}

	if (!data || !data[0])
		return;

	if (!Q_strncmp(data, "http://", 7))
	{
		ShowURL(data);
		return;
	}

	if (data[0] != '<')
	{
		ShowText(data);
		return;
	}

	FileHandle_t hFile = filesystem()->Open(TEMP_HTML_FILE, "wb", "DEFAULT_WRITE_PATH");

	if (hFile == FILESYSTEM_INVALID_HANDLE)
		return;

	filesystem()->Write(data, length, hFile);
	filesystem()->Close(hFile);

	if (filesystem()->Size(TEMP_HTML_FILE) == (unsigned int)length)
		ShowFile(TEMP_HTML_FILE);
}

void CTextWindow::ShowFile(const char *filename)
{
	if (Q_stristr(filename, ".htm") || Q_stristr(filename, ".html"))
	{
		char localURL[_MAX_PATH + 7];
		Q_strncpy(localURL, "file://", sizeof(localURL));

		char pPathData[_MAX_PATH];
		filesystem()->GetLocalPath(filename, pPathData, sizeof(pPathData));
		Q_strncat(localURL, pPathData, sizeof(localURL), COPY_ALL_CHARACTERS);

		ShowURL(localURL);
	}
	else
	{
		FileHandle_t f = filesystem()->Open(m_szMessage, "rb", "GAME");

		if (!f)
			return;

		char buffer[2048];
		int size = min(filesystem()->Size(f), sizeof(buffer) - 1);
		filesystem()->Read(buffer, size, f);
		filesystem()->Close(f);
		buffer[size] = 0;

		ShowText(buffer);
	}
}

void CTextWindow::Update(void)
{
	SetTitle(m_szTitle, false);

	m_pTitleLabel->SetText(m_szTitle);
	m_pTitleLabel->SetVisible(true);

#if defined (ENABLE_HTMLWINDOW)
	m_pHTML->SetVisible(false);
#endif
	m_pTextMessage->SetVisible(false);

	if (m_nContentType == TYPE_INDEX)
	{
		ShowIndex(m_szMessage);
	}
	else if (m_nContentType == TYPE_URL)
	{
		ShowURL(m_szMessage);
	}
	else if (m_nContentType == TYPE_FILE)
	{
		ShowFile(m_szMessage);
	}
	else if (m_nContentType == TYPE_TEXT)
	{
		ShowText(m_szMessage);
	}
	else
	{
		DevMsg("CTextWindow::Update: unknown content type %i\n", m_nContentType);
	}
}

void CTextWindow::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		if (m_szExitCommand[0])
			engine->pfnClientCmd(m_szExitCommand);

		m_bIsExited = true;
		g_pViewPort->ShowPanel(this, false);
		filesystem()->RemoveFile(TEMP_HTML_FILE, "DEFAULT_WRITE_PATH");
	}

	BaseClass::OnCommand(command);
}

void CTextWindow::SetData(KeyValues *data)
{
	SetData(data->GetInt("type"), data->GetString("title"), data->GetString("msg"), data->GetString("cmd"));
}

void CTextWindow::SetData(int type, const char *title, const char *message, const char *command)
{
	Q_strncpy(m_szTitle, title, sizeof(m_szTitle));
	Q_strncpy(m_szMessage, message, sizeof(m_szMessage));

	if (command)
	{
		Q_strncpy(m_szExitCommand, command, sizeof(m_szExitCommand));
	}
	else
	{
		m_szExitCommand[0] = 0;
	}

	m_bIsExited = false;
	m_nContentType = type;

	Update();
}

void CTextWindow::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	g_pViewPort->ShowBackGround(bShow);

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