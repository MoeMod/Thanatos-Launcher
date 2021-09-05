#ifndef CSTRIKECHATDIALOG_H
#define CSTRIKECHATDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include "chatdialog.h"

class CCSChatDialog : public CChatDialog
{
	DECLARE_CLASS_SIMPLE(CCSChatDialog, CChatDialog);

public:
	CCSChatDialog(void);

public:
	virtual void CreateChatInputLine(void);
	virtual void CreateChatLines(void);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

public:
	virtual void OnThink(void);

public:
	void MsgFunc_SayText(void);
	void MsgFunc_TextMsg(void);

public:
	void Print(int msg_dest, const char *msg_name, const char *param1 = "", const char *param2 = "", const char *param3 = "", const char *param4 = "");
	int GetChatInputOffset(void);
	void SetVisible(bool state);

public:
	virtual Color GetTextColorForClient(TextColor colorNum, int clientIndex);
	virtual Color GetClientColor(int clientIndex);

public:
	int m_iSpectator;
	int m_iSaveX, m_iSaveY;
};

#endif