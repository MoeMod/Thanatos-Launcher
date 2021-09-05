#ifndef VGUI_HUD_ELEMENT_H
#define VGUI_HUD_ELEMENT_H

class CHudElement
{
public:
	virtual	~CHudElement(void) {}

public:
	virtual void Init(void) = 0;
	virtual void VidInit(void) = 0;
	virtual void Reset(void) = 0;
	virtual void Think(void) = 0;

public:
	virtual vgui2::VPANEL GetVPanel(void) = 0;
	virtual bool IsVisible(void) = 0;
	virtual void SetParent(vgui2::VPANEL parent) = 0;
	virtual void SetParent(vgui2::Panel *newParent) = 0;
	virtual bool IsMouseInputEnabled(void) = 0;
	virtual void SetMouseInputEnabled(bool state) = 0;
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) = 0;
};

#define DECLARE_HUD_ELEMENT_SIMPLE() \
	vgui2::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); } \
	virtual bool IsVisible(void) { return BaseClass::IsVisible(); } \
	virtual void SetParent(vgui2::VPANEL parent) { BaseClass::SetParent(parent); } \
	virtual void SetParent(vgui2::Panel *newParent) { BaseClass::SetParent(newParent); } \
	virtual bool IsMouseInputEnabled(void) { return BaseClass::IsMouseInputEnabled(); } \
	virtual void SetMouseInputEnabled(bool state) { BaseClass::SetMouseInputEnabled(state); } \
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) { BaseClass::InvalidateLayout(layoutNow, reloadScheme); }

#endif