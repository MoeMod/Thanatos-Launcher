#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

#include "client.h"
#include "CounterStrikeViewport.h"

#define ConvertCRtoNL ConvertCRtoNL2
#define StripEndNewlineFromString StripEndNewlineFromString2

pfnUserMsgHook g_pfnTextMsg;

DECLARE_MESSAGE(m_TextMessage, TextMsg);

int CHudTextMessage::Init(void)
{
	g_pfnTextMsg = HOOK_MESSAGE(TextMsg);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
};

char *CHudTextMessage::LocaliseTextString(const char *msg, char *dst_buffer, int buffer_size)
{
	char *dst = dst_buffer;

	for (char *src = (char *)msg; *src != 0 && buffer_size > 0; buffer_size--)
	{
		if (*src == '#')
		{
			static char word_buf[255];
			char *wdst = word_buf, *word_start = src;

			for (++src ; (*src >= 'A' && *src <= 'z') || (*src >= '0' && *src <= '9'); wdst++, src++)
				*wdst = *src;

			*wdst = 0;

			client_textmessage_t *clmsg = TextMessageGet(word_buf);

			if (!clmsg || !(clmsg->pMessage))
			{
				src = word_start;
				*dst = *src;
				dst++, src++;
				continue;
			}

			for (char *wsrc = (char *)clmsg->pMessage; *wsrc != 0; wsrc++, dst++)
				*dst = *wsrc;

			*dst = 0;
		}
		else
		{
			*dst = *src;
			dst++, src++;
			*dst = 0;
		}
	}

	dst_buffer[buffer_size - 1] = 0;
	return dst_buffer;
}

char *CHudTextMessage::BufferedLocaliseTextString(const char *msg)
{
	static char dst_buffer[1024];
	LocaliseTextString(msg, dst_buffer, 1024);
	return dst_buffer;
}

char *CHudTextMessage::LookupString(const char *msg, int *msg_dest)
{
	if (!msg)
		return "";

	if (msg[0] == '#')
	{
		char *result = NULL;
		client_textmessage_t *clmsg = TextMessageGet(msg + 1);

		if (!clmsg || !(clmsg->pMessage))
		{
			result = (char *)msg;
		}
		else
		{
			if (msg_dest)
			{
				if (clmsg->effect < 0)
					*msg_dest = -clmsg->effect;
			}

			result = (char *)clmsg->pMessage;
		}

		if (result)
		{
			if (result[0] == '#')
			{
				static char convert[1024];
				const wchar_t *pBuf = vgui2::localize()->Find(result);

				if (pBuf)
				{
					vgui2::localize()->ConvertUnicodeToANSI(pBuf, convert, sizeof(convert));
					return convert;
				}
			}

			return result;
		}

		return "";
	}

	return (char *)msg;
}

void StripEndNewlineFromString(char *str)
{
	int s = strlen(str) - 1;

	if (str[s] == '\n' || str[s] == '\r')
		str[s] = 0;
}

char *ConvertCRtoNL(char *str)
{
	for (char *ch = str; *ch != 0; ch++)
	{
		if (*ch == '\r')
			*ch = '\n';
	}

	return str;
}

int CHudTextMessage::MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf)
{
	if (!cl_newchat->value)
		return g_pfnTextMsg(pszName, iSize, pbuf);

	if (gViewPortInterface && !gViewPortInterface->AllowedToPrintText())
		return 1;

	BEGIN_READ(pbuf, iSize);

	int msg_dest = READ_BYTE();

	if (msg_dest == HUD_PRINTRADIO || msg_dest == HUD_PRINTTALK)
	{
		if (g_pViewPort->FireMessage(pszName, iSize, pbuf))
			return 1;
	}

	return g_pfnTextMsg(pszName, iSize, pbuf);
}