#include "EventObject.h"
#include "GameInstance.h"

CEventObject::CEventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CEventObject::CEventObject(const CEventObject& Prototype)
	: CGameObject{Prototype}
	, m_bEventCall{ Prototype.m_bEventCall }
{
}

HRESULT CEventObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEventObject::Initialize(void* pArg)
{

	EVENT_DESC* pDesc = static_cast<EVENT_DESC*>(pArg);

	m_fCallDelay = pDesc->fCallDelay;
	m_fEffectiveRange = pDesc->fEffectiveRange;

	m_iCount = pDesc->iCount;
	m_fCountDelay = pDesc->fCountDelay;
	m_iFrequency = pDesc->iFrequency;
	m_fFrequencyDelay = pDesc->fFrequencyDelay;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_bActivatedFirstFrame = true;
	m_bAttackEnable = false;

	return S_OK;
}

void CEventObject::Priority_Update(_float fTimeDelta)
{
	if (true == m_bEventCall)
	{
		m_fCallTimeAcc += fTimeDelta;
		// 호출 딜레이 시간 지나면 호출 상태 진입(렌더 활성화)
		if (m_fCallTimeAcc >= 1.f)
		{
			if (false == m_bActive)
				m_bActive = true;

			_bool bIsEventEnd = false;
			// 렌더 활성화 시 각 객체에서 구현한 이벤트 진행
			if (true == m_bActive)
				bIsEventEnd = Event(fTimeDelta, m_iEventType);

			// 이벤트 종료 시 호출 상태 종료
			if (true == bIsEventEnd)
				Shutdown_Event();
		}
	}
}

void CEventObject::Update(_float fTimeDelta)
{
	
}

void CEventObject::Late_Update(_float fTimeDelta)
{
	
}

HRESULT CEventObject::Render()
{
    return S_OK;
}

void CEventObject::Execute_Event(_uint iEventType, _fvector vPoint)
{
	m_iEventType = iEventType;
	m_fCallTimeAcc = 0.f;
	m_iCountAcc = 0;
	m_iFrequencyAcc = 0;
	m_fFrequencyTimeAcc = 0.f;
	m_bEventCall = true;
	XMStoreFloat3(&m_vDestination, vPoint);
}

void CEventObject::Shutdown_Event()
{
	m_bEventCall = false;
}

void CEventObject::Free()
{
	__super::Free();
}
