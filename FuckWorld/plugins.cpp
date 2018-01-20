#include <metahook.h>
#include "exportfuncs.h"

#include "plugins.h"
#include "Window.h"
#include "BaseUI.h"
#include "Info.h"
#include "SteamAPI.h"
#include "Video.h"
#include "Gamma.h"
#include "Modules.h"
#include "Hook_FileSystem.h"
#include "mempatchs.h"

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;

HINSTANCE g_hInstance;
extern HINSTANCE g_hThisModule, g_hEngineModule; // launcher.cpp, metahook.cpp
extern DWORD g_dwEngineBase, g_dwEngineSize; // metahook.cpp
DWORD g_dwEngineBuildnum;
DWORD g_iVideoMode;
int g_iVideoWidth, g_iVideoHeight, g_iBPP;
bool g_bWindowed;
extern bool g_bIsNewEngine; // metahook.cpp
bool g_bIsUseSteam;
bool g_bIsRunningSteam;
bool g_bIsDebuggerPresent;
extern IFileSystem *g_pFileSystem; // launcher.cpp


void IPlugins::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	BOOL(*IsDebuggerPresent)(void) = (BOOL(*)(void))GetProcAddress(GetModuleHandle("kernel32.dll"), "IsDebuggerPresent");

	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;

	g_hInstance = GetModuleHandle(NULL);
	g_bIsUseSteam = false;
	g_bIsRunningSteam = false;
	g_bIsDebuggerPresent = IsDebuggerPresent() != FALSE;

	FileSystem_InstallHook(g_pInterface->FileSystem);
	Module_InstallHook();
	Window_Init();
}

void IPlugins::Shutdown(void)
{
	Module_Shutdown();
}

void IPlugins::LoadEngine(void)
{
	//g_pFileSystem = g_pInterface->FileSystem;
	g_iVideoMode = g_pMetaHookAPI->GetVideoMode(&g_iVideoWidth, &g_iVideoHeight, &g_iBPP, &g_bWindowed);
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
	g_iBPP = 32;

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
		return;

	g_hEngineModule = g_pMetaHookAPI->GetEngineModule();
	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();

	MemPatch_Start(MEMPATCH_STEP_LOADENGINE);

	SteamAPI_InstallHook();
	BaseUI_InstallHook();
	Info_InstallHook();
}

void ViewPort_InstallHook(cl_exportfuncs_t *pExportfuncs); // ViewPort_Interface.cpp

void IPlugins::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	g_pMetaHookAPI->WriteMemory(&gExportfuncs, (BYTE *)pExportFunc, sizeof(gExportfuncs));
	//memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

	Module_LoadClient(pExportFunc);
	Exportfuncs_InstallHook(pExportFunc);
	ViewPort_InstallHook(pExportFunc);

	MemPatch_Start(MEMPATCH_STEP_LOADCLIENT);

	Window_LoadClient();
	VID_Init();
}

void IPlugins::ExitGame(int iResult)
{
	VID_Shutdown();
	Gamma_Shutdown();
}

EXPOSE_SINGLE_INTERFACE(IPlugins, IPlugins, METAHOOK_PLUGIN_API_VERSION);