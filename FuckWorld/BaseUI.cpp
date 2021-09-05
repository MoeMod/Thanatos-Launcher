#include <metahook.h>
#include <plugins.h>

#include <IBaseUI.h>
#include <VGUI\ISurface.h>
#include <VGUI\IScheme.h>
#include <VGUI\ILocalize.h>
#include <vgui_controls/Controls.h>
#include <IKeyValuesSystem.h>

#include <IEngineVGui.h>

#include "vgui2.h"

#include "BaseUI.h"
#include "Hook_Scheme.h"
#include "Hook_KeyValuesSystem.h"
#include "Hook_Surface.h"
#include "Input.h"

void (__fastcall *g_pfnCBaseUI_Initialize)(void *pthis, int, CreateInterfaceFn *factories, int count);
void (__fastcall *g_pfnCBaseUI_Start)(void *pthis, int, struct cl_enginefuncs_s *engineFuncs, int interfaceVersion);
void (__fastcall *g_pfnCBaseUI_Shutdown)(void *pthis, int);
int (__fastcall *g_pfnCBaseUI_Key_Event)(void *pthis, int, int down, int keynum, const char *pszCurrentBinding);
void (__fastcall *g_pfnCBaseUI_CallEngineSurfaceProc)(void *pthis, int, void *hwnd, unsigned int msg, unsigned int wparam, long lparam);
void (__fastcall *g_pfnCBaseUI_Paint)(void *pthis, int, int x, int y, int right, int bottom);
void (__fastcall *g_pfnCBaseUI_HideGameUI)(void *pthis, int);
void (__fastcall *g_pfnCBaseUI_ActivateGameUI)(void *pthis, int);
bool (__fastcall *g_pfnCBaseUI_IsGameUIVisible)(void *pthis, int);
void (__fastcall *g_pfnCBaseUI_HideConsole)(void *pthis, int);
void (__fastcall *g_pfnCBaseUI_ShowConsole)(void *pthis, int);

// for 7561
void(__fastcall *g_pfnCBaseUI_CallEngineSurfaceAppHandler)(void *pthis, int, void *event, void *userData);

class CBaseUI : public IBaseUI
{
public:
	virtual void Initialize(CreateInterfaceFn *factories, int count);
	virtual void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion);
	virtual void Shutdown(void);

	virtual int Key_Event(int down, int keynum, const char *pszCurrentBinding);

	virtual void CallEngineSurfaceProc(void *hwnd, unsigned int msg, unsigned int wparam, long lparam);
	virtual void Paint(int x, int y, int right, int bottom);

	virtual void HideGameUI(void);
	virtual void ActivateGameUI(void);
	virtual bool IsGameUIVisible(void);
	virtual void HideConsole(void);
	virtual void ShowConsole(void);
};

IBaseUI *g_pBaseUI;
vgui2::IPanel *g_pPanel;

void CBaseUI::Initialize(CreateInterfaceFn *factories, int count)
{
	g_pfnCBaseUI_Initialize(this, 0, factories, count);

	CreateInterfaceFn fnVGUI2CreateInterface = Sys_GetFactoryThis(); // Sys_GetFactory((HINTERFACEMODULE)GetModuleHandle("vgui2.dll"));
	CreateInterfaceFn fnEngineCreateInterface = g_pMetaHookAPI->GetEngineFactory();

	vgui2::ISurface *pSurface = (vgui2::ISurface *)fnEngineCreateInterface(VGUI_SURFACE_INTERFACE_VERSION, NULL);
	vgui2::ISchemeManager *pSchemeManager = (vgui2::ISchemeManager *)fnVGUI2CreateInterface(VGUI_SCHEME_INTERFACE_VERSION, NULL);
	vgui2::ILocalize *pLocalize = (vgui2::ILocalize *)fnVGUI2CreateInterface(VGUI_LOCALIZE_INTERFACE_VERSION, NULL);
	vgui2::IInputInternal *pInput = (vgui2::IInputInternal *)fnVGUI2CreateInterface(VGUI_INPUT_INTERFACE_VERSION, NULL);

	IKeyValuesSystem *pKeyValuesSystem = (IKeyValuesSystem *)fnVGUI2CreateInterface(KEYVALUESSYSTEM_INTERFACE_VERSION, NULL);

	SchemeManager_InstallHook(pSchemeManager);
	//Input_InstallHook(pInput);
	KeyValuesSystem_InstallHook(pKeyValuesSystem);
	Surface_InstallHook(pSurface);

	g_pPanel = (vgui2::IPanel *)fnVGUI2CreateInterface(VGUI_PANEL_INTERFACE_VERSION, NULL);

	//vgui2::VGuiControls_Init("BaseUI", factories, count);
	factories[1] = fnVGUI2CreateInterface;
}

void CBaseUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion)
{
	g_pfnCBaseUI_Start(this, 0, engineFuncs, interfaceVersion);
}

void CBaseUI::Shutdown(void)
{
	g_pfnCBaseUI_Shutdown(this, 0);
}

int CBaseUI::Key_Event(int down, int keynum, const char *pszCurrentBinding)
{
	return g_pfnCBaseUI_Key_Event(this, 0, down, keynum, pszCurrentBinding);
}

void CBaseUI::CallEngineSurfaceProc(void *hwnd, unsigned int msg, unsigned int wparam, long lparam)
{
	g_pfnCBaseUI_CallEngineSurfaceProc(this, 0, hwnd, msg, wparam, lparam);
}

void __fastcall CBaseUI_CallEngineSurfaceAppHandler(void *pthis, int, void *event, void *userData)
{
	return g_pfnCBaseUI_CallEngineSurfaceAppHandler(pthis, 0, event, userData);
}

void CBaseUI::Paint(int x, int y, int right, int bottom)
{
	g_pfnCBaseUI_Paint(this, 0, x, y, right, bottom);
}

void CBaseUI::HideGameUI(void)
{
	g_pfnCBaseUI_HideGameUI(this, 0);
}

void CBaseUI::ActivateGameUI(void)
{
	g_pfnCBaseUI_ActivateGameUI(this, 0);
}

bool CBaseUI::IsGameUIVisible(void)
{
	return g_pfnCBaseUI_IsGameUIVisible(this, 0);
}

void CBaseUI::HideConsole(void)
{
	g_pfnCBaseUI_HideConsole(this, 0);
}

void CBaseUI::ShowConsole(void)
{
	g_pfnCBaseUI_ShowConsole(this, 0);
}

void BaseUI_InstallHook(void)
{
	CreateInterfaceFn fnCreateInterface = g_pMetaHookAPI->GetEngineFactory();
	g_pBaseUI = (IBaseUI *)fnCreateInterface(BASEUI_INTERFACE_VERSION, NULL);

	CBaseUI BaseUI;
	DWORD *pVFTable = *(DWORD **)&BaseUI;

	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 1, (void *)pVFTable[1], (void *&)g_pfnCBaseUI_Initialize);
	/*g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 2, (void *)pVFTable[2], (void *&)g_pfnCBaseUI_Start);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 3, (void *)pVFTable[3], (void *&)g_pfnCBaseUI_Shutdown);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 4, (void *)pVFTable[4], (void *&)g_pfnCBaseUI_Key_Event);
	
	if (g_bIsNewEngine)
		g_pMetaHookAPI->InlineHook((void *)pVFTable[5], CBaseUI_CallEngineSurfaceAppHandler, (void *&)g_pfnCBaseUI_CallEngineSurfaceAppHandler);
	else
		g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 5, (void *)pVFTable[5], (void *&)g_pfnCBaseUI_CallEngineSurfaceProc);

	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 6, (void *)pVFTable[6], (void *&)g_pfnCBaseUI_Paint);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 7, (void *)pVFTable[7], (void *&)g_pfnCBaseUI_HideGameUI);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 8, (void *)pVFTable[8], (void *&)g_pfnCBaseUI_ActivateGameUI);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 9, (void *)pVFTable[9], (void *&)g_pfnCBaseUI_IsGameUIVisible);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 10, (void *)pVFTable[10], (void *&)g_pfnCBaseUI_HideConsole);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 11, (void *)pVFTable[11], (void *&)g_pfnCBaseUI_ShowConsole);*/
}