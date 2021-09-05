class CHudHeadName : public CHudBase
{
public:
	int Draw(float flTime);
	int Init(void);
	int VidInit(void);
	void Reset(void);

public:
	void BuildUnicodeList(void);
	BOOL IsValidEntity(cl_entity_s *pEntity);

private:
	wchar_t m_sUnicodes[MAX_CLIENTS][32];
	float m_flNextBuild;
	vgui2::HFont m_hHeadFont;
};