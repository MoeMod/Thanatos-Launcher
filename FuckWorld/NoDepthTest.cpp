#include <metahook.h>
#include <gl/gl.h>

extern metahook_api_t gMetaHookAPI;

void (APIENTRY *qglBegin)(GLenum mode) = NULL;
void (APIENTRY *qglDisable)(GLenum mode) = NULL;

void WINAPI NDT_Hook_glBegin(GLenum mode)
{
	if (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
	{
		qglDisable(GL_DEPTH_TEST);
	}

	return qglBegin(mode);
}

void NDT_InstallHook()
{
	HMODULE hOpenGL = GetModuleHandle("opengl32.dll");
	qglBegin = (void (APIENTRY *)(GLenum))GetProcAddress(hOpenGL, "glBegin");
	qglDisable = (void (APIENTRY *)(GLenum))GetProcAddress(hOpenGL, "glDisable");

	gMetaHookAPI.InlineHook(qglBegin, NDT_Hook_glBegin, (void *&)qglBegin);
}