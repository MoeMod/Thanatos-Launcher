#include "filesystem.h"
#include <windows.h>
#include <malloc.h>

wchar_t *UTF8ToUnicode(const char* str);

char *GetFileType(const char fileName[], char type[])
{
	int i = strlen(fileName) - 1, j;
	char ch;

	for (type[0] = '\0'; i >= 0; i--)
	{
		if (fileName[i] == '.')
		{
			for (j = i; fileName[j] != '\0'; j++)
			{
				ch = fileName[j];
				type[j - i] = ('A' <= ch && ch <= 'Z') ? (ch + 'a' - 'A') : ch;
			}

			type[j - i] = '\0';
			break;
		}
		else if (fileName[i] == '/' || fileName[i] == '\\')
		{
			break;
		}
	}

	return type;
}

bool IFileSystem::FileExists(const char *pFileName, const char *pPathID)
{
	return FileExists(pFileName);
}

bool IFileSystem::IsFileWritable(char const *pFileName, const char *pPathID)
{
	return false;
}

bool IFileSystem::SetFileWritable(char const *pFileName, bool writable, const char *pPathID)
{
	return false;
}

bool IFileSystem::IsDirectory(const char *pFileName, const char *pathID)
{
	return false;
}

bool IFileSystem::GetFileTypeForFullPath(char const *pFullPath, wchar_t *buf, size_t bufSizeInBytes)
{
	static char charBuf[32];
	GetFileType(pFullPath, charBuf);
	wcsncpy(buf, UTF8ToUnicode(charBuf), bufSizeInBytes);
	return false;
}