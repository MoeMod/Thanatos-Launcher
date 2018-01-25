
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "event_api.h"
#include <triangleapi.h>
#include "calcscreen.h"

#include "z4e_damagestar.h"

static CHudZ4EDamageStar g_HudZ4EDamageStar;
CHudZ4EDamageStar &HudZ4EDamageStar()
{
	return g_HudZ4EDamageStar;
}

int CHudZ4EDamageStar::MsgFunc_Z4E_DamageStar(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	BYTE n = READ_BYTE();
	BYTE iType = READ_BYTE();
	BYTE iOriginStatus = READ_BYTE();

	for (int i = 0; i < n; i++)
	{
		Vector2D start(ScreenWidth / 2 - 26, ScreenHeight / 2 - 26);
		if (iOriginStatus)
		{
			float origin[3] = { READ_FLOAT(), READ_FLOAT(), READ_FLOAT() };
			float scr[2];
			CalcScreen(origin, scr);
			start = scr;
		}

		g_HudZ4EDamageStar.m_iStarCount++;
		if (g_HudZ4EDamageStar.m_iStarCount >= 10)
		{
			// ...
			g_HudZ4EDamageStar.Accumulate();
			g_HudZ4EDamageStar.m_iStarCount %= 10;
		}

		float angle = g_HudZ4EDamageStar.m_iStarCount * 38.f *  (M_PI_F / 180.0f);
		
		Vector2D delta;
		SinCos(angle, &delta[1], &delta[0]);
		Vector2D end = start + delta * 100.f;
		g_HudZ4EDamageStar.AddStar(start, end, 0.5f, iType, 0);
	}
	
	return 0;
}

int CHudZ4EDamageStar::Init(void)
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	MSG_HookUserMsg("DamageStar", CHudZ4EDamageStar::MsgFunc_Z4E_DamageStar);

	//Cmd_HookCmd("test", [](){
	//	g_HudZ4EDamageStar.AddStar(Vector2D( ScreenWidth / 2, ScreenHeight / 2 ), Vector2D( ScreenWidth - 50, ScreenHeight - 100 ), 1.0, 0);
	//});

	return 1;
}

int CHudZ4EDamageStar::VidInit(void)
{
	m_iStarCount = 0;
	return 1;
}


int CHudZ4EDamageStar::Draw(float flTime)
{
	for (auto iter = m_StarList.cbegin(); iter != m_StarList.cend(); /* nothing here*/)
	{
		auto &p = *iter;
		if (!p->ShouldDraw(flTime))
		{
			iter = m_StarList.erase(iter);
			continue;
		}
		p->Draw(flTime);
		++iter;
	}
	return 0;
}

int CHudZ4EDamageStar::Accumulate()
{
	Vector2D end(ScreenWidth - 50, ScreenHeight - 100);
	for (auto iter = m_StarList.cbegin(); iter != m_StarList.cend(); ++iter)
	{
		auto &p = *iter;
		if (p->m_iStatus)
			continue;
		p->m_iStatus = 1;
		p->m_vecStart = p->GetPos();
		p->m_vecEnd = end;
		p->m_flStartTime = gEngfuncs.GetClientTime();
		p->m_flDuration = 1.f;
	}
	return 0;
}

void CHudZ4EDamageStar::AddStar(const Vector2D &vecStart, const Vector2D &vecEnd, float flDuration, BYTE iType, BYTE iStatus)
{
	float time = gEngfuncs.GetClientTime();
	m_StarList.push_front(std::make_unique<Star>(Star{ vecStart, vecEnd, time, flDuration, iType, iStatus }));
}

bool CHudZ4EDamageStar::Star::ShouldDraw(float flTime) const
{
	return !m_iStatus || flTime - m_flStartTime < m_flDuration;
}

void CHudZ4EDamageStar::Star::Draw(float flTime) const
{
	Vector2D vecPosition = GetPos();
	static auto star1 = vgui::scheme()->GetImage("resource/Hud/damagestar/NoBossStar", true);
	static auto star2 = vgui::scheme()->GetImage("resource/Hud/damagestar/AIDamageEffect", true);
	static auto star_ef = vgui::scheme()->GetImage("resource/Hud/damagestar/NoBossEffect", true);
	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);

	auto &star = m_iType ? star2 : star1;

	float percent = (flTime - m_flStartTime) / m_flDuration;
	if (percent > 1)
	{
		int i = (int)percent;
		percent = (i & 1) ? (0.8f + (percent - i) / 5.f) : (1.0f - (percent - i) / 5.f);
	}
		

	if (m_iStatus)
	{
		
		star->SetColor(Color(255, 255, 255, 255 * (1 - percent)));
		star->SetPos(vecPosition.x, vecPosition.y);
		star->Paint();

		star_ef->SetColor(Color(255, 255, 255, 255 * (1 - percent) * (1 - percent) * 0.75f));
		star_ef->SetPos(vecPosition.x, vecPosition.y);
		star_ef->Paint();
	}
	else
	{
		star->SetColor(Color(255, 255, 255, 255 * percent * 0.75));
		star->SetPos(vecPosition.x, vecPosition.y);
		star->Paint();
	}
}

Vector2D CHudZ4EDamageStar::Star::GetPos() const
{
	float percent = (gEngfuncs.GetClientTime() - m_flStartTime) / m_flDuration;
	if (percent > 1)
		percent = 1;
	return m_vecStart + (m_vecEnd - m_vecStart) * percent * percent;
}