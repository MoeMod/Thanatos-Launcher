#include "hud.h"
#include "hud_util.h"
#include "cstrikeclientscoreboard.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>
#include <cl_entity.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/SectionedListPanel.h>

#include "CounterStrikeViewport.h"
#include "client.h"

using namespace vgui2;

CCSClientScoreBoardDialog::CCSClientScoreBoardDialog(void) : CClientScoreBoardDialog()
{
	m_pPlayerListT = new SectionedListPanel(this, "TPlayerList");
	m_pPlayerListCT = new SectionedListPanel(this, "CTPlayerList");

	m_pPlayerCountLabel_T = new Label(this, "TPlayersAlive", "");
	m_pScoreLabel_T = new Label(this, "TTeamScore", "");
	m_pPingLabel_T = new Label(this, "TPlayerLatencyLabel", "");
	m_pPlayerCountLabel_CT = new Label(this, "CTPlayersAlive", "");
	m_pScoreLabel_CT = new Label(this, "CTTeamScore", "");
	m_pPingLabel_CT = new Label(this, "CTPlayerLatencyLabel", "");

	SetVisible(false);
	SetPaintBackgroundEnabled(true);

	LoadControlSettings("Resource/UI/ScoreBoard.res", "GAME");

	m_iAvatarWidth = scheme()->GetProportionalScaledValue(8);
	m_iNameWidth = scheme()->GetProportionalScaledValue(118);
	m_iClassWidth = scheme()->GetProportionalScaledValue(60);
	m_iScoreWidth = scheme()->GetProportionalScaledValue(32);
	m_iDeathWidth = scheme()->GetProportionalScaledValue(32);
	m_iPingWidth = scheme()->GetProportionalScaledValue(40);
	m_iListWidth = m_iAvatarWidth + m_iNameWidth + m_iClassWidth + m_iScoreWidth + m_iDeathWidth + m_iPingWidth + scheme()->GetProportionalScaledValue(2);

	SetServerName("Thanatos Zone");
}

CCSClientScoreBoardDialog::~CCSClientScoreBoardDialog(void)
{
}

void CCSClientScoreBoardDialog::SetServerName(const char *hostname)
{
	Panel *control = FindChildByName("ServerNameLabel");

	if (control)
	{
		wchar_t wzServerLabel[64], wzServerName[64];
		localize()->ConvertANSIToUnicode(hostname, wzServerName, sizeof(wzServerName));
		localize()->ConstructString(wzServerLabel, sizeof(wzServerLabel), localize()->Find("#Cstrike_SB_Server"), 1, wzServerName);
		PostMessage(control, new KeyValues("SetText", "text", wzServerLabel));
		control->MoveToFront();
	}

	if (IsVisible())
		Update();
}

void CCSClientScoreBoardDialog::SetMapName(const char *mapname)
{
	SetDialogVariable("mapname", mapname);

	if (IsVisible())
		Update();
}

void CCSClientScoreBoardDialog::UpdatePlayerClass(int playerIndex, KeyValues *kv)
{
	const char *hltv = engine->PlayerInfo_ValueForKey(playerIndex, "*hltv");

	if (hltv && atoi(hltv) > 0)
	{
		char numspecs[32];
		Q_snprintf(numspecs, sizeof(numspecs), "%i Spectators", m_HLTVSpectators);
		kv->SetString("class", numspecs);
	}
	else if (g_PlayerExtraInfo[playerIndex].teamnumber != TEAM_SPECTATOR && g_PlayerExtraInfo[playerIndex].teamnumber != TEAM_UNASSIGNED)
	{
		if (g_PlayerExtraInfo[playerIndex].dead)
		{
			kv->SetString("class", "#Cstrike_DEAD");
		}
		else if (g_PlayerScoreAttrib[playerIndex] & SCOREATTRIB_ZOMBIE)
		{
			kv->SetString("class", "#Cstrike_ZOMBIE");
		}
	}
	else if (g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamnumber == TEAM_UNASSIGNED || g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamnumber == TEAM_SPECTATOR || g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead || g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamnumber == g_PlayerExtraInfo[playerIndex].teamnumber)
	{
		if (g_PlayerExtraInfo[playerIndex].has_c4)
		{
			kv->SetString("class", "#Cstrike_BOMB");
		}
		else if (g_PlayerExtraInfo[playerIndex].vip)
		{
			kv->SetString("class", "#Cstrike_VIP");
		}
	}
}

void CCSClientScoreBoardDialog::ShowPanel(bool bShow)
{
	if (gHUD.m_iIntermission && !bShow)
		return;

	BaseClass::ShowPanel(bShow);
}

void CCSClientScoreBoardDialog::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSClientScoreBoardDialog::PaintBorder(void)
{
	BaseClass::PaintBorder();
}

void CCSClientScoreBoardDialog::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetProportional(false);
	SetVisible(false);
	SetBgColor(Color(0, 0, 0, 192));

	if (m_pPlayerListT)
	{
		Panel *control = FindChildByName("TPlayerLabel");

		//if (control && control->IsVisible())
		//	m_pPlayerListT->SetPaintOffset(5, control->GetTall() * 1.05);

		m_pPlayerListT->SetImageList(m_pImageList, false);
		m_pPlayerListT->SetBgColor(Color(0, 0, 0, 0));
		m_pPlayerListT->SetBorder(NULL);
	}

	if (m_pPlayerListCT)
	{
		Panel *control = FindChildByName("CTPlayerLabel");

		//if (control && control->IsVisible())
		//	m_pPlayerListCT->SetPaintOffset(5, control->GetTall() * 1.05);

		m_pPlayerListCT->SetImageList(m_pImageList, false);
		m_pPlayerListCT->SetBgColor(Color(0, 0, 0, 0));
		m_pPlayerListCT->SetBorder(NULL);
	}

	if (m_pPlayerList)
		m_pPlayerList->SetVisible(false);

	if (m_pPlayerCountLabel_T && m_pScoreLabel_T && m_pPingLabel_T)
	{
		m_pPlayerCountLabel_T->SetFgColor(COLOR_RED);
		m_pScoreLabel_T->SetFgColor(COLOR_RED);
	}

	if (m_pPlayerCountLabel_CT && m_pScoreLabel_CT && m_pPingLabel_CT)
	{
		m_pPlayerCountLabel_CT->SetFgColor(COLOR_BLUE);
		m_pScoreLabel_CT->SetFgColor(COLOR_BLUE);
	}
}

void CCSClientScoreBoardDialog::Init(void)
{
	InitPlayerList(m_pPlayerListT, TEAM_TERRORIST);
	InitPlayerList(m_pPlayerListCT, TEAM_CT);
}

void CCSClientScoreBoardDialog::VidInit(void)
{
	m_flTimeLeft = 0;
}

void CCSClientScoreBoardDialog::Reset(void)
{
	const char *mapname = gEngfuncs.pfnGetLevelName();

	if (strlen(mapname) > 5)
	{
		char newname[32];
		strcpy(newname, &mapname[5]);
		newname[strlen(newname) - 4] = 0;
		SetMapName(newname);
	}
	else
		SetMapName(mapname);
}

bool CCSClientScoreBoardDialog::CSPlayerSortFunc(vgui2::SectionedListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);

	Assert(it1 && it2);

	int v1 = it1->GetInt("frags");
	int v2 = it2->GetInt("frags");

	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	v1 = it1->GetInt("deaths");
	v2 = it2->GetInt("deaths");

	if (v1 > v2)
		return false;
	else if (v1 < v2)
		return true;

	int iPlayerIndex1 = it1->GetInt("playerIndex");
	int iPlayerIndex2 = it2->GetInt("playerIndex");

	return (iPlayerIndex1 > iPlayerIndex2);
}

void CCSClientScoreBoardDialog::InitPlayerList(SectionedListPanel *pPlayerList, int teamNumber)
{
	pPlayerList->SetVerticalScrollbar(false);
	pPlayerList->RemoveAll();
	pPlayerList->RemoveAllSections();
	pPlayerList->AddSection(0, "Players", CSPlayerSortFunc);
	pPlayerList->SetSectionAlwaysVisible(0, true);
	pPlayerList->SetSectionFgColor(0, Color(255, 255, 255, 255));
	pPlayerList->SetBgColor(Color(0, 0, 0, 0));
	pPlayerList->SetBorder(NULL);

	if (teamNumber)
	{
		pPlayerList->SetSectionFgColor(0, g_pViewPort->GetTeamColor(teamNumber));
		//pPlayerList->SetSectionDividerColor(0, Color(0, 0, 0, 0));
	}

	//if (pPlayerList->IsSectionHeaderVisible())
	if (true)
	{
		float fullWidth = m_iListWidth;
		int listWidth = pPlayerList->GetWide();

		pPlayerList->AddColumnToSection(0, "avatar", "", SectionedListPanel::COLUMN_CENTER, listWidth * (m_iAvatarWidth / fullWidth));
		pPlayerList->AddColumnToSection(0, "name", "#Cstrike_TitlesTXT_PLAYERS", 0, listWidth * (m_iNameWidth / fullWidth));
		pPlayerList->AddColumnToSection(0, "class", "", SectionedListPanel::COLUMN_CENTER, listWidth * (m_iClassWidth / fullWidth));
		pPlayerList->AddColumnToSection(0, "frags", "#Cstrike_TitlesTXT_SCORE", SectionedListPanel::COLUMN_CENTER, listWidth * (m_iScoreWidth / fullWidth));
		pPlayerList->AddColumnToSection(0, "deaths", "#Cstrike_TitlesTXT_DEATHS", SectionedListPanel::COLUMN_CENTER, listWidth * (m_iDeathWidth / fullWidth));
		pPlayerList->AddColumnToSection(0, "ping", "#Cstrike_TitlesTXT_LATENCY", SectionedListPanel::COLUMN_CENTER, listWidth * (m_iPingWidth / fullWidth));
	}
	else
	{
		pPlayerList->AddColumnToSection(0, "avatar", "", SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth);
		pPlayerList->AddColumnToSection(0, "name", "#Cstrike_TitlesTXT_PLAYERS", 0, m_iNameWidth);
		pPlayerList->AddColumnToSection(0, "class", "", SectionedListPanel::COLUMN_CENTER, m_iClassWidth);
		pPlayerList->AddColumnToSection(0, "frags", "#Cstrike_TitlesTXT_SCORE", SectionedListPanel::COLUMN_CENTER, m_iScoreWidth);
		pPlayerList->AddColumnToSection(0, "deaths", "#Cstrike_TitlesTXT_DEATHS", SectionedListPanel::COLUMN_CENTER, m_iDeathWidth);
		pPlayerList->AddColumnToSection(0, "ping", "#Cstrike_TitlesTXT_LATENCY", SectionedListPanel::COLUMN_CENTER, m_iPingWidth);
	}
}

void CCSClientScoreBoardDialog::UpdateRoundTimer(void)
{
	Panel *control = FindChildByName("Label_Clock");

	if (control)
	{
		int timer = (int)(m_flTimeLeft - gHUD.m_flTime);

		char szText[32];
		sprintf(szText, "%d:%02d\n", (timer / 60), (timer % 60));

		wchar_t wzClockLabel[64];
		wchar_t wzClockTimer[32];
		localize()->ConvertANSIToUnicode(szText, wzClockTimer, sizeof(wzClockTimer));
		localize()->ConstructString(wzClockLabel, sizeof(wzClockLabel), localize()->Find("#Cstrike_Time_LeftVariable"), 1, wzClockTimer);
		PostMessage(control, new KeyValues("SetText", "text", wzClockLabel));
		control->MoveToFront();

		if (timer > 0)
			control->SetVisible(true);
		else
			control->SetVisible(false);

		control = FindChildByName("Icon_Clock");

		if (control)
		{
			if (timer > 0)
				control->SetVisible(true);
			else
				control->SetVisible(false);
		}
	}
}

void CCSClientScoreBoardDialog::Update(void)
{
	UpdateTeamInfo();
	UpdatePlayerList();
	UpdateSpectatorList();
	UpdateRoundTimer();

	if (NeedsUpdate())
		m_fNextUpdateTime = g_pViewPort->GetCurrentTime() + 1.0f;
}

void CCSClientScoreBoardDialog::UpdateTeamInfo(void)
{
	for (int teamIndex = TEAM_TERRORIST; teamIndex <= TEAM_CT; teamIndex++)
	{
		wchar_t *teamName = NULL;
		int teamCounts = GetTeamCounts(teamIndex);
		int teamAliveCounts = GetTeamAliveCounts(teamIndex);

		SectionedListPanel *pTeamList = NULL;
		Label *pTeamLabel = NULL;
		Label *pPlayerCountLabel = NULL;
		Label *pScoreLabel = NULL;
		Label *pPingLabel = NULL;

		wchar_t name[64];
		wchar_t string1[1024];

		if (!teamName)
		{
			localize()->ConvertANSIToUnicode(g_pViewPort->GetTeamName(teamIndex), name, sizeof(name));
			teamName = name;
		}

		switch (teamIndex)
		{
			case TEAM_TERRORIST:
			{
				teamName = localize()->Find("#Cstrike_ScoreBoard_Ter");

				pTeamList = m_pPlayerListT;
				pTeamLabel = (Label *)FindChildByName("T_Label");
				pPlayerCountLabel = m_pPlayerCountLabel_T;
				pScoreLabel = m_pScoreLabel_T;
				pPingLabel = m_pPingLabel_T;

				_swprintf(string1, L"%d/%d", teamAliveCounts, teamCounts);
				break;
			}

			case TEAM_CT:
			{
				teamName = localize()->Find("#Cstrike_ScoreBoard_CT");

				pTeamList = m_pPlayerListCT;
				pTeamLabel = (Label *)FindChildByName("CT_Label");
				pPlayerCountLabel = m_pPlayerCountLabel_CT;
				pScoreLabel = m_pScoreLabel_CT;
				pPingLabel = m_pPingLabel_CT;

				_swprintf(string1, L"%d/%d", teamAliveCounts, teamCounts);
				break;
			}
		}

		pTeamLabel->SetText(teamName);
		pPlayerCountLabel->SetText(string1);

		int pingsum = 0;
		int numcounted = 0;

		for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
		{
			if (IsConnected(playerIndex) && g_PlayerExtraInfo[playerIndex].teamnumber == teamIndex)
			{
				int ping = g_PlayerInfoList[playerIndex].ping;

				if (ping >= 1)
				{
					pingsum += ping;
					numcounted++;
				}
			}
		}

		if (numcounted > 0)
		{
			int ping = (int)((float)pingsum / (float)numcounted);

			_swprintf(string1, L"%d", ping);
			pPingLabel->SetText(string1);
		}
		else
		{
			pPingLabel->SetText("");
		}

		if (pScoreLabel->IsVisible())
		{
			_swprintf(string1, L"%d", g_pViewPort->GetTeamScore(teamIndex));
			pScoreLabel->SetText(string1);
		}
		else
		{
#if 0
			_swprintf(string1, L"%s (%d)", localize()->Find("#Cstrike_SB_Score"), g_pViewPort->GetTeamScore(teamIndex));
			pTeamList->ModifyColumn(0, "frags", string1);
#endif
		}
	}
}

void CCSClientScoreBoardDialog::UpdatePlayerList(void)
{
	m_pPlayerListT->RemoveAll();
	m_pPlayerListCT->RemoveAll();

	for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
	{
		if (IsConnected(playerIndex))
		{
			SectionedListPanel *pPlayerList = NULL;

			switch (g_PlayerExtraInfo[playerIndex].teamnumber)
			{
				case TEAM_TERRORIST:
				{
					pPlayerList = m_pPlayerListT;
					break;
				}

				case TEAM_CT:
				{
					pPlayerList = m_pPlayerListCT;
					break;
				}
			}

			if (pPlayerList == NULL)
			{
				continue;
			}

			KeyValues *pKeyValues = new KeyValues("data");
			GetPlayerScoreInfo(playerIndex, pKeyValues);

			int itemID = pPlayerList->AddItem(0, pKeyValues);
			Color clr = g_pViewPort->GetTeamColor(g_PlayerExtraInfo[playerIndex].teamnumber);
			pPlayerList->SetItemFgColor(itemID, clr);

			if (playerIndex == gHUD.m_iPlayerNum)
				pPlayerList->SetSelectedItem(itemID);

			pKeyValues->deleteThis();
		}
	}
}

void CCSClientScoreBoardDialog::UpdateSpectatorList(void)
{
	char szSpectatorList[512] = "";
	int nSpectators = 0;

	for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
	{
		if (ShouldShowAsSpectator(playerIndex))
		{
			if (nSpectators > 0)
				Q_strncat(szSpectatorList, ", ", ARRAYSIZE(szSpectatorList));

			Q_strncat(szSpectatorList, g_PlayerInfoList[playerIndex].name, ARRAYSIZE(szSpectatorList));
			nSpectators++;
		}
	}

	wchar_t wzSpectators[512] = L"";

	if (nSpectators > 0)
	{
		const char *pchFormat = (1 == nSpectators ? "#Cstrike_ScoreBoard_Spectator" : "#Cstrike_ScoreBoard_Spectators");

		wchar_t wzSpectatorCount[16];
		wchar_t wzSpectatorList[1024];
		_snwprintf(wzSpectatorCount, ARRAYSIZE(wzSpectatorCount), L"%i", nSpectators);
		localize()->ConvertANSIToUnicode(szSpectatorList, wzSpectatorList, sizeof(wzSpectatorList));
		localize()->ConstructString(wzSpectators, sizeof(wzSpectators), localize()->Find(pchFormat), 2, wzSpectatorCount, wzSpectatorList);

		SetDialogVariable("spectators", wzSpectators);
	}
	else
		SetDialogVariable("spectators", localize()->Find("#Cstrike_Scoreboard_NoSpectators"));
}

bool CCSClientScoreBoardDialog::ShouldShowAsSpectator(int iPlayerIndex)
{
	int iTeam = g_PlayerExtraInfo[iPlayerIndex].teamnumber;

	if (IsConnected(iPlayerIndex))
	{
		if (iTeam == TEAM_SPECTATOR || iTeam == TEAM_UNASSIGNED)
			return true;
	}

	return false;
}

bool CCSClientScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	hud_player_info_t info;
	engine->pfnGetPlayerInfo(playerIndex, &info);

	if (info.name == NULL || info.name[0] == '\0')
		return false;

	const char *oldName = info.name;
	int bufsize = strlen(oldName) * 2 + 1;
	char *newName = (char *)_alloca(bufsize);
	gViewPortInterface->MakeSafeName(oldName, newName, bufsize);

	if (info.ping <= 0)
	{
		const char *bot = engine->PlayerInfo_ValueForKey(playerIndex, "*bot");

		if (bot && atoi(bot) > 0)
			kv->SetString("ping", "BOT");
		else
			kv->SetString("ping", "HOST");
	}
	else
		kv->SetInt("ping", info.ping);

	kv->SetString("name", info.name);
	kv->SetString("class", "");
	kv->SetInt("deaths", g_PlayerExtraInfo[playerIndex].deaths);
	kv->SetInt("frags", g_PlayerExtraInfo[playerIndex].frags);

	UpdatePlayerAvatar(playerIndex, kv);
	UpdatePlayerClass(playerIndex, kv);
	return true;
}

void CCSClientScoreBoardDialog::MsgFunc_TimeLeft(void)
{
	m_flTimeLeft = gHUD.m_flTime + READ_WORD();
}