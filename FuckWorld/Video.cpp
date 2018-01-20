#define _WIN32_WINNT 0x0501
#include <metahook.h>
#include "gamma.h"
#include "Video.h"
#include "Plugins.h"
#include <keydefs.h>

HWND g_hMainWnd = NULL;
HDC g_hMainDC;
WNDPROC g_WndProc;

BOOL g_fActiveApp = FALSE;
BOOL g_fMinimized = FALSE;

BOOL (WINAPI *g_pfnDestroyWindow)(HWND hWnd);
BOOL (WINAPI *g_pfnSetCursorPos)(int X, int Y);

hook_t *g_phDestroyWindow = NULL;
hook_t *g_phSetCursorPos = NULL;
hook_t *g_phChangeDisplaySettingsA;
hook_t *g_phShowWindow;

HRESULT (CALLBACK *g_pVID_EnumDisplayModesProc)(void *lpDDSurfaceDesc, DWORD *pBPP);
hook_t *g_phVID_EnumDisplayModesProc;

CVideoMode_Common *(*g_pfnVideoMode_Create)(void);
hook_t *g_phVideoMode_Create;

bool g_bNeedRestart = false;

extern int g_iVideoWidth, g_iVideoHeight, g_iBPP;
extern bool g_bWindowed;

CVideoMode_Common *g_pVideoMode = NULL;

bool mouse_rawinput = false;
int mouse_xdelta = 0;
int mouse_ydelta = 0;
int mouse_x = 0, mouse_y = 0;
int mouse_last_x = 0, mouse_last_y = 0;
int mouse_button = 0;
bool mouse_relative = false;
bool mouse_lock = false;

BOOL WINAPI VID_DestroyWindow(HWND hWnd)
{
	if (hWnd == g_hMainWnd)
		Gamma_Restore();

	return g_pfnDestroyWindow(hWnd);
}

BOOL WINAPI VID_SetCursorPos(int X, int Y)
{
	if (!g_fActiveApp || g_fMinimized)
		return TRUE;

	if (X < 0 || Y < 0)
		return TRUE;

	return g_pfnSetCursorPos(X, Y);
}

BOOL VID_IsMinimized(void)
{
	return g_fMinimized;
}

HWND VID_GetMainWindow(void)
{
	return g_hMainWnd;
}

void VID_SwitchFullScreen(bool windowed, bool native)
{
	g_bWindowed = windowed;

	if (!windowed)
	{
		DEVMODE dm;
		memset(&dm, 0, sizeof(dm));

		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = g_iVideoWidth;
		dm.dmPelsHeight = g_iVideoHeight;
		dm.dmBitsPerPel = g_iBPP;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		char *value;

		if (gEngfuncs.CheckParm("-freq", &value))
		{
			dm.dmFields |= DM_DISPLAYFREQUENCY;
			dm.dmDisplayFrequency = atoi(value);
		}

		int result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

		if (result != DISP_CHANGE_SUCCESSFUL)
		{
			gEngfuncs.Con_DPrintf("ChangeDisplaySettings failed, result:%d, error:%d\n", result, GetLastError());
			return;
		}

		int x, y, width, height;
		int style = WS_CLIPSIBLINGS | WS_POPUP | WS_VISIBLE;

		SetWindowLong(g_hMainWnd, GWL_STYLE, style);
		SetWindowLong(g_hMainWnd, GWL_EXSTYLE, 0);

		width = g_iVideoWidth;
		height = g_iVideoHeight;
		x = 0;
		y = 0;

		MoveWindow(g_hMainWnd, x, y, width, height, TRUE);

		UpdateWindow(g_hMainWnd);
		ShowWindow(g_hMainWnd, SW_SHOW);
		SetForegroundWindow(g_hMainWnd);
		SetFocus(g_hMainWnd);

		Gamma_Check();
		VideoMode_SetWindowed(false);
	}
	else
	{
		int x, y, width, height;
		int style = WS_CLIPSIBLINGS | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX;

		if (native)
			style &= ~WS_DLGFRAME;

		SetWindowLong(g_hMainWnd, GWL_STYLE, style);
		SetWindowLong(g_hMainWnd, GWL_EXSTYLE, WS_EX_WINDOWEDGE);

		width = g_iVideoWidth;
		height = g_iVideoHeight;

		RECT rc;
		rc.left = rc.top = 0;
		rc.right = width;
		rc.bottom = height;

		AdjustWindowRectEx(&rc, style, FALSE, 0);
		ChangeDisplaySettings(NULL, 0);

		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		int screenWidth, screenHeight;

		if (native)
		{
			RECT workarea;
			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&workarea, 0);

			screenWidth = (workarea.right - workarea.left);
			screenHeight = (workarea.bottom - workarea.top);

			if (screenWidth < width || screenHeight < height)
			{
				screenWidth = GetSystemMetrics(SM_CXSCREEN);
				screenHeight = GetSystemMetrics(SM_CYSCREEN);
			}
		}
		else
		{
			screenWidth = GetSystemMetrics(SM_CXSCREEN);
			screenHeight = GetSystemMetrics(SM_CYSCREEN);
		}

		x = (screenWidth - width) / 2;
		y = (screenHeight - height) / 2;

		MoveWindow(g_hMainWnd, x, y, width, height, TRUE);

		UpdateWindow(g_hMainWnd);
		ShowWindow(g_hMainWnd, SW_SHOW);
		SetForegroundWindow(g_hMainWnd);
		SetFocus(g_hMainWnd);

		Gamma_Restore();
		VideoMode_SetWindowed(true);
	}

	gEngfuncs.Con_DPrintf("VID_SwitchFullScreen: switch %s\n", windowed ? "window" : "fullscreen");
}

bool VID_CanRawInput(void)
{
	return mouse_rawinput;
}

void VID_ReleaseMouseLock(void)
{
	ClipCursor(NULL);
}

void VID_SetMouseLock(bool enable)
{
	if (!g_fActiveApp)
		enable = false;

	if (enable)
	{
		LONG cx, cy;
		RECT rect;
		GetWindowRect(g_hMainWnd, &rect);

		if (mouse_relative)
		{
			cx = (rect.left + rect.right) / 2;
			cy = (rect.top + rect.bottom) / 2;

			rect.left = cx - 1;
			rect.right = cx + 1;
			rect.top = cy - 1;
			rect.bottom = cy + 1;
		}

		ClipCursor(&rect);
	}
	else
	{
		ClipCursor(NULL);
	}

	mouse_lock = enable;
}

void VID_SetRelativeMouseMode(bool enabled)
{
	mouse_relative = enabled;
}

BOOL VID_GetRelativeMouseState(int *x, int *y)
{
	if (x)
		*x = mouse_xdelta;

	if (y)
		*y = mouse_ydelta;

	mouse_xdelta = 0;
	mouse_ydelta = 0;

	return mouse_rawinput;
}

int VID_GetRelativeMouseButton(void)
{
	int button = mouse_button;

	mouse_button = 0;
	return button;
}

void VID_MouseButton(int state)
{
	mouse_button = 0;

	if (state & RI_MOUSE_BUTTON_1_DOWN)
		mouse_button |= 1;

	if (state & RI_MOUSE_BUTTON_2_DOWN)
		mouse_button |= 2;

	if (state & RI_MOUSE_BUTTON_3_DOWN)
		mouse_button |= 2;
}

void VID_MouseMotion(int x, int y)
{
	int xrel = x;
	int yrel = y;

	x = (mouse_last_x + x);
	y = (mouse_last_y + y);

	mouse_x += xrel;
	mouse_y += yrel;

	mouse_xdelta += xrel;
	mouse_ydelta += yrel;

	mouse_last_x = x;
	mouse_last_y = y;
}

void VID_ClearMouseState(void)
{
	mouse_x = 0;
	mouse_y = 0;

	mouse_xdelta = 0;
	mouse_ydelta = 0;

	mouse_last_x = 0;
	mouse_last_y = 0;

	mouse_button = 0;
}

void VID_VidInit(void)
{
	VID_ClearMouseState();
}

void VID_ClearButtons(void)
{
	for (int i = 0; i < 256; i++)
	{
		gEngfuncs.Key_Event(i, false);
	}
}

LRESULT VID_MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);

			if (width && height)
			{
				g_iVideoWidth = width;
				g_iVideoHeight = height;
			}

			break;
		}

		case WM_ACTIVATEAPP:
		{
			g_fActiveApp = LOWORD(wParam);
			g_fMinimized = HIWORD(wParam) != WA_INACTIVE;

			if (g_fActiveApp && !g_fMinimized)
			{
				Gamma_Check();
				VID_ClearMouseState();

				if (mouse_lock)
					VID_SetMouseLock(mouse_lock);
			}
			else
			{
				Gamma_Restore();
			}

			LRESULT lpResult = CallWindowProc(g_WndProc, hWnd, uMsg, wParam, lParam);

			if (!g_fActiveApp)
				VID_ClearButtons();

			return lpResult;
		}

		case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			if (!mouse_rawinput)
				break;

			GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			if (inp.header.dwType == RIM_TYPEMOUSE)
			{
				RAWMOUSE *mouse = &inp.data.mouse;

				if ((mouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
				{
					VID_MouseMotion((int)mouse->lLastX, (int)mouse->lLastY);
				}
				else
				{
					static POINT initialMousePoint = { 0, 0 };

					if (initialMousePoint.x == 0 && initialMousePoint.y == 0)
					{
						initialMousePoint.x = mouse->lLastX;
						initialMousePoint.y = mouse->lLastY;
					}

					VID_MouseMotion((int)(mouse->lLastX-initialMousePoint.x), (int)(mouse->lLastY-initialMousePoint.y));

					initialMousePoint.x = mouse->lLastX;
					initialMousePoint.y = mouse->lLastY;
				}

				VID_MouseButton(mouse->usButtonFlags);
			}

			break;
		}

		case WM_COPYDATA:
		{
			COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT*)lParam;

			char command[128];
			strcpy(command, (char *)pCopyData->lpData);
			command[pCopyData->cbData] = 0;

			gEngfuncs.pfnClientCmd(command);
			return 1;
		}

#ifdef _DEBUG
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			if (gEngfuncs.pfnGetCvarFloat("in_focusdebug") > 0)
			{
				void IN_NeedKBInput(void);
				void IN_CalculateMouseVisible(void);
				void IN_TestMouseVisible(void);

				if (GetKeyState(VK_SPACE) & 0x80000000)
				{
					if (GetKeyState(VK_LBUTTON) & 0x80000000)
					{
						IN_NeedKBInput();
					}
					else if (GetKeyState(VK_RBUTTON) & 0x80000000)
					{
						IN_CalculateMouseVisible();
					}
					else if (GetKeyState(VK_MBUTTON) & 0x80000000)
					{
						IN_TestMouseVisible();
					}
				}
			}

			break;
		}

		case WM_PAINT:
		{
			break;
		}
#endif
	}

	if (g_pVideoMode)
	{
		switch (uMsg)
		{
			case WM_SYSKEYDOWN:
			{
				case VK_MENU:
				{
					if (GetKeyState(VK_RETURN) & 0x80000000)
					{
						bool windowed = !g_bWindowed;
						const char *name = g_pVideoMode->GetName();

						if (name)
						{
							char szCmd[256];
							sprintf(szCmd, "_setrenderer %s %s\n", name, windowed ? "windowed" : "fullscreen");
							gEngfuncs.pfnClientCmd(szCmd);
						}

						VID_SwitchFullScreen(windowed, (GetKeyState(VK_SHIFT) & 0x80000000) ? true : false);
						return 1;
					}

					break;
				}
			}
		}
	}

	return CallWindowProc(g_WndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK VID_EnumWindowsProc(HWND hWnd, LPARAM)
{
	char szClassName[32];
	GetClassName(hWnd, szClassName, sizeof(szClassName));

	if (!strcmp(szClassName, "Valve001") || !strcmp(szClassName, "SDL_app"))
	{
		g_hMainWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

HRESULT CALLBACK VID_EnumDisplayModesProc(void *lpDDSurfaceDesc, DWORD *pBPP)
{
	DWORD dwWidth = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0xC);
	DWORD dwHeight = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0x8);
	DWORD dwBPP = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0x54);

	if (dwHeight >= 480 && dwWidth > dwHeight && dwBPP == *pBPP)
		g_pVideoMode->AddMode(dwWidth, dwHeight, dwBPP);

	return 1;
}

void VID_GetVideoSize(void)
{
	RECT rc = { 0, 0, 0, 0 };

	if (GetClientRect(g_hMainWnd, &rc))
	{
		g_iVideoWidth = rc.right - rc.left;
		g_iVideoHeight = rc.bottom - rc.top;
	}
}

void VID_Init(void)
{
	EnumThreadWindows(GetCurrentThreadId(), VID_EnumWindowsProc, 0);

	if (g_hMainWnd == NULL)
		g_hMainWnd = FindWindow("Valve001", NULL);

	g_hMainDC = GetDC(g_hMainWnd);
	g_WndProc = (WNDPROC)GetWindowLong(g_hMainWnd, GWL_WNDPROC);
	g_phDestroyWindow = g_pMetaHookAPI->InlineHook((void *)DestroyWindow, VID_DestroyWindow, (void *&)g_pfnDestroyWindow);
	g_phSetCursorPos = g_pMetaHookAPI->InlineHook((void *)SetCursorPos, VID_SetCursorPos, (void *&)g_pfnSetCursorPos);

	if (SetWindowLong(g_hMainWnd, GWL_WNDPROC, (LONG)VID_MainWndProc) == 0)
	{
		gEngfuncs.Con_Printf("VID_Init: Can't set new WndProc (%d)!!\n", GetLastError());
		return;
	}

	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x02;
	Rid.dwFlags = RIDEV_CAPTUREMOUSE;
	Rid.hwndTarget = g_hMainWnd;

	if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) != FALSE)
		mouse_rawinput = true;

	if (CommandLine()->CheckParm("-restart"))
	{
		ShowWindow(g_hMainWnd, SW_SHOW);
		SetActiveWindow(g_hMainWnd);
		SetForegroundWindow(g_hMainWnd);
		SetFocus(g_hMainWnd);
	}

	if (GetActiveWindow() == g_hMainWnd)
		g_fActiveApp = TRUE;

	if (IsIconic(g_hMainWnd))
		g_fMinimized = TRUE;

	VID_GetVideoSize();
}

void VID_Shutdown(void)
{
	VID_ReleaseMouseLock();

	if (mouse_rawinput)
	{
		RAWINPUTDEVICE Rid;
		Rid.usUsagePage = 0x01; 
		Rid.usUsage = 0x02; 
		Rid.dwFlags = RIDEV_REMOVE;
		Rid.hwndTarget = NULL;

		RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
	}

	SetWindowLong(g_hMainWnd, GWL_WNDPROC, (LONG)g_WndProc);

	if (g_phDestroyWindow)
		g_pMetaHookAPI->UnHook(g_phDestroyWindow);

	if (g_phSetCursorPos)
		g_pMetaHookAPI->UnHook(g_phSetCursorPos);
}

void VID_SetCSOModels(bool status)
{
	char szCmd[256];
	sprintf(szCmd, "_sethdmodels %i\n", status);
	gEngfuncs.pfnClientCmd(szCmd);

	g_bNeedRestart = true;
}

void VID_SetVideoMode(int width, int height, int bpp)
{
	char szCmd[256];
	sprintf(szCmd, "_setvideomode %i %i %i\n", width, height, bpp);
	gEngfuncs.pfnClientCmd(szCmd);

	if (width != g_iVideoWidth || height != g_iVideoHeight)
	{
		g_iVideoWidth = width;
		g_iVideoHeight = height;
		g_bNeedRestart = true;
	}
	else
		g_bNeedRestart = false;
}

void VID_GetCurrentRenderer(char *name, int namelen, int *windowed)
{
	*windowed = g_bWindowed;

	if (name)
	{
		switch (g_iVideoMode)
		{
			case VIDEOMODE_OPENGL:
			{
				strncpy(name, "gl", namelen - 1);
				name[namelen - 1] = 0;
				break;
			}

			case VIDEOMODE_D3D:
			{
				strncpy(name, "d3d", namelen - 1);
				name[namelen - 1] = 0;
				break;
			}
		}
	}
}
void VID_SetRenderer(char *renderer, bool windowed)
{
	char szCmd[256];
	sprintf(szCmd, "_setrenderer %s %s\n", renderer, windowed ? "windowed" : "fullscreen");
	gEngfuncs.pfnClientCmd(szCmd);
}

void VID_Restart(void)
{
	if (g_bNeedRestart)
	{
		VID_Shutdown();

		gEngfuncs.pfnClientCmd("_restart\n");
	}
}

void VideoMode_SetWindowed(bool state)
{
	*(bool *)((DWORD)g_pVideoMode + 440) = state;
}

bool VideoMode_IsWindowed(void)
{
	return *(bool *)((DWORD)g_pVideoMode + 440);
}

CVideoMode_Common *VideoMode_Create(void)
{
	g_pVideoMode = g_pfnVideoMode_Create();
	return g_pVideoMode;
}

void VID_HideWindow(void)
{
	ShowWindow(g_hMainWnd, SW_HIDE);
}

void VID_CloseWindow(void)
{
	g_pfnDestroyWindow(g_hMainWnd);
}