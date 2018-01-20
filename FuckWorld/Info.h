#define INFO_SETVALUEFORKEY_SIG "\x8B\x44\x24\x08\x80\x38\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\xC3"
#define INFO_SETVALUEFORKEY_SIG_NEW "\x55\x8B\xEC\x8B\x45\x0C\x80\x38\x2A\x75\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x5D\xC3"

extern void (*g_pfnInfo_SetValueForKey)(char *s, char *key, char *value, int maxsize);
extern struct hook_s *g_phInfo_SetValueForKey;

char *Info_ValueForKey(char *s, char *key);
void Info_RemoveKey(char *s, char *key);
void Info_RemovePrefixedKeys(char *start, char prefix);
qboolean Info_IsKeyImportant(char *key);
char *Info_FindLargestKey(char *s);
void Info_SetValueForStarKey(char *s, char *key, char *value, int maxsize);
void Info_Print(char *s);
void Info_SetValueForKey(char *s, char *key, char *value, int maxsize);
void Info_InstallHook();