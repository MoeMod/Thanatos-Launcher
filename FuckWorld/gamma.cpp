#include <metahook.h>
#include <math.h>
#include "plugins.h"
#include "CVARs.h"
#include "Video.h"

cvar_t *g_pcvarGamma = NULL;

BOOL g_bSupportGamma = false;
float g_flCurrentGamma = -1.0;
word g_gammaRamp[768];
word g_stateRamp[768];

void Gamma_Init(void)
{
	g_pcvarGamma = gEngfuncs.pfnRegisterVariable("bte_monitorgamma", "2.2", FCVAR_ARCHIVE);

	if (g_bSupportGamma)
		return;

	if (CommandLine()->CheckParm("-nogamma"))
		return;

	if (g_bWindowed)
		return;

	memset(g_stateRamp, 0, sizeof(g_stateRamp));
	g_bSupportGamma = GetDeviceGammaRamp(g_hMainDC, g_stateRamp);
}

BOOL Gamma_IsSupport(void)
{
	return g_bSupportGamma;
}

#ifndef bound
#define bound(a, b, c) max(min(b, c), a)
#endif

void Gamma_Build(void)
{
	double invGamma, div;

	invGamma = 1.0 / (3.1 - bound(1.6, g_flCurrentGamma, 2.6));
	div = (double)1.0 / 255.5;

	memcpy(g_gammaRamp, g_stateRamp, sizeof(g_gammaRamp));

	for (int i = 0; i < 256; i++)
	{
		word v = (word)(65535.0 * pow(((double)i + 0.5) * div, invGamma) + 0.5);

		g_gammaRamp[i + 0] = bound(0, v, 65535);
		g_gammaRamp[i + 256] = bound(0, v, 65535);
		g_gammaRamp[i + 512] = bound(0, v, 65535);
	}
}

void Gamma_Check(void)
{
	if (g_bWindowed)
		return;

	static bool initialize = false;

	if (!g_bSupportGamma)
	{
		if (initialize == false)
		{
			Gamma_Init();
			initialize = true;
		}

		return;
	}

	if (g_flCurrentGamma != g_pcvarGamma->value)
	{
		g_flCurrentGamma = g_pcvarGamma->value;
		Gamma_Build();
		SetDeviceGammaRamp(g_hMainDC, g_gammaRamp);
	}
}

void Gamma_Restore(void)
{
	if (g_bWindowed)
		return;

	if (!g_bSupportGamma)
		return;

	g_flCurrentGamma = -1;
	SetDeviceGammaRamp(g_hMainDC, g_stateRamp);
}

void Gamma_Shutdown(void)
{
	Gamma_Restore();
}