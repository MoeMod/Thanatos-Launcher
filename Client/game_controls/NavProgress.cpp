#include "hud.h"
#include "NavProgress.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>

#include <vgui_controls/Label.h>

using namespace vgui2;

CNavProgress::CNavProgress(void) : Frame(NULL, PANEL_NAV_PROGRESS)
{
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetMinimumSize(1, 1);

	SetTitleBarVisible(false);

	m_pTitle = new Label(this, "TitleLabel", "");
	m_pText = new Label(this, "TextLabel", "");

	m_pProgressBarBorder = new Panel(this, "ProgressBarBorder");
	m_pProgressBar = new Panel(this, "ProgressBar");
	m_pProgressBarSizer = new Panel(this, "ProgressBarSizer");
	m_pProgressBar->SetMinimumSize(13, 1);

	LoadControlSettings("Resource/UI/NavProgress.res", "GAME");
	Reset();
}

CNavProgress::~CNavProgress(void)
{
}

void CNavProgress::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pProgressBarSizer->SetVisible(false);

	m_pProgressBarBorder->SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	m_pProgressBarBorder->SetBgColor(Color(0, 0, 0, 0));

	m_pProgressBar->SetBorder(pScheme->GetBorder("ButtonBorder"));
	m_pProgressBar->SetBgColor(pScheme->GetColor("ProgressBar.FgColor", Color(0, 0, 0, 0)));

	SetPaintBackgroundType(2);
}

void CNavProgress::PerformLayout(void)
{
	BaseClass::PerformLayout();

	if (m_numTicks)
	{
		int w = m_pProgressBarSizer->GetWide();
		w = w * m_currentTick / m_numTicks;
		m_pProgressBar->SetWide(w);
	}
}

void CNavProgress::Init(const char *title, int numTicks, int startTick)
{
	m_pText->SetText(title);

	m_numTicks = max(1, numTicks);
	m_currentTick = max(0, min(m_numTicks, startTick));

	InvalidateLayout();
}

void CNavProgress::Update(const char *statusText, int tick)
{
	m_pText->SetText(statusText);
	m_currentTick = max(0, min(m_numTicks, tick));

	InvalidateLayout();
}

void CNavProgress::SetData(KeyValues *data)
{
	Init(data->GetString("msg"), data->GetInt("total"), data->GetInt("current"));
}

void CNavProgress::ShowPanel(bool bShow)
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

void CNavProgress::Init(void)
{
}

void CNavProgress::VidInit(void)
{
	SetVisible(false);
}

void CNavProgress::Reset(void)
{
}

void CNavProgress::Update(void)
{
}