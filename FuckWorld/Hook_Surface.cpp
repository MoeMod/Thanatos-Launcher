#include <metahook.h>
#include <plugins.h>
#include <vgui/ISurface.h>
#include <IEngineSurface.h>

vgui2::ISurface *g_pSurface = NULL;

using namespace vgui2;

void(__fastcall *g_pfnCSurface_GetScreenSize)(void *pthis, int edx, int &wide, int &tall) = 0;


void __fastcall Hook_GetScreenSize(void* pthis, int edx, int &wide, int &tall)
{
	wide = g_iVideoWidth;
	tall = g_iVideoHeight;
	//return g_pfnCSurface_GetScreenSize(this, 0, wide, tall);
}

void Surface_InstallHook(vgui2::ISurface *pSurface)
{
	CreateInterfaceFn engineFactory = g_pMetaHookAPI->GetEngineFactory();
	g_pSurface = pSurface;

	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 32, (void *)Hook_GetScreenSize, (void *&)g_pfnCSurface_GetScreenSize);
}