#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "r_efx.h"
#include "event_api.h"
#include "pm_shared.h"

#define IS_FIRSTPERSON_SPEC (g_iUser1 == OBS_IN_EYE || (g_iUser1 && (gHUD.m_Spectator.m_pip->value == INSET_IN_EYE)))
