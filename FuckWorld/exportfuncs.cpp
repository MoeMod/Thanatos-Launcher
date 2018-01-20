#include <metahook.h>
#include <engfuncs.h>
#include "event_api.h"
#include "IMEInput.h"
#include "CVARs.h"
#include "Window.h"
#include "msghook.h"
#include "Video.h"
#include "Gamma.h"
#include "qgl.h"
#include "common.h"
#include "client.h"
#include "Version.h"

extern "C"
{
#include "HLSDK/pm_shared/pm_defs.h"
#include "HLSDK/pm_shared/pm_shared.h"
}

#include "Client/hud.h"
#include "game_shared/voice_status.h"
#include "BugReport/CrashHandle.h"

#include "Client/tri.h"
#include "Client/entity.h"
#include "Client/inputw32.h"
#include "Client/in_camera.h"
#include "Client/view.h"
#include "Client/demo.h"
#include "Client/kbutton.h"

cl_enginefunc_t gEngfuncs;

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	Engfuncs_InstallHook(pEnginefuncs, iVersion);

	//INEIN_InstallHook();
	COM_Init();
	QGL_Init();
	MSG_Init();

	InitCrashHandle();
	
	int result = gExportfuncs.Initialize(pEnginefuncs, iVersion);

	if (result)
		IN_InstallHook();

	Version_Init();

	return result;
}

void InitInput(void);
void ShutdownInput(void);

void HUD_Init(void)
{
	gExportfuncs.HUD_Init();

	gHUD.Init();
	InitInput();
}

int HUD_VidInit(void)
{
	int result = gExportfuncs.HUD_VidInit();

	VID_VidInit();
	gHUD.VidInit();

	return result;
}

int HUD_Redraw(float time, int intermission)
{
	int result = gExportfuncs.HUD_Redraw(time, intermission);

	gHUD.Redraw(time, intermission);
	return result;
}

int HUD_UpdateClientData(client_data_t *pcldata, float flTime)
{
	gHUD.UpdateClientData(pcldata, flTime);
	IN_Commands();
	return gExportfuncs.HUD_UpdateClientData(pcldata, flTime);
}

void HUD_Frame(double time)
{
	Gamma_Check();

	GetClientVoice()->Frame(time);
}

void HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(g_vecEyePos);

	g_vecEyePos[0] += ppmove->origin[0];
	g_vecEyePos[1] += ppmove->origin[1];
	g_vecEyePos[2] += ppmove->origin[2];

	return gExportfuncs.HUD_PlayerMove(ppmove, server);
}

void HUD_Shutdown(void)
{
	Version_Shutdown();
	ShutdownInput();
	return gExportfuncs.HUD_Shutdown();
}

void HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	gExportfuncs.HUD_VoiceStatus(entindex, bTalking);

	if (entindex >= 0 && entindex < gEngfuncs.GetMaxClients())
	{
		if (bTalking)
		{
			g_PlayerExtraInfo[entindex].radarflashes = 99999;
			g_PlayerExtraInfo[entindex].radarflash = gHUD.m_flTime;
			g_PlayerExtraInfo[entindex].radarflashon = 1;
		}
		else
		{
			g_PlayerExtraInfo[entindex].radarflash = -1;
			g_PlayerExtraInfo[entindex].radarflashon = 0;
			g_PlayerExtraInfo[entindex].radarflashes = 0;
		}
	}

	GetClientVoice()->UpdateSpeakerStatus(entindex, bTalking);
	
}

// Client/GameStudioModelRenderer.cpp
int GameStudioModelRenderer_InstallHook(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);

int HUD_GetStudioModelInterface(int iVersion, struct r_studio_interface_s **ppStudioInterface, struct engine_studio_api_s *pEngineStudio)
{
	Window_VidInit();
	CVAR_Init();

	int result = gExportfuncs.HUD_GetStudioModelInterface(iVersion, ppStudioInterface, pEngineStudio);
	GameStudioModelRenderer_InstallHook(iVersion, ppStudioInterface, pEngineStudio);
	return result;
}

void HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	PM_Init(ppmove);

	gExportfuncs.HUD_PlayerMoveInit(ppmove);
}


// client/input.cpp
int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding);

// cdll_int.cpp
void HUD_DirectorMessage(int iSize, void *pbuf);

// hl_weapons.cpp
void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);

void Exportfuncs_InstallHook(cl_exportfuncs_t *pExportFunc)
{
	pExportFunc->Initialize = Initialize;
	pExportFunc->HUD_Init = HUD_Init;
	pExportFunc->HUD_VidInit = HUD_VidInit;
	pExportFunc->HUD_Redraw = HUD_Redraw;
	pExportFunc->HUD_UpdateClientData = HUD_UpdateClientData;
	pExportFunc->HUD_Frame = HUD_Frame;
	pExportFunc->HUD_PlayerMove = HUD_PlayerMove;
	pExportFunc->HUD_Shutdown = HUD_Shutdown;
	pExportFunc->HUD_VoiceStatus = HUD_VoiceStatus;
	pExportFunc->HUD_GetStudioModelInterface = HUD_GetStudioModelInterface;
	pExportFunc->HUD_PlayerMoveInit = HUD_PlayerMoveInit;

	// tri.cpp
	pExportFunc->HUD_DrawNormalTriangles = HUD_DrawNormalTriangles;
	pExportFunc->HUD_DrawTransparentTriangles = HUD_DrawTransparentTriangles;

	// entity.cpp
	pExportFunc->HUD_AddEntity = HUD_AddEntity;
	pExportFunc->HUD_ProcessPlayerState = HUD_ProcessPlayerState;
	pExportFunc->HUD_TxferLocalOverrides = HUD_TxferLocalOverrides;
	pExportFunc->HUD_TxferPredictionData = HUD_TxferPredictionData;
	pExportFunc->HUD_CreateEntities = HUD_CreateEntities;
	//pExportFunc->HUD_StudioEvent = HUD_StudioEvent;

	// input.cpp
	pExportFunc->HUD_Key_Event = HUD_Key_Event;
	pExportFunc->CL_CreateMove = CL_CreateMove;
	

	// cdll_int.cpp
	pExportFunc->HUD_DirectorMessage = HUD_DirectorMessage;

	// inputw32.cpp
	pExportFunc->IN_ActivateMouse = IN_ActivateMouse;
	pExportFunc->IN_DeactivateMouse = IN_DeactivateMouse;
	pExportFunc->IN_MouseEvent = IN_MouseEvent;
	pExportFunc->IN_Accumulate = IN_Accumulate;
	pExportFunc->IN_ClearStates = IN_ClearStates;

	// in_camera.cpp
	pExportFunc->CAM_Think = CAM_Think;
	pExportFunc->CL_IsThirdPerson = CL_IsThirdPerson;

	// view.cpp
	pExportFunc->V_CalcRefdef = V_CalcRefdef;

	// demo.cpp
	pExportFunc->Demo_ReadBuffer = Demo_ReadBuffer;

	// input.cpp
	pExportFunc->KB_Find = KB_Find;
	
	// hl_weapons.cpp
	pExportFunc->HUD_PostRunCmd = HUD_PostRunCmd;
}