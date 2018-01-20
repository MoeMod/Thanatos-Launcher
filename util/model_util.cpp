#include <metahook.h>
#include <com_model.h>
#include "sprite.h"

int ModelFrameCount(model_t *mod)
{
	if (!mod || !mod->cache.data)
		return 0;

	if (mod->type == mod_sprite)
	{
		return ((msprite_t *)mod->cache.data)->numframes;
	}
	else
		gEngfuncs.Con_DPrintf("ModelFrameCount nonsupport type: %d\n", mod->type);

	return 0;
}

int GetSpriteTexnum(model_t *mod, int frame)
{
	if (!mod || !mod->cache.data)
		return 0;

	if (mod->type == mod_sprite)
	{
		return ((msprite_t *)mod->cache.data)->frames[0].frameptr[frame].gl_texturenum;
	}
	else
		gEngfuncs.Con_DPrintf("GetSpriteTexnum nonsupport type: %d\n", mod->type);

	return 0;
}