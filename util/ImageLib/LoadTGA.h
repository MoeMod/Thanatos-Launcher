#define LOADTGA_SIG "\x8B\x44\x24\x14\x8B\x4C\x24\x10\x8B\x54\x24\x0C\x6A\x01\x50\x8B\x44\x24\x10\x51\x8B\x4C\x24\x10\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\xC3"
#define LOADTGA_SIG_NEW "\x55\x8B\xEC\x8B\x45\x18\x8B\x4D\x14\x8B\x55\x10\x6A\x00\x50\x8B\x45\x0C\x51\x8B\x4D\x08\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\x5D\xC3"

extern int (*g_pfnLoadTGA)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
extern struct hook_s *g_phLoadTGA;

extern bool g_bLoadingTGA;
extern int g_iLastTGAWidth, g_iLastTGAHeight;
extern byte g_bTGABuffer[];

int LoadTGA(const char *name, byte *targa_rgba, int bufferSize, int *width, int *height, int FlipTga = 0);
bool GetTGASize(const char *szFilename, int *width, int *height);