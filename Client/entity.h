
int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
void HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client);
void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src);
void HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
void HUD_CreateEntities(void);
void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);
