#include <metahook.h>
#include <sys.h>

char *Sys_GetLongPathName(void);
extern IFileSystem *g_pFileSystem; // launcher.cpp

void(__fastcall *g_pfnIFileSystem_Mount)(void *pthis, int edx) = nullptr;
void(__fastcall *g_pfnIFileSystem_Unmount)(void *pthis, int edx) = nullptr;
void(__fastcall *g_pfnIFileSystem_RemoveAllSearchPaths)(void *pthis, int edx) = nullptr;
void(__fastcall *g_pfnIFileSystem_AddSearchPath)(void *pthis, int edx, const char *pPath, const char *pathID) = nullptr;

using IFileSystem_Part = IBaseInterface;
class CNewFileSystem : public IFileSystem_Part
{
public:
	virtual void Mount(void);
	virtual void Unmount(void);
	virtual void RemoveAllSearchPaths(void);
	virtual void AddSearchPath(const char *pPath, const char *pathID = 0);
	// ...
};

void CNewFileSystem::Mount(void)
{
	return g_pfnIFileSystem_Mount(this, 0);
}

void CNewFileSystem::Unmount(void)
{
	return g_pfnIFileSystem_Unmount(this, 0);
}

void CNewFileSystem::RemoveAllSearchPaths(void)
{
	return g_pfnIFileSystem_Unmount(this, 0);
}

void CNewFileSystem::AddSearchPath(const char *pPath, const char *pathID)
{
	static bool bInitalized = false;

	if (!bInitalized && !strcmp(pathID, "GAME"))
	{
		char szThanatosDir[128];
		sprintf(szThanatosDir, "%s\\thanatos", Sys_GetLongPathName());
		g_pfnIFileSystem_AddSearchPath(this, 0, szThanatosDir, pathID);
		bInitalized = true;
	}

	return g_pfnIFileSystem_AddSearchPath(this, 0, pPath, pathID);
}

void FileSystem_InstallHook(IFileSystem *pFileSystem)
{
	g_pFileSystem = pFileSystem;

	CNewFileSystem NewFileSystem;
	DWORD *pVFTable = *(DWORD **)&NewFileSystem;

	//g_pMetaHookAPI->VFTHook(g_pFileSystem, 0, 1, (void *)pVFTable[1], (void *&)g_pfnIFileSystem_Mount);
	//g_pMetaHookAPI->VFTHook(g_pFileSystem, 0, 2, (void *)pVFTable[2], (void *&)g_pfnIFileSystem_Unmount);
	//g_pMetaHookAPI->VFTHook(g_pFileSystem, 0, 3, (void *)pVFTable[3], (void *&)g_pfnIFileSystem_RemoveAllSearchPaths);
	g_pMetaHookAPI->VFTHook(g_pFileSystem, 0, 4, (void *)pVFTable[4], (void *&)g_pfnIFileSystem_AddSearchPath);
}
