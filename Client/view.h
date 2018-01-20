#if !defined (VIEWH)
#define VIEWH
#pragma once

void V_StartPitchDrift(void);
void V_StopPitchDrift(void);
void V_CalcRefdef(struct ref_params_s *pparams);

#endif