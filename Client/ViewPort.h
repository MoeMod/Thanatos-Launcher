#pragma once

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <vgui/VGUI2.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <VGUI_controls/Controls.h>
#include <VGUI_controls/Panel.h>
#include <VGUI_controls/Frame.h>

#include "EngineInterface.h"
#include "BaseViewport.h"
#include "ViewPortPanel.h"

#define COLOR_RED Color(255, 64, 64, 255)
#define COLOR_BLUE Color(153, 204, 255, 255)
#define COLOR_YELLOW Color(255, 178, 0, 255)
#define COLOR_GREEN Color(153, 255, 153, 255)
#define COLOR_GREY Color(204, 204, 204, 255)

#define MAX_SERVERNAME_LENGTH 64

#define PANEL_ALL "all"
#define PANEL_ACTIVE "active"

class CHudLayer;

class CCSBackGroundPanel;
class CCSClassMenu_TER;
class CCSClassMenu_CT;
class CCSTextWindow;
class CCSClientScoreBoardDialog;
class CCSTeamMenu;
class CommandMenu;
class CCSSpectatorGUI;
class CCSSpectatorMenu;
class CCSBuyMenu_CT;
class CCSBuyMenu_TER;
class CNavProgress;
class CCSChatDialog;

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

class CViewport : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CViewport, vgui2::Panel);

public:
	CViewport(void);
	~CViewport(void);

public:
	void Start(void);
	void SetParent(vgui2::VPANEL parent);
	void SetVisible(bool state);

public:
	virtual int GetViewPortScheme(void);
	virtual vgui2::VPANEL GetViewPortPanel(void);
	virtual void CreateBackGround(void);
	virtual void ShowBackGround(bool bShow);

public:
	void Init(void);
	void VidInit(void);
	void Reset(void);
	void Think(void);

public:
	bool IsInLevel(void);
	bool IsInMultiplayer(void);
	bool IsVIPMap(void);
	bool IsBombDefuseMap(void);
	float GetCurrentTime(void);
	char *GetServerName(void);

public:
	void HideAllVGUIMenu(void);
	bool ShowVGUIMenu(int iMenu);
	bool HideVGUIMenu(int iMenu);
	void ActivateClientUI(void);
	void HideClientUI(void);
	void ShowSpectatorGUI(void);
	void ShowSpectatorGUIBar(void);
	void HideSpectatorGUI(void);
	void UpdateSpectatorPanel(void);
	void UpdateSpectatorGUI(void);
	void UpdateSpectatorGUIBar(void);
	void DeactivateSpectatorGUI(void);
	bool IsSpectatorGUIVisible(void);
	bool IsSpectatorBarVisible(void);
	void SetSpectatorBanner(const char *image);
	int GetSpectatorBottomBarHeight(void);
	int GetSpectatorTopBarHeight(void);
	bool IsScoreBoardVisible(void);
	void ShowScoreBoard(void);
	void HideScoreBoard(void);
	void HideVGUIMenu(void);
	CViewPortPanel *AddNewPanel(CViewPortPanel *pPanel, char const *pchDebugName = NULL);
	CViewPortPanel *FindPanelByName(const char *szPanelName);
	void PostMessageToPanel(CViewPortPanel *pPanel, KeyValues *pKeyValues);
	void PostMessageToPanel(const char *pName, KeyValues *pKeyValues);
	CViewPortPanel *GetActivePanel(void);
	void ShowPanel(const char *pName, bool state);
	void ShowPanel(CViewPortPanel *pPanel, bool state);
	void UpdateAllPanels(void);
	void RemoveAllPanels(void);
	void StartMessageMode(void);
	void StartMessageMode2(void);
	bool SelectMenuItem(int menu_item);
	void UpdateMapSprite(void);
	void RenderMapSprite(void);
	void CalcRefdef(struct ref_params_s *pparams);
	void HideVoiceLabels(void);
	int FireMessage(const char *pszName, int iSize, void *pbuf);
	void ShowCommandMenu(void);
	void UpdateCommandMenu(void);
	void HideCommandMenu(void);
	int IsCommandMenuVisible(void);
	Color GetTeamColor(int index);
	char *GetTeamName(int index);
	int GetTeamScore(int index);
	void StartProgressBar(const char *title, int numTicks, int startTicks, bool isTimeBased);
	void UpdateProgressBar(const char *statusText, int tick);
	void StopProgressBar(void);
	void PrintText(int msg_dest, const char *msg_name, const char *param1 = "", const char *param2 = "", const char *param3 = "", const char *param4 = "");
	bool AllowedToPrintText(void);

private:
	Color m_TeamColors[4];
	int m_iTeamScores[4];

private:
	float m_flCurrentTime;
	bool m_bInitialied;
	bool m_bShowBackGround;
	CUtlVector<CViewPortPanel *> m_Panels;
	CViewPortPanel *m_pActivePanel;
	CViewPortPanel *m_pLastActivePanel;
	CCSBackGroundPanel *m_pBackGround;
	CViewPortPanel *m_pPanelShow;
	CHudLayer *m_pHudLayer;
	char m_szServerName[MAX_SERVERNAME_LENGTH];

private:
	CommandMenu *m_pCommandMenu;
	CNavProgress *m_pNavProgress;
	CCSTeamMenu *m_pTeamMenu;
	CCSClassMenu_TER *m_pClassMenu_TER;
	CCSClassMenu_CT *m_pClassMenu_CT;
	CCSTextWindow *m_pTextWindow;
	CCSClientScoreBoardDialog *m_pScoreBoard;
	CCSSpectatorGUI *m_pSpectatorGUI;
	CCSSpectatorMenu *m_pSpectatorMenu;
	CCSBuyMenu_CT *m_pBuyMenu_CT;
	CCSBuyMenu_TER *m_pBuyMenu_TER;
	CCSChatDialog *m_pChatDialog;
};

extern CViewport *g_pViewPort;