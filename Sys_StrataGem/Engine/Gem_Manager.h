#pragma once
#include "Base.h"

BEGIN(Engine)

class CGem_Manager final : public CBase
{
private:
	CGem_Manager();
	virtual ~CGem_Manager() = default;

public:
	HRESULT			Initialize();
	void			Update(_float fTimeDelta);

public:
	// UI¿ë	
	_int				Get_KeyByIndex(_uint iIndex);
	_uint				Get_GemCount() { return _uint(m_Gems.size()); }
	vector<_uint>&		Get_SeqByKey(_uint iKey);
	HRESULT				Get_CoolInfo(_uint iGemKey, _float& fOutCoolTime, _float& fOutCoolRate);
	_uint				Get_CurrentArrowIndex() { return m_iCurrentIndex; }
	_bool				Get_GemCheck() { return m_bCheck; }
	_bool				Get_SequeningByKey(_uint iKey);

	// ÄÁÅÙÃ÷¿ë
	class CEventObject* Get_EventObject(_uint iKey) { return Find_EventObject(iKey); }
public:
	HRESULT				Add_Gem(_uint iGemKey, vector<_uint>& GemSequence, _float fCoolTime, class CEventObject* pEventObject);

	void				Check_Start();
	_int				Check_Gems(_uint iKeyInput);
	void				Check_End();

private:
	class CGem*			Find_Gem(_uint iGemKey);
	class CEventObject* Find_EventObject(_uint iGemKey);

private:
	map<_uint, class CGem*>				m_Gems;
	map<_uint, class CEventObject*>		m_EventObjects;

	_bool								m_bCheck{};
	_uint								m_iCurrentIndex{};

public:
	static CGem_Manager* Create();
	virtual void Free() override;
};

END