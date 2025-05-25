#pragma once
#include "PoolingObject.h"
#include "PoolEffect.h"

BEGIN(Engine)

class CPool_Manager final : public CBase
{

private:
	CPool_Manager();
	virtual ~CPool_Manager() = default;

public:
	HRESULT Initialize();
	void	Priority_Update(_float fTimeDelta);
	void	Update(_float fTimeDelta);
	void	Late_Update(_float fTimeDelta);

	void	Clear();

public:
	HRESULT						Pool_Set(CPoolingObject::POOL_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg);
	CPoolingObject*				Pool_Get(CPoolingObject::POOL_TYPE eType, _fvector vTargetPos, _fvector vStartPos, _float fLifeTime, _float fDistance, _bool bIsBallistic);
	vector<CPoolingObject*>*	Get_Pools(CPoolingObject::POOL_TYPE eType) { return &Pools[eType]; }

	HRESULT						Effect_Set(CPoolEffect::EFFECT_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg);
	CPoolEffect*				Effect_Get(CPoolEffect::EFFECT_TYPE eType, _fvector vStartPos, _fvector vTargetPos);

private:
	vector<CPoolingObject*>		Pools[CPoolingObject::POOL_END];

	vector<CPoolEffect*>		Effects[CPoolEffect::EFFECT_END];
	class CGameInstance*		m_pGameInstance = { nullptr };

public:
	static CPool_Manager* Create();
	virtual void Free() override;
};

END