#include <vgui_controls/Frame.h>

class CBackGroundPanel : public vgui2::Frame
{
	typedef vgui2::Frame BaseClass;

public:
	CBackGroundPanel(vgui2::Panel *parent) : BaseClass(parent, "ViewPortBackGround")
	{
		SetScheme("ClientScheme");

		SetTitleBarVisible(false);
		SetMoveable(false);
		SetSizeable(false);
		SetProportional(true);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}

public:
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetBgColor(pScheme->GetColor("ViewportBG", Color(0, 0, 0, 0)));
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();
	}

	virtual void OnMousePressed(vgui2::MouseCode code)
	{
	}

	virtual vgui2::VPANEL IsWithinTraverse(int x, int y, bool traversePopups)
	{
		return NULL;
	}
};