#include "framework.h"
#include "GameInstance.h"

#include "GemInfo.h"
#include "GemSequence.h"

CGemInfo::CGemInfo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIContainer{ pDevice, pContext }
{
}

CGemInfo::CGemInfo(const CGemInfo& Prototype)
	:CUIContainer{ Prototype }
{
}

HRESULT CGemInfo::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemInfo::Initialize(void* pArg)
{
	GEMINFO_DESC* pDesc = static_cast<GEMINFO_DESC*>(pArg);

	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	m_pParentPosX = pDesc->pParentPosX;
	pDesc->fX = *m_pParentPosX - pDesc->fSizeX * 0.5f + (pDesc->fSizeY - 5.f) * 0.5f + 2.f;
	pDesc->fSizeX = pDesc->fSizeY - 5.f;
	pDesc->fSizeY = pDesc->fSizeY - 5.f;

	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	m_eType = pDesc->eType;

	Set_Name(m_eType);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fY = vPos.m128_f32[1];

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_GemSequence()))
		return E_FAIL;

	return S_OK;
}

void CGemInfo::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGemInfo::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	m_fX = *m_pParentPosX - m_fSizeX * 0.5f + (m_fSizeY - 5.f) * 0.5f + 2.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
}

void CGemInfo::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_UI, this);
}

HRESULT CGemInfo::Render()
{
	_float fCoolTime{}, fCoolRate{};
	m_pGameInstance->Get_CoolInfo(m_eType, fCoolTime, fCoolRate);


	
	_uint2 vViewportSize = m_pGameInstance->Get_ViewportSize();
	// 쿨타임 진행 중이 아니면 시퀀스 출력
	if (0.f == fCoolTime)
	{
		m_pGameInstance->Render_Font(TEXT("Font_Hell"), m_szName, _float2((m_fX + vViewportSize.x * 0.5f) - m_fSizeX - 20.f, -(m_fY) + vViewportSize.y * 0.5f - m_fSizeY * 0.5f));
		__super::Render();
	}
	// 아니면 쿨타임 폰트 출력(쿨타임 T-00:00)
	else
	{
		m_pGameInstance->Render_Font(TEXT("Font_Hell"), m_szName, _float2((m_fX + vViewportSize.x * 0.5f) - m_fSizeX - 20.f, -(m_fY) + vViewportSize.y * 0.5f - m_fSizeY * 0.5f), XMVectorSet(0.7f, 0.7f, 0.7f, 1.f));
		wsprintf(m_szCool, TEXT("쿨다운 T-%d:%02d"), (_int)fCoolTime / 60, (_int)fCoolTime % 60);
		m_pGameInstance->Render_Font(TEXT("Font_Hell"), m_szCool, _float2((m_fX + vViewportSize.x * 0.5f) - m_fSizeX - 20.f, -(m_fY) + vViewportSize.y * 0.5f));
	}

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	// 체크 진행 중인데 시퀀스 종료 상태인 경우 회색 처리 과정 추가 필요

	m_pShaderCom->Begin(4);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();


	return S_OK;
}

HRESULT CGemInfo::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_GemIconBG"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxPosTexDiscard"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;


	return S_OK;
}

HRESULT CGemInfo::Ready_GemSequence()
{
	CUIObject* pGemUI = nullptr;
	CGemSequence::GEMSEQ_DESC SeqDesc{};
	SeqDesc.eType = m_eType;
	SeqDesc.pParentPosX = &m_fX;
	SeqDesc.fX = m_fX;
	SeqDesc.fY = m_fY;
	SeqDesc.fSizeX = m_fSizeX;
	SeqDesc.fSizeY = m_fSizeY;

	pGemUI = static_cast<CGemSequence*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemSequence"), &SeqDesc));
	
	m_PartUIs.push_back(pGemUI);
	return S_OK;
}

void CGemInfo::Set_Name(GEM_TYPE eType)
{
	switch (eType)
	{
	case REINFORCEMENT:
		wsprintf(m_szName, TEXT("증원"));
		break;
	case SUPPLY:
		wsprintf(m_szName, TEXT("재보급"));
		break;
	case HORNET_STRAFE:
		wsprintf(m_szName, TEXT("이글 기총소사"));
		break;
	case HORNET_HALF:
		wsprintf(m_szName, TEXT("이글 500kg 폭탄"));
		break;
	case ORBITAL_PRECISION:
		wsprintf(m_szName, TEXT("궤도 정밀 타격"));
		break;
	case ORBITAL_380:
		wsprintf(m_szName, TEXT("궤도 380mm 고폭 폭격"));
		break;
	case SENTRY_GATLING:
		wsprintf(m_szName, TEXT("A/G-16 개틀링 센트리"));
		break;
	default:
		break;
	}
	
}

CGemInfo* CGemInfo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemInfo* pInstance = new CGemInfo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemInfo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemInfo::Clone(void* pArg)
{
	CGemInfo* pInstance = new CGemInfo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemInfo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemInfo::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
