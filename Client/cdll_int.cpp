#include "hud.h"

CHud gHUD;

void HUD_DirectorMessage(int iSize, void *pbuf)
{
	if (!gHUD.m_Spectator.DirectorMessage(iSize, pbuf))
		return;

	gExportfuncs.HUD_DirectorMessage(iSize, pbuf);
}