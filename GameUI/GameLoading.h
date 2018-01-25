#pragma once

#include <vgui_controls/PHandle.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Progressbar.h>

#include <set>

using namespace vgui;

class CGameLoading : public Panel
{
	DECLARE_CLASS_SIMPLE(CGameLoading, Panel);

public:
	CGameLoading(Panel *parent, const char *name);
	~CGameLoading();
	virtual void PerformLayout(void);
	
	void SetStatusText(const char *szStatusText);
	bool SetProgressPoint(int progressPoint);
	void SetProgressRange(int min, int max);
	void SetProgressVisible(bool bSet);
	void Activate();

	virtual void PaintBackground(); // 重画高清♂无码的载入tga

private:

	Label *m_pStatusText;
	ProgressBar *m_pProgress;
	int m_iRangeMin, m_iRangeMax;
};