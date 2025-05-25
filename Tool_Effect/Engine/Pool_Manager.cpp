#include "..\Public\Pool_Manager.h"

#include "GameInstance.h"


CPool_Manager::CPool_Manager()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPool_Manager::Initialize()
{
	return S_OK;
}

void CPool_Manager::Priority_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < CPoolingObject::POOL_END; i++)
	{
		for (auto& pObject : Pools[i])
		{
			pObject->Priority_Update(fTimeDelta);
		}
	}

	for (size_t i = 0; i < CPoolEffect::EFFECT_END; i++)
	{
		for (auto& pEffect : Effects[i])
		{
			pEffect->Priority_Update(fTimeDelta);
		}
	}
}

void CPool_Manager::Update(_float fTimeDelta)
{
	for (size_t i = 0; i < CPoolingObject::POOL_END; i++)
	{
		for (auto& pObject : Pools[i])
		{
			pObject->Update(fTimeDelta);
		}
	}
	
	for (size_t i = 0; i < CPoolEffect::EFFECT_END; i++)
	{
		for (auto& pEffect : Effects[i])
		{
			pEffect->Update(fTimeDelta);
		}
	}
}

void CPool_Manager::Late_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < CPoolingObject::POOL_END; i++)
	{
		for (auto& pObject : Pools[i])
		{
			pObject->Late_Update(fTimeDelta);
		}
	}

	for (size_t i = 0; i < CPoolEffect::EFFECT_END; i++)
	{
		for (auto& pEffect : Effects[i])
		{
			pEffect->Late_Update(fTimeDelta);
		}
	}
}

void CPool_Manager::Clear()
{
	for (size_t i = 0; i < CPoolingObject::POOL_END; i++)
	{
		for (auto& pObject : Pools[i])
		{
			Safe_Release(pObject);
		}
		Pools[i].clear();
	}

	for (size_t i = 0; i < CPoolEffect::EFFECT_END; i++)
	{
		for (auto& pEffect : Effects[i])
		{
			Safe_Release(pEffect);
		}
		Effects[i].clear();
	}
}

HRESULT CPool_Manager::Pool_Set(CPoolingObject::POOL_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg)
{
	if (Pools[eType].size() != 0)
		return E_FAIL;

	Pools[eType].reserve(iNum);

	CPoolingObject* pPoolObject = { nullptr };

	for (size_t i = 0; i < iNum; i++)
	{
		pPoolObject = static_cast<CPoolingObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ePrototypeLevelID, strPrototypeTag, pArg));
		if (nullptr == pPoolObject)
			return E_FAIL;

		Pools[eType].push_back(pPoolObject);
	}
	
	return S_OK;
}

CPoolingObject* CPool_Manager::Pool_Get(CPoolingObject::POOL_TYPE eType, _fvector vTargetPos, _fvector vStartPos, _float fLifeTime, _float fDistance, _bool bIsBallistic)
{
	for (auto& pPoolObject : Pools[eType])
	{
		if (false == pPoolObject->Get_Active())
		{
			pPoolObject->Set_Type(eType);
			CTransform* pPoolTransform = static_cast<CTransform*>(pPoolObject->Find_Component(g_strTransformTag));
			pPoolTransform->Set_State(CTransform::STATE_POSITION, vStartPos);
			pPoolTransform->LookAt(vTargetPos);
			if (CPoolingObject::POOL_MISSILE == eType)
			{
				pPoolObject->Set_Target(vTargetPos);
				_float fRand = m_pGameInstance->Compute_Random(-2, 2);
				_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				pPoolTransform->LookAt(vStartPos + vUp);
				_vector vRight = pPoolTransform->Get_State(CTransform::STATE_RIGHT);
				pPoolTransform->LookAt(vStartPos + vUp + vRight * fRand);
			}
			pPoolObject->Set_LifeTime(fLifeTime);
			// pPoolObject->Set_Distance(fDistance);
			pPoolObject->Set_Active();
			if (true == bIsBallistic)
				pPoolObject->Set_Ballistic();
			return pPoolObject;
		}
	}
	
	return nullptr;
}

HRESULT CPool_Manager::Effect_Set
(CPoolEffect::EFFECT_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg)
{
	if (Effects[eType].size() != 0)
		return E_FAIL;

	Effects[eType].reserve(iNum);

	CPoolEffect* pPoolEffect = { nullptr };

	for (size_t i = 0; i < iNum; i++)
	{
		pPoolEffect = static_cast<CPoolEffect*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ePrototypeLevelID, strPrototypeTag, pArg));
		if (nullptr == pPoolEffect)
			return E_FAIL;

		pPoolEffect->Set_Active(false);
		Effects[eType].push_back(pPoolEffect);
	}

	return S_OK;
}

CPoolEffect* CPool_Manager::Effect_Get(CPoolEffect::EFFECT_TYPE eType, _fvector vStartPos, _fvector vTargetPos)
{
	for (auto& pPoolEffect : Effects[eType])
	{
		if (false == pPoolEffect->Get_Active())
		{
			pPoolEffect->Set_Active();
			CTransform* pPoolTransform = static_cast<CTransform*>(pPoolEffect->Find_Component(g_strTransformTag));
			pPoolTransform->Set_State(CTransform::STATE_POSITION, vStartPos);
			if(XMVectorGetX(vTargetPos) != 0.f)
				pPoolTransform->LookAt(vTargetPos);
			return pPoolEffect;
		}
	}

	return nullptr;
}


CPool_Manager* CPool_Manager::Create()
{
	CPool_Manager* pInstance = new CPool_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed To Created : CPool_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPool_Manager::Free()
{
	__super::Free();

	for (size_t i = 0; i < CPoolingObject::POOL_END; i++)
	{
		for (auto& pObject : Pools[i])
		{
			Safe_Release(pObject);
		}
		Pools[i].clear();
	}

	for (size_t i = 0; i < CPoolEffect::EFFECT_END; i++)
	{
		for (auto& pEffect : Effects[i])
		{
			Safe_Release(pEffect);
		}
		Effects[i].clear();
	}

	Safe_Release(m_pGameInstance);
}
