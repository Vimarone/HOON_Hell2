#pragma once
#include "Base.h"

BEGIN(Engine)

class CGem final : public CBase
{
public:
	// ATTACK : R, DEFENCE : G, SUPPLY : B, OTHER : Y(yellow)
	enum GEM_TYPE { ATTACK, DEFENCE, SUPPLY, OTHER, GEM_END };

private:
	CGem();
	~CGem() = default;

public:
	HRESULT		Initialize(vector<_uint>& GemSequence, _float fCoolTime);
	void		Update(_float fTimeDelta);

public:
	_bool		Get_Cooling() { return m_bCooling; }
	_bool		Get_Activate() { return m_bActivate; }
	_bool		Get_Sequencing() { return m_bSequencing; }
	_uint		Get_SequenceEndIndex() { return _uint(m_GemSequence.size()) - 1; }
	_float		Get_ActivatedTimeAcc() { return m_fActivatedTimeAcc; }
	_float		Get_CoolRate() { return m_fActivatedTimeAcc / m_fCoolTime; }
	vector<_uint>& Get_Seq() { return m_GemSequence; }

public:
	void		Start_Sequencing() { m_bSequencing = true; }
	void		End_Sequencing() { m_bSequencing = false; }
	_bool		Compare_Sequence(_uint iIndex, _uint iInput);

	void		Activate() { m_bActivate = true; m_bCooling = true; m_bSequencing = false; }

private:
	_bool			m_bSequencing = { false };
	vector<_uint>	m_GemSequence;

	_bool			m_bCooling = { false };
	_float			m_fCoolTime = {};

	_bool			m_bActivate = { false };
	_float			m_fActivatedTimeAcc = {};

public:
	static CGem* Create(vector<_uint>& GemSequence, _float fCoolTime);
	virtual void Free() override;
};

END