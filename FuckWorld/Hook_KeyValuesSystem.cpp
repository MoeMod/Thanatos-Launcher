#include <metahook.h>
#include <Interface/IKeyValuesSystem.h>

IKeyValuesSystem *g_pKeyValuesSystem;
IKeyValuesSystem *KeyValuesSystem(void)
{
	return g_pKeyValuesSystem;
}

class CKeyValuesSystem : public IKeyValuesSystem
{
public:
	virtual void RegisterSizeofKeyValues(int size);
	virtual void *AllocKeyValuesMemory(int size);
	virtual void FreeKeyValuesMemory(void *pMem);
	virtual HKeySymbol GetSymbolForString(const char *name);
	virtual const char *GetStringForSymbol(HKeySymbol symbol);
	virtual HLocalized GetLocalizedFromANSI(const char *string);
	virtual const char *GetANSIFromLocalized(HLocalized l);
	virtual void AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name);
	virtual void RemoveKeyValuesFromMemoryLeakList(void *pMem);
};

void(__fastcall *g_pfnRegisterSizeofKeyValues)(void *pthis, int edx, int size) = NULL;
void *(__fastcall *g_pfnAllocKeyValuesMemory)(void *pthis, int edx, int size) = NULL;
void(__fastcall *g_pfnFreeKeyValuesMemory)(void *pthis, int edx, void *pMem) = NULL;
HKeySymbol(__fastcall *g_pfnGetSymbolForString)(void *pthis, int edx, const char *name) = NULL;
const char *(__fastcall *g_pfnGetStringForSymbol)(void *pthis, int edx, HKeySymbol symbol) = NULL;
HLocalized(__fastcall *g_pfnGetLocalizedFromANSI)(void *pthis, int, const char *string);
const char *(__fastcall *g_pfnGetANSIFromLocalized)(void *pthis, int, HLocalized l);
void(__fastcall *g_pfnAddKeyValuesToMemoryLeakList)(void *pthis, int edx, void *pMem, HKeySymbol name) = NULL;
void(_fastcall *g_pfnRemoveKeyValuesFromMemoryLeakList)(void *pthis, int edx, void *pMem) = NULL;

void CKeyValuesSystem::RegisterSizeofKeyValues(int size)
{
	return g_pfnRegisterSizeofKeyValues(this, 0, size);
}

void *CKeyValuesSystem::AllocKeyValuesMemory(int size)
{
	return ::operator new(size);
}
void CKeyValuesSystem::FreeKeyValuesMemory(void *pMem)
{
	return ::operator delete(pMem);
}

HKeySymbol CKeyValuesSystem::GetSymbolForString(const char *name)
{
	return g_pfnGetSymbolForString(this, 0, name);
}

const char *CKeyValuesSystem::GetStringForSymbol(HKeySymbol symbol)
{
	return g_pfnGetStringForSymbol(this, 0, symbol);
}

HLocalized CKeyValuesSystem::GetLocalizedFromANSI(const char *string)
{
	return g_pfnGetLocalizedFromANSI(this, 0, string);
}

const char *CKeyValuesSystem::GetANSIFromLocalized(HLocalized l)
{
	return g_pfnGetANSIFromLocalized(this, 0, l);
}

void CKeyValuesSystem::AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name)
{
	return g_pfnAddKeyValuesToMemoryLeakList(this, 0, pMem, name);
}

void CKeyValuesSystem::RemoveKeyValuesFromMemoryLeakList(void *pMem)
{
	return g_pfnRemoveKeyValuesFromMemoryLeakList(this, 0, pMem);
}

void KeyValuesSystem_InstallHook(IKeyValuesSystem *pKeyValuesSystem)
{
	g_pKeyValuesSystem = pKeyValuesSystem;
	
	CKeyValuesSystem KeyValuesSystem;
	DWORD *pVFTable = *(DWORD **)&KeyValuesSystem;

	/*g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 1, (void *)pVFTable[1], (void *&)g_pfnRegisterSizeofKeyValues);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 2, (void *)pVFTable[2], (void *&)g_pfnAllocKeyValuesMemory);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 3, (void *)pVFTable[3], (void *&)g_pfnFreeKeyValuesMemory);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 4, (void *)pVFTable[4], (void *&)g_pfnGetSymbolForString);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 5, (void *)pVFTable[5], (void *&)g_pfnGetStringForSymbol);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 6, (void *)pVFTable[6], (void *&)g_pfnGetLocalizedFromANSI);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 7, (void *)pVFTable[7], (void *&)g_pfnGetANSIFromLocalized);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 8, (void *)pVFTable[8], (void *&)g_pfnAddKeyValuesToMemoryLeakList);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 9, (void *)pVFTable[9], (void *&)g_pfnRemoveKeyValuesFromMemoryLeakList);*/
}