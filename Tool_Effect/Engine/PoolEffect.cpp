#include "PoolEffect.h"
#include "GameInstance.h"

CPoolEffect::CPoolEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPoolEffect::CPoolEffect(const CPoolEffect& Prototype)
	:CGameObject{ Prototype }
{
}

HRESULT CPoolEffect::Initialize_Prototype(void* pArg)
{
	return S_OK;
}

HRESULT CPoolEffect::Initialize(void* pArg)
{
	// 클라이언트 게임 플레이 씬에서 각 이펙트(각 객체X) 풀링 할 때 파일 입출력하고 desc 만들어서 객체에 던진 후  여기 던지기

	// 트랜스폼
	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// 오브젝트 정보 채우기
	EFFECTPOOL_DESC* pDesc = static_cast<EFFECTPOOL_DESC*>(pArg);
	m_bPlayUVAnim = pDesc->bPlayUVAnim;
	m_iPattern = pDesc->iPattern;
	m_fFinalSize = pDesc->fFinalSize;
	m_vStartColor = pDesc->vStartColor;
	m_vEndColor = pDesc->vEndColor;
	
	m_vNumAtlas.x = pDesc->vNumAtlas.x;
	m_vNumAtlas.y = pDesc->vNumAtlas.y;
	m_vNumAtlas.z = pDesc->iNumAtlas;
	m_vUVInterval = pDesc->vUVInterval;


	// 버퍼
	CVIBuffer_Particle_Instancing::PARTICLE_INSTANCING_DESC Desc = pDesc->ParticleDesc;
	m_iNumInstance = Desc.iNumInstance;

	m_pVIBufferCom = CVIBuffer_Particle_Instancing::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBufferCom)
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Initialize(&Desc)))
		return E_FAIL;

	m_Components.emplace(g_strVIBufferTag, m_pVIBufferCom);
	Safe_AddRef(m_pVIBufferCom);

	m_bLightAdd = false;
	m_bActive = false;

	return S_OK;
}

void CPoolEffect::Priority_Update(_float fTimeDelta)
{
}

void CPoolEffect::Update(_float fTimeDelta)
{
	if (false == m_bActive || nullptr == m_pVIBufferCom)
		return;

	_bool bEffectEnd = false;
	switch (m_iPattern)
	{
	case CVIBuffer_Particle_Instancing::EXPLOSION:
		bEffectEnd = m_pVIBufferCom->Explosion(fTimeDelta);
		break;
	case CVIBuffer_Particle_Instancing::DROP:
		bEffectEnd = m_pVIBufferCom->Drop(fTimeDelta);
		break;
	case CVIBuffer_Particle_Instancing::RISE:
		bEffectEnd = m_pVIBufferCom->Rise(fTimeDelta);
		break;
	case CVIBuffer_Particle_Instancing::FRACTION:
		bEffectEnd = m_pVIBufferCom->Fraction(fTimeDelta);
		break;
	case CVIBuffer_Particle_Instancing::HOLD:
		bEffectEnd = m_pVIBufferCom->Hold(fTimeDelta);
		break;
	}
	if (true == m_bPlayUVAnim)
		m_vUVInterval = _float2(1.f / (_float)m_vNumAtlas.x, 1.f / (_float)m_vNumAtlas.y);

	if (true == bEffectEnd)
	{
		if (true == m_bActive)
		{
			m_bActive = false;
			m_pVIBufferCom->Reset_Life();
		}
	}
}

void CPoolEffect::Late_Update(_float fTimeDelta)
{

}

HRESULT CPoolEffect::Render()
{
	return S_OK;
}

void CPoolEffect::Compute_CamDistance()
{
	_float4			vCamPosition = m_pGameInstance->Get_CamPosition();
	_vector			vWorldPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	m_fCamDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(&vCamPosition) - vWorldPos));
}

void CPoolEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
}
