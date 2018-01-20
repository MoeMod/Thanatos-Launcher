extern cl_enginefunc_t gEngfuncs;

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void HUD_Init(void);
int HUD_Redraw(float time, int intermission);
void HUD_VoiceStatus(int entindex, qboolean bTalking);
int HUD_GetStudioModelInterface(int iVersion, struct r_studio_interface_s **ppStudioInterface, struct engine_studio_api_s *pEngineStudio);


void Exportfuncs_InstallHook(cl_exportfuncs_t *pExportFunc);