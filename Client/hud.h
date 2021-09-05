#ifndef HUD_H
#define HUD_H

#include <metahook.h>
#include "msghook.h"
#include "util_vector.h"
//#include "vgui.h"
#include "links.h"
#include "ViewPort.h"
#include "VGUI/IImage.h"

#include <vector>

#define RGB_YELLOWISH 0x00FFA000 // 255, 160, 0
#define RGB_REDISH 0x00FF1010 // 255, 16, 16
#define RGB_GREENISH 0x0000A000 // 0, 160, 0

#define DHN_DRAWZERO 1
#define DHN_2DIGITS 2
#define DHN_3DIGITS 4
#define DHN_FILLZERO 8
#define DHN_4DIGITS 16
#define DHN_5DIGITS 32
#define MIN_ALPHA 100

#define MAX_SPRITE_NAME_LENGTH 24

typedef struct
{
	int x, y;
}
POSITION;

enum
{
	MAX_PLAYERS = 32,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

#ifndef MAX_CLIENTS
#define MAX_CLIENTS 32
#endif

typedef struct
{
	unsigned char r, g, b, a;
}
RGBA;

#define HUD_ACTIVE 1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH 32

class CHudBase
{
public:
	POSITION m_pos;
	int m_type;
	int m_iFlags;

public:
	virtual ~CHudBase(void) {}
	virtual int Init(void) { return 0; }
	virtual int VidInit(void) { return 0; }
	virtual int Draw(float flTime) { return 0; }
	virtual void Think(void) { return; }
	virtual void Reset(void) { return; }
	virtual void InitHUDData(void) {}
};

#include "cvardef.h"
#include "hud_spectator.h"
#include "ammo.h"
#include "player.h"

#define FADE_TIME 100

class CHudAmmo : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	int DrawWList(float flTime);

	void CalculateCrosshairColor(void);
	void CalculateCrosshairDrawMode(void);
	void CalculateCrosshairSize(void);

	int DrawCrosshair(float flTime, int weaponid);
	int DrawCrosshairEx(float flTime, int weaponid, int iBarSize, float flCrosshairDistance, bool bAdditive, int r, int g, int b, int a);

	int MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_CurWeapon2(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX2(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeapPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeapPickup2(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ItemPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Crosshair(const char *pszName, int iSize, void *pbuf);

	void SlotInput(int iSlot);

	void _cdecl UserCmd_Slot1(void);
	void _cdecl UserCmd_Slot2(void);
	void _cdecl UserCmd_Slot3(void);
	void _cdecl UserCmd_Slot4(void);
	void _cdecl UserCmd_Slot5(void);
	void _cdecl UserCmd_Slot6(void);
	void _cdecl UserCmd_Slot7(void);
	void _cdecl UserCmd_Slot8(void);
	void _cdecl UserCmd_Slot9(void);
	void _cdecl UserCmd_Slot10(void);
	void _cdecl UserCmd_Close(void);
	void _cdecl UserCmd_NextWeapon(void);
	void _cdecl UserCmd_PrevWeapon(void);
	void _cdecl UserCmd_Adjust_Crosshair(void);

public:
	float m_fFade;
	RGBA m_rgba;
	WEAPON *m_pWeapon;
	int m_HUD_bucket0;
	int m_HUD_selection;
	HSPRITE m_hObserverCrosshair;
	wrect_t m_rcObserverCrosshair;
	int m_bObserverCrosshair;
	int m_iAmmoLastCheck;
	float m_flCrosshairDistance;
	int m_iAlpha;
	int m_R;
	int m_G;
	int m_B;
	int m_cvarR;
	int m_cvarG;
	int m_cvarB;
	int m_iCurrentCrosshair;
	bool m_bAdditive;
	int m_iCrosshairScaleBase;
	char m_szLastCrosshairColor[32];
	float m_flLastCalcTime;
};

#include "health.h"

struct extra_player_info_t
{
	short frags;
	short deaths;
	short team_id;
	BOOL has_c4;
	BOOL vip;
	Vector origin;
	float radarflash;
	int radarflashon;
	int radarflashes;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
	bool dead;
	float showhealth;
	int health;
	char location[32];
};

struct team_info_t
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

#define MAX_HOSTAGES 24

struct hostage_info_t
{
	bool dead;
	Vector origin;
	int health;
	float radarflash;
	int radarflashon;
	int radarflashes;
	char teamname[MAX_TEAM_NAME];
};

extern hud_player_info_t g_PlayerInfoList[];
extern extra_player_info_t *g_PlayerExtraInfo;
extern hostage_info_t *g_HostageInfo;
extern int g_PlayerScoreAttrib[];
extern struct tempent_s *g_DeadPlayerModels[];

class CHudDeathNotice : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	void Reset(void);
	int VidInit(void);
	int Draw(float flTime);
	int GetDeathNoticeY(void);

public:
	int MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf);

private:
	int m_HUD_d_skull;
	int m_headSprite, m_headWidth;
	vgui2::IImage *m_killBg[3], *m_deathBg[3];
	bool m_showKill;
	float m_killEffectTime, m_killIconTime;
	int m_iDrawBgWidth, m_iDrawBgHeight;
	int m_iFontHeight;

private:
	HSPRITE m_KillerMsgSprite;
};

#include "nightvision.h"

class CHudMenu : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);

public:
	int MsgFunc_ShowMenu(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf);

	void _cdecl UserCmd_SizeDown(void);
	void _cdecl UserCmd_SizeUp(void);

public:
	bool SelectMenuItem(int menu_item);

public:
	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
};

class CHudSayText : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);

public:
	int MsgFunc_SayText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_SendAudio(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_SendRadio(const char *pszName, int iSize, void *pbuf);

public:
	void _cdecl UserCmd_MessageMode(void);
	void _cdecl UserCmd_MessageMode2(void);

public:
	void SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex = -1, char *sstr1 = NULL, char *sstr2 = NULL, char *sstr3 = NULL, char *sstr4 = NULL);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);

public:
	friend class CHudSpectator;

private:
	struct cvar_s *m_HUD_saytext;
	struct cvar_s *m_HUD_saytext_time;
};

class CHudBattery : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);

	int MsgFunc_Battery(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ArmorType(const char *pszName, int iSize, void *pbuf);

public:
	int m_HUD_suit_empty, m_HUD_suit_full;
	int m_HUD_suithelmet_empty, m_HUD_suithelmet_full;
	int m_iBat;
	float m_fFade;
	int m_iArmorType;
};

const int maxHUDMessages = 16;

struct message_parms_t
{
	client_textmessage_t *pMessage;
	float time;
	int x, y;
	int totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

class CHudTextMessage : public CHudBase
{
public:
	static char *LocaliseTextString(const char *msg, char *dst_buffer, int buffer_size);
	static char *BufferedLocaliseTextString(const char *msg);

public:
	int Init(void);
	char *LookupString(const char *msg_name, int *msg_dest = NULL);

public:
	int MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);
};

const int MESSAGE_ARG_LEN = 64;
const int MAX_MESSAGE_ARGS = 4;

typedef struct
{
	client_textmessage_t *pMessage;
	unsigned int font;
	wchar_t args[MAX_MESSAGE_ARGS][MESSAGE_ARG_LEN];
	int numArgs;
	int hintMessage;
}
client_message_t;

class CHudMessage : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);

public:
	int MsgFunc_HudText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HudTextPro(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HudTextArgs(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf);

public:
	float FadeBlend(float fadein, float fadeout, float hold, float localTime);
	int XPosition(float x, int width, int lineWidth);
	int YPosition(float y, int height);

public:
	void MessageAdd(const char *pName, int hintMessage, unsigned int font, float time);
	void MessageAdd(client_textmessage_t *newMessage);
	int MessageAdd(const char *pName, float time, int hintMessage, unsigned int font);
	void MessageScanNextChar(unsigned int font);
	void MessageScanStart(void);
	void MessageDrawScan(client_message_t *pClientMessage, float time, unsigned int font);

private:
	client_message_t m_pMessages[maxHUDMessages];
	float m_startTime[maxHUDMessages];
	message_parms_t m_parms;
	float m_gameTitleTime;
	client_textmessage_t *m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
};

#define MAX_SPRITE_NAME_LENGTH 24

class CHudStatusIcons : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);

public:
	int MsgFunc_StatusIcon(const char *pszName, int iSize, void *pbuf);

	enum
	{
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};

	void EnableIcon(char *pszIconName, unsigned char red, unsigned char green, unsigned char blue, bool bFlash);
	void DisableIcon(char *pszIconName);

private:
	int m_iCrossWidth;
	int m_bFlashOn;
	float m_tmNextFlash;

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
		int bFlash;
	}
	icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];
};

class CHudAccountBalance : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);

public:
	int MsgFunc_Money(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_BlinkAcct(const char *pszName, int iSize, void *pbuf);

public:
	int m_iAccount;
	int m_HUD_dollar;
	int m_HUD_minus;
	int m_HUD_plus;
	int m_iAccountDelta;
	float m_fFade;
	float m_fFadeFast;
	bool m_bShowDelta;
	int m_iBlinkCount;
	float m_fBlinkTime;
};

class CHudItemStatus : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);

public:
	int MsgFunc_ItemStatus(const char *pszName, int iSize, void *pbuf);

public:
	bool m_hasDefuser;
	bool m_hasNightvision;
};

class CHudScenarioStatus : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float fTime);

public:
	int MsgFunc_Scenario(const char *pszName, int iSize, void *pbuf);

public:
	HSPRITE m_hSprite;
	wrect_t m_rect;
	int m_alpha;
	float m_nextFlash;
	float m_flashInterval;
	int m_flashAlpha;
};

class CHudRoundTimer : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);

public:
	int MsgFunc_RoundTime(const char *pszName, int iSize, void *pbuf);

public:
	float m_flTimeEnd;
	int m_HUD_stopwatch;
	float m_fFade;
	float m_flNewPeriod;
	float m_flNextToggle;
	bool m_bPanicColor;
	int m_closestRight;
};

class CHudProgressBar : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);

public:
	int MsgFunc_BarTime(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_BarTime2(const char *pszName, int iSize, void *pbuf);

public:
	float m_tmEnd;
	float m_tmStart;
	float m_tmNewPeriod;
	float m_tmNewElapsed;
	int m_width;
	int m_height;
	int m_x0;
	int m_y0;
	int m_x1;
	int m_y1;
};

#include "headname.h"
#include "radar.h"
#include "thirdperson.h"
#include "graphicmenu.h"
#include "viewmodel.h"

class CHud
{
public:
	CHud(void) : m_iSpriteCount(0) {}
	~CHud(void);

public:
	void Init(void);
	void VidInit(void);
	int Redraw(float flTime, int intermission);
	void Think(void);
	int UpdateClientData(client_data_t *cdata, float time);
	void CalcRefdef(struct ref_params_s *pparams);
	int GetSpriteIndex(const char *SpriteName);
	int DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b);
	int DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b);
	int DrawHudStringReverse(int xpos, int ypos, int iMinX, char *szString, int r, int g, int b);
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int DrawHudNumber(int x, int y, int iNumber, int r, int g, int b);
	int GetNumWidth(int iNumber, int iFlags);
	int GetNumBits(int iNumber);
	void AddHudElem(CHudBase *phudelem);
	float GetSensitivity(void);

	HSPRITE GetSprite(int index)
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}

	wrect_t &GetSpriteRect(int index)
	{
		return m_rgrcRects[index];
	}

public:
	int MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Fog(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_TimeLeft(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_SpecHealth(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_SpecHealth2(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ShadowIdx(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HudSetting(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HLTV(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_BombDrop(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Location(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ScoreAttrib(const char *pszName, int iSize, void *pbuf);

public:
	std::vector<CHudBase *> m_HudList;
	int m_iRes;
	float m_flTime;
	float m_fOldTime;
	double m_flTimeDelta;
	int m_iPlayerNum;
	client_sprite_t *m_pSpriteList;
	int m_iSpriteCount;
	int m_iSpriteCountAllRes;
	float m_flMouseSensitivity;
	HSPRITE *m_rghSprites;
	wrect_t *m_rgrcRects;
	char *m_rgszSpriteNames;
	int m_HUD_number_0;
	SCREENINFO m_scrinfo;
	int m_iHideHUDDisplay;
	int m_fPlayerDead;
	int m_iIntermission;
	int m_iFOV;
	int m_iFontHeight;
	int m_iFontEngineHeight;
	Vector m_vecOrigin;
	Vector m_vecAngles;
	int m_iKeyBits;
	int m_iWeaponBits;
	struct cvar_s *m_pCvarDraw;
	char m_szGameMode[32];
	int m_iHudSetting;
	int m_PlayerFOV[MAX_CLIENTS];
	bool m_bShowTimer;

private:
	struct cvar_s *default_fov;

public:
	CHudAmmo m_Ammo;
	CHudHealth m_Health;
	CHudDeathNotice m_DeathNotice;
	CHudSpectator m_Spectator;
	CHudNightVision m_NightVision;
	CHudSayText m_SayText;
	CHudTextMessage m_TextMessage;
	CHudMenu m_Menu;
	CHudRoundTimer m_roundTimer;
	CHudAccountBalance m_accountBalance;
	CHudBattery m_Battery;
	CHudItemStatus m_itemStatus;
	CHudHeadName m_headName;
	CHudRadar m_Radar;
	CHudThirdPerson m_ThirdPerson;
	CHudGraphicMenu m_GraphicMenu;
	CHudMessage m_Message;
	CHudStatusIcons m_StatusIcons;
	CHudScenarioStatus m_scenarioStatus;
	CHudProgressBar m_progressBar;
};

extern CHud gHUD;
extern cvar_t *hud_saytext_time;

extern struct cvar_s *cl_righthand;
extern struct cvar_s *cl_radartype;
extern struct cvar_s *cl_dynamiccrosshair;
extern struct cvar_s *cl_crosshair_color;
extern struct cvar_s *cl_crosshair_size;
extern struct cvar_s *cl_crosshair_translucent;
extern struct cvar_s *cl_crosshair_type;
extern struct cvar_s *cl_killeffect;
extern struct cvar_s *cl_killmessage;
extern struct cvar_s *cl_headname;
extern struct cvar_s *cl_newmenu;
extern struct cvar_s *cl_newmenu_drawbox;
extern struct cvar_s *cl_newradar;
extern struct cvar_s *cl_newradar_size;
extern struct cvar_s *cl_newradar_r;
extern struct cvar_s *cl_newradar_g;
extern struct cvar_s *cl_newradar_b;
extern struct cvar_s *cl_newradar_a;
extern struct cvar_s *cl_newchat;
extern struct cvar_s *cl_shadows;
extern struct cvar_s *cl_scoreboard;
extern struct cvar_s *cl_fog_skybox;
extern struct cvar_s *cl_fog_density;
extern struct cvar_s *cl_fog_r;
extern struct cvar_s *cl_fog_g;
extern struct cvar_s *cl_fog_b;
extern struct cvar_s *cl_minmodel;
extern struct cvar_s *cl_min_t;
extern struct cvar_s *cl_min_ct;
extern struct cvar_s *cl_corpsestay;
extern struct cvar_s *cl_corpsefade;

#endif