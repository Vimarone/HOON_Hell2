#include "framework.h"
#include "GameInstance.h"

#include "GemArrow.h"

CGemArrow::CGemArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject { pDevice, pContext }
{
}

CGemArrow::CGemArrow(const CGemArrow& Prototype)
	:CUIObject { Prototype }
{
}

HRESULT CGemArrow::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemArrow::Initialize(void* pArg)
{
	GEMARROW_DESC* pDesc = static_cast<GEMARROW_DESC*>(pArg);
	m_eType = pDesc->eType;
	m_iIndex = pDesc->iIndex;

	m_pParentPosX = pDesc->pParentPosX;
	pDesc->fSizeX = pDesc->fSizeX * 0.5f;
	pDesc->fSizeY = pDesc->fSizeY * 0.5f;

	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	pDesc->fX = *m_pParentPosX - m_fSizeX * 0.25f + (m_fSizeY + 2.f) * m_iIndex;
	pDesc->fY = pDesc->fY + (m_fSizeY * 0.5f);
	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);
	m_iTextureNum = pDesc->iArrow;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fY = vPos.m128_f32[1];

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CGemArrow::Priority_Update(_float fTimeDelta)
{
}

void CGemArrow::Update(_float fTimeDelta)
{
	m_fX = *m_pParentPosX - m_fSizeX * 0.25f + (m_fSizeY + 2.f) * m_iIndex;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
}

void CGemArrow::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_UI, this);
}

HRESULT CGemArrow::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureNum)))
		return E_FAIL;

	// 체크 진행 중인데 시퀀스 종료 상태인 경우 회색 처리 과정 추가 필요
	// 체크 진행 중에 현재 인덱스 미만인 것들도 회색 처리 필요

	_uint iShaderPass = 3;
	if ((true == m_pGameInstance->Get_GemCheck() && m_iIndex < m_pGameInstance->Get_CurrentArrowIndex()) || false == m_pGameInstance->Get_SequeningByKey(m_eType))
		iShaderPass = 4;
	m_pShaderCom->Begin(iShaderPass);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CGemArrow::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_GemArrow"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;


	return S_OK;
}

CGemArrow* CGemArrow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemArrow* pInstance = new CGemArrow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemArrow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemArrow::Clone(void* pArg)
{
	CGemArrow* pInstance = new CGemArrow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemArrow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemArrow::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
