#include "mathlib/Vector2D.h"
#include <list>
#include <memory>

class CHudZ4EDamageStar : public CHudBase
{
public:
	virtual int Init(void);
	virtual int VidInit(void);
	virtual int Draw(float flTime);

public:
	static int MsgFunc_Z4E_DamageStar(const char *pszName, int iSize, void *pbuf);
	void AddStar(const Vector2D &vecStart, const Vector2D &vecEnd, float flDuration, BYTE iType, BYTE iStatus);
	int Accumulate();

private:
	struct Star
	{
		Vector2D m_vecStart;
		Vector2D m_vecEnd;
		float m_flStartTime;
		float m_flDuration;
		BYTE m_iType;
		BYTE m_iStatus;

		bool ShouldDraw(float flTime) const;
		void Draw(float flTime) const;
		Vector2D GetPos() const;
	};
	int m_iStarCount;
	std::list<std::unique_ptr<Star>> m_StarList;
};

CHudZ4EDamageStar &HudZ4EDamageStar();