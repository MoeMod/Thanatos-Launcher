#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

#include <winsani_in.h>
#include <windows.h>
#include <winsani_out.h>

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];

#include <wrect.h>
#include <interface.h>
#include <cdll_int.h>

extern cl_enginefunc_t *engine;

namespace vgui2
{
	class ISurface;
};

#endif