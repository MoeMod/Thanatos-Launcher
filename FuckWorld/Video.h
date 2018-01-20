#define VID_ENUMDISPLAYMODESPROC_SIG "\x8B\x4C\x24\x04\x53\x56\x57\x8B\x51\x08\x8B\x41\x0C\x8B\x71\x54\x8B\xFA\xC1\xE7\x04\x8D\x1C\xC0\x32\xC9\x3B\xDF"

extern DWORD g_iVideoMode;
extern int g_iVideoWidth, g_iVideoHeight;
extern bool g_bWindowed;
extern BOOL g_fActiveApp;
extern BOOL g_fMinimized;

extern HWND g_hMainWnd;
extern HDC g_hMainDC;

extern HRESULT (CALLBACK *g_pVID_EnumDisplayModesProc)(void *lpDDSurfaceDesc, DWORD *pBPP);
extern struct hook_s *g_phVID_EnumDisplayModesProc;

BOOL CALLBACK VID_EnumWindowsProc(HWND hWnd, LPARAM);
HRESULT CALLBACK VID_EnumDisplayModesProc(void *lpDDSurfaceDesc, DWORD *pBPP);
void VID_Init(void);
void VID_Shutdown(void);
void VID_VidInit(void);
void VID_SetVideoMode(int width, int height, int bpp);
void VID_SetCSOModels(bool status);
void VID_SwitchFullScreen(bool windowed, bool native = false);
void VID_GetCurrentRenderer(char *name, int namelen, int *windowed);
void VID_SetRenderer(char *renderer, bool windowed);
void VID_Restart(void);
void VID_HideWindow(void);
void VID_CloseWindow(void);
bool VID_CanRawInput(void);
void VID_SetMouseLock(bool enable);
void VID_SetRelativeMouseMode(bool enabled);
BOOL VID_GetRelativeMouseState(int *x, int *y);
int VID_GetRelativeMouseButton(void);
void VID_ClearMouseState(void);
BOOL VID_IsMinimized(void);
HWND VID_GetMainWindow(void);

class CVideoMode_Common
{
public:
	virtual const char *GetName(void);
	virtual bool Init(void *pvInstance);
	virtual void Shutdown(void);
	virtual bool AddMode(int width, int height, int bpp);
	virtual struct vmode_s *GetCurrentMode(void);
	virtual struct vmode_s *GetMode(int num);
	virtual int GetModeCount(void);
	virtual bool IsWindowedMode(void);
	virtual bool GetInitialized(void);
	virtual void SetInitialized(bool init);
	virtual void UpdateWindowPosition(void);
	virtual void FlipScreen(void);
	virtual void RestoreVideo(void);
	virtual void ReleaseVideo(void);

public:
	virtual ~CVideoMode_Common(void);

public:
	virtual int MaxBitsPerPixel(void);
	virtual void ReleaseFullScreen(void);
	virtual void ChangeDisplaySettingsToFullscreen(void);
};

#define VIDEOMODE_CREATE_SIG "\x83\xEC\x08\x8B\x0D\x2A\x2A\x2A\x2A\x53\x56\x33\xF6\x8B\x01\x56\x68\x2A\x2A\x2A\x2A\xFF\x50\x08"
#define VIDEOMODE_CREATE_SIG_NEW "\x55\x8B\xEC\x51\x8B\x0D\x2A\x2A\x2A\x2A\x53\x6A\x00\x68\x2A\x2A\x2A\x2A\x8B\x01\xFF\x50\x08"

extern CVideoMode_Common *(*g_pfnVideoMode_Create)(void);
extern struct hook_s *g_phVideoMode_Create;

void VideoMode_SetWindowed(bool state);
bool VideoMode_IsWindowed(void);
CVideoMode_Common *VideoMode_Create(void);