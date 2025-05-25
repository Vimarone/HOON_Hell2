#include "framework.h"
#include "GameInstance.h"

#include "GemIcon.h"

CGemIcon::CGemIcon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject { pDevice, pContext }
{
}

CGemIcon::CGemIcon(const CGemIcon& Prototype)
	:CUIObject { Prototype }
{
}

HRESULT CGemIcon::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemIcon::Initialize(void* pArg)
{
	GEMICON_DESC* pDesc = static_cast<GEMICON_DESC*>(pArg);

	m_pParentPosX = pDesc->pParentPosX;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	pDesc->fX = *m_pParentPosX - pDesc->fSizeX * 0.5f + (pDesc->fSizeY - 5.f) * 0.5f + 5.f;
	pDesc->fSizeX = pDesc->fSizeY - 5.f;
	pDesc->fSizeY = pDesc->fSizeY - 5.f;


	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	m_eType = pDesc->eType;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fY = vPos.m128_f32[1];

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CGemIcon::Priority_Update(_float fTimeDelta)
{
	_int a = 10;
}

void CGemIcon::Update(_float fTimeDelta)
{
	m_fX = *m_pParentPosX - m_fSizeX * 0.5f + (m_fSizeY - 5.f) * 0.5f + 5.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
}

void CGemIcon::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_UI, this);
}

HRESULT CGemIcon::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	_float fCoolTime{}, fDiscardRate{};
	m_pGameInstance->Get_CoolInfo(m_eType, fCoolTime, fDiscardRate);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_DiscardRate", &fDiscardRate, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_eType)))
		return E_FAIL;




	// 체크 진행 중인데 시퀀스 종료 상태인 경우 회색 처리 과정 추가 필요
	//_uint iShaderPass = 3;
	//if (true == m_pGameInstance->Get_GemCheck() && false == m_pGameInstance->Get_SequeningByKey(m_eType))
	//	iShaderPass = 4;
	m_pShaderCom->Begin(3);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CGemIcon::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_StrataIcon"),
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

CGemIcon* CGemIcon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemIcon* pInstance = new CGemIcon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemIcon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemIcon::Clone(void* pArg)
{
	CGemIcon* pInstance = new CGemIcon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemIcon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemIcon::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
