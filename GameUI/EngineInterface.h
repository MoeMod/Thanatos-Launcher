//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Includes all the headers/declarations necessary to access the
//			engine interface
//
// $NoKeywords: $
//=============================================================================

#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

// these stupid set of includes are required to use the cdll_int interface
#include <assert.h>
#include "port.h"
#include "archtypes.h"

typedef float vec_t;
typedef float vec2_t[2];

#ifndef vec3_t
typedef float vec3_t[3];
#endif

#include <wrect.h>
#define IN_BUTTONS_H
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

// engine interface
#include <cdll_int.h>

// engine interface singleton accessor
extern cl_enginefunc_t gEngfuncs;
extern cl_enginefunc_t *engine;
extern class IGameUIFuncs *gameuifuncs;

// from cl_util.h
#define HOOK_MESSAGE(x) engine->pfnHookUserMsg(#x, __MsgFunc_##x );
#define DECLARE_MESSAGE(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf)

#endif // ENGINEINTERFACE_H
