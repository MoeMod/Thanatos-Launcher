#include "hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "in_defs.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define	CAM_MODE_RELAX 1
#define CAM_MODE_FOCUS 2

vec3_t v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles, v_lastFacing;
float v_frametime, v_lastDistance;
float v_cameraRelaxAngle = 5.0f;
float v_cameraFocusAngle = 35.0f;
int v_cameraMode = CAM_MODE_FOCUS;
qboolean v_resetCamera = 1;

cvar_t *cl_chasedist;
cvar_t *v_centerspeed;

extern "C"
{
	void PM_ParticleLine(float *start, float *end, int pcolor, float life, float vert);
	int PM_GetVisEntInfo(int ent);
	int PM_GetPhysEntInfo(int ent);
	void InterpolateAngles(float *start, float *end, float *output, float frac);
	void NormalizeAngles(float *angles);
	float Distance(const float *v1, const float *v2);
	float AngleBetweenVectors(const float *v1, const float *v2);

	float vJumpOrigin[3];
	float vJumpAngles[3];
}

typedef struct pitchdrift_s
{
	float pitchvel;
	int nodrift;
	float driftmove;
	double laststop;
}
pitchdrift_t;
static pitchdrift_t pd;

void V_StartPitchDrift(void)
{
	if (pd.laststop == gEngfuncs.GetClientTime())
		return;

	if (pd.nodrift || !pd.pitchvel)
	{
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift(void)
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift = 1;
	pd.pitchvel = 0;
}

void V_GetChaseOrigin(float *angles, float *origin, float distance, float *returnvec)
{
	vec3_t vecEnd;
	vec3_t forward;
	vec3_t vecStart;
	pmtrace_t *trace;
	int maxLoops = 8;

	int ignoreent = -1;

	cl_entity_t *ent = NULL;

	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, -1, forward);
	VectorCopy(origin, vecStart);
	VectorMA(vecStart, distance, forward, vecEnd);

	while (maxLoops > 0)
	{
		trace = gEngfuncs.PM_TraceLine(vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent);

		if (trace->ent <= 0)
			break;

		ent = gEngfuncs.GetEntityByIndex(PM_GetPhysEntInfo(trace->ent));

		if (ent == NULL)
			break;

		if (ent->curstate.solid == SOLID_BSP && !ent->player)
			break;

		if (Distance(trace->endpos, vecEnd) < 1.0f)
		{
			break;
		}
		else
		{
			ignoreent = trace->ent;
			VectorCopy(trace->endpos, vecStart);
		}

		maxLoops--;
	}

	VectorMA(trace->endpos, 4, trace->plane.normal, returnvec);

	v_lastDistance = Distance(trace->endpos, origin);
}

float MaxAngleBetweenAngles(float *a1, float *a2)
{
	float d, maxd = 0.0f;

	NormalizeAngles(a1);
	NormalizeAngles(a2);

	for (int i = 0; i < 3; i++)
	{
		d = a2[i] - a1[i];

		if (d > 180)
		{
			d -= 360;
		}
		else if (d < -180)
		{
			d += 360;
		}

		d = fabs(d);

		if (d > maxd)
			maxd = d;
	}

	return maxd;
}

void V_SmoothInterpolateAngles(float *startAngle, float *endAngle, float *finalAngle, float degreesPerSec)
{
	float absd, frac, d, threshhold;

	NormalizeAngles(startAngle);
	NormalizeAngles(endAngle);

	for (int i = 0; i < 3; i++)
	{
		d = endAngle[i] - startAngle[i];

		if (d > 180.0f)
		{
			d -= 360.0f;
		}
		else if (d < -180.0f)
		{
			d += 360.0f;
		}

		absd = fabs(d);

		if (absd > 0.01f)
		{
			frac = degreesPerSec * v_frametime;
			threshhold = degreesPerSec / 4;

			if (absd < threshhold)
			{
				float h = absd / threshhold;
				h *= h;
				frac *= h;
			}

			if (frac > absd)
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if (d > 0)
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}
	}

	NormalizeAngles(finalAngle);
}

void V_GetSingleTargetCam(cl_entity_t *ent1, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;
	qboolean deadPlayer = ent1->player && (ent1->curstate.solid == SOLID_NOT);

	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
	float distance = 112.0f + (16.0f * dfactor);

	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;
	else if (deadPlayer)
		distance *= 1.5f;

	v_lastDistance += v_frametime * 32.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
	{
		if (deadPlayer)
			newOrigin[2] += 2;
		else
			newOrigin[2] += 17;
	}
	else
		newOrigin[2] += 8;

	VectorCopy(ent1->angles, newAngle);

	if (flags & DRC_FLAG_FACEPLAYER)
		newAngle[1] += 180.0f;

	newAngle[0] += 12.5f * dfactor;

	if (flags & DRC_FLAG_FINAL)
		newAngle[0] = 22.5f;

	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);
	V_GetChaseOrigin(angle, newOrigin, distance, origin);
}

void V_GetDoubleTargetsCam(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3]; float tempVec[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;
	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
	float distance = 112.0f + (16.0f * dfactor);

	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;

	v_lastDistance += v_frametime * 32.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
		newOrigin[2] += 17;
	else
		newOrigin[2] += 8;

	VectorSubtract(ent2->origin, ent1->origin, newAngle);
	VectorAngles(newAngle, newAngle);

	newAngle[0] = -newAngle[0];
	newAngle[0] += 12.5f * dfactor;

	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	float d = MaxAngleBetweenAngles(v_lastAngles, newAngle);

	if ((d < v_cameraFocusAngle) && (v_cameraMode == CAM_MODE_RELAX))
	{
		VectorCopy(v_lastAngles, newAngle);
	}
	else if ((d < v_cameraRelaxAngle) && (v_cameraMode == CAM_MODE_FOCUS))
	{
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		v_cameraMode = CAM_MODE_FOCUS;
	}

	if (v_resetCamera || (v_cameraMode == CAM_MODE_RELAX))
	{
		VectorCopy(newAngle, angle);
	}
	else
	{
		V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 180.0f);
	}

	V_GetChaseOrigin(newAngle, newOrigin, distance, origin);

	if (v_lastDistance < 64.0f)
		origin[2] += 16.0f * (1.0f - (v_lastDistance / 64.0f));

	VectorSubtract(ent2->origin, origin, tempVec);
	VectorAngles(tempVec, tempVec);
}

void V_GetDirectedChasePosition(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	if (v_resetCamera)
	{
		v_lastDistance = 4096.0f;
	}

	if ((ent2 == (cl_entity_t *)0xFFFFFFFF) || (ent1->player && (ent1->curstate.solid == SOLID_NOT)))
	{
		V_GetSingleTargetCam(ent1, angle, origin);
	}
	else if (ent2)
	{
		V_GetDoubleTargetsCam(ent1, ent2, angle, origin);
	}
	else
	{
		float newOrigin[3];

		int flags = gHUD.m_Spectator.m_iObserverFlags;
		float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
		float distance = 112.0f + (16.0f * dfactor);

		if (flags & DRC_FLAG_FINAL)
			distance *= 2.0f;

		v_lastDistance += v_frametime * 32.0f;

		if (distance > v_lastDistance)
			distance = v_lastDistance;

		VectorCopy(ent1->origin, newOrigin);

		if (ent1->player)
			newOrigin[2] += 17;
		else
			newOrigin[2] += 8;

		V_GetChaseOrigin(angle, newOrigin, distance, origin);
	}

	VectorCopy(angle, v_lastAngles);
}

void V_GetDeathCam(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3];
	float distance = 168.0f;

	v_lastDistance += v_frametime * 96.0f;

	if (v_resetCamera)
		v_lastDistance = 64.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
		newOrigin[2] += 17;

	if (ent2)
	{
		VectorSubtract(ent2->origin, ent1->origin, newAngle);
		VectorAngles(newAngle, newAngle);
		newAngle[0] = -newAngle[0];
	}
	else
	{
		newAngle[0] = 90.0f;
		newAngle[1] = 0.0f;
		newAngle[2] = 0;
	}

	V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);
	V_GetChaseOrigin(angle, newOrigin, distance, origin);

	VectorCopy(angle, v_lastAngles);
}

void V_GetChasePos(int target, float *cl_angles, float *origin, float *angles)
{
	cl_entity_t *ent = NULL;

	if (target)
		ent = gEngfuncs.GetEntityByIndex(target);

	if (!ent)
	{
		VectorCopy(vJumpAngles, angles);
		VectorCopy(vJumpOrigin, origin);
		return;
	}

	if (ent->index == gEngfuncs.GetLocalPlayer()->index)
	{
		if (g_iUser3)
			V_GetDeathCam(ent, gEngfuncs.GetEntityByIndex(g_iUser3), angles, origin);
		else
			V_GetDeathCam(ent, NULL, angles, origin);
	}
	else if (gHUD.m_Spectator.m_autoDirector->value)
	{
		if (g_iUser3)
			V_GetDirectedChasePosition(ent, gEngfuncs.GetEntityByIndex(g_iUser3), angles, origin);
		else
			V_GetDirectedChasePosition(ent, (cl_entity_t *)0xFFFFFFFF, angles, origin);
	}
	else
	{
		if (cl_angles == NULL)
		{
			VectorCopy(ent->angles, angles);
			angles[0] *= -1;
		}
		else
			VectorCopy(cl_angles, angles);

		VectorCopy(ent->origin, origin);

		origin[2] += 17;

		V_GetChaseOrigin(angles, origin, cl_chasedist->value, origin);
	}

	v_resetCamera = false;
}

void V_ResetChaseCam(void)
{
	v_resetCamera = true;
}

void V_GetInEyePos(int target, float *origin, float *angles)
{
	if (!target)
	{
		VectorCopy(vJumpAngles, angles);
		VectorCopy(vJumpOrigin, origin);
		return;
	}

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex(target);

	if (!ent)
		return;

	VectorCopy(ent->origin, origin);
	VectorCopy(ent->angles, angles);

	angles[PITCH] *= -3.0f;

	if (ent->curstate.solid == SOLID_NOT)
	{
		angles[ROLL] = 80;
		origin[2] += -8;
	}
	else if (ent->curstate.usehull == 1)
	{
		origin[2] += 12;
	}
	else
	{
		origin[2] += 17;
	}
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	g_pViewPort->CalcRefdef(pparams);

	gHUD.CalcRefdef(pparams);

	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
	v_cl_angles = pparams->cl_viewangles;
	v_sim_org = pparams->simorg;
	
	if (!pparams->paused)
	{
		v_lastAngles = v_angles;
	}
	return gExportfuncs.V_CalcRefdef(pparams);
}

void V_Init(void)
{
	Cmd_HookCmd("centerview", V_StartPitchDrift);

	cl_chasedist = gEngfuncs.pfnGetCvarPointer("cl_chasedist");
	v_centerspeed = gEngfuncs.pfnGetCvarPointer("v_centerspeed");
}