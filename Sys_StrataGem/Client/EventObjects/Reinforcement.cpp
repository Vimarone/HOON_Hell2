#include "framework.h"
#include "Reinforcement.h"
#include "GameInstance.h"
#include "Player.h"

CReinforcement::CReinforcement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEventObject{ pDevice, pContext }
{
}

CReinforcement::CReinforcement(const CReinforcement& Prototype)
	: CEventObject{ Prototype }
{
}

HRESULT CReinforcement::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CReinforcement::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_bActive = false;
	m_bEventCall = false;

	return S_OK;
}

void CReinforcement::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (nullptr == m_pPlayerState)
		m_pPlayerState = static_cast<CPlayer*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front())->Get_State_Ptr();
}

void CReinforcement::Update(_float fTimeDelta)
{

}

void CReinforcement::Late_Update(_float fTimeDelta)
{

}

HRESULT CReinforcement::Render()
{
	return S_OK;
}

_bool CReinforcement::Event(_float fTimeDelta, _uint iEventType)
{
	// 플레이어 사망 상태이면 리스폰
	if (nullptr != m_pPlayerState)
	{
		if (*m_pPlayerState & CPlayer::STATE_DIE)
			*m_pPlayerState |= CPlayer::STATE_SPAWN;
	}
	return true;
}

CReinforcement* CReinforcement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CReinforcement* pInstance = new CReinforcement(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CReinforcement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CReinforcement::Clone(void* pArg)
{
	CReinforcement* pInstance = new CReinforcement(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CReinforcement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CReinforcement::Free()
{
	__super::Free();
}
