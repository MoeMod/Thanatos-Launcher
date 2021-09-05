#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"

#include <vgui/VGUI2.h>
#include <vgui/ISurface.h>
#include <vgui/ilocalize.h>
#include <KeyValues.h>

void GetHudSize(int &w, int &h)
{
	vgui2::VPANEL hudParent = enginevgui->GetPanel(PANEL_CLIENTDLL);

	if (hudParent)
		vgui2::ipanel()->GetSize(hudParent, w, h);
	else
		vgui2::surface()->GetScreenSize(w, h);
}