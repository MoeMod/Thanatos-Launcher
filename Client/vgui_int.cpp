#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"

#include <vgui/vgui.h>
#include <vgui/ISurface.h>
#include <vgui/ilocalize.h>
#include <KeyValues.h>

void GetHudSize(int &w, int &h)
{
	vgui::VPANEL hudParent = enginevgui->GetPanel(vgui::PANEL_CLIENTDLL);

	if (hudParent)
		vgui::ipanel()->GetSize(hudParent, w, h);
	else
		vgui::surface()->GetScreenSize(w, h);
}