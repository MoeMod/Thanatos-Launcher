#ifndef ISTEAMAPPS_H
#define ISTEAMAPPS_H
#ifdef _WIN32
#pragma once
#endif

class ISteamApps
{
public:
	virtual bool BIsSubscribed(void) = 0;
	virtual bool BIsLowViolence(void) = 0;
	virtual bool BIsCybercafe(void) = 0;
	virtual bool BIsVACBanned(void) = 0;
	virtual const char *GetCurrentGameLanguage(void) = 0;
	virtual const char *GetAvailableGameLanguages(void) = 0;
};

#endif