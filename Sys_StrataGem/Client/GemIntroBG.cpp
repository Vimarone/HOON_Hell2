#include "framework.h"
#include "GameInstance.h"

#include "GemIntroBG.h"

CGemIntroBG::CGemIntroBG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject { pDevice, pContext }
{
}

CGemIntroBG::CGemIntroBG(const CGemIntroBG& Prototype)
	:CUIObject { Prototype }
{
}

HRESULT CGemIntroBG::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemIntroBG::Initialize(void* pArg)
{
	GEMBG_DESC* pDesc = static_cast<GEMBG_DESC*>(pArg);

	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Gems()))
		return E_FAIL;

	return S_OK;
}

void CGemIntroBG::Priority_Update(_float fTimeDelta)
{
}

void CGemIntroBG::Update(_float fTimeDelta)
{
}

void CGemIntroBG::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_UIBG, this);
}

HRESULT CGemIntroBG::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;


	m_pShaderCom->Begin(0);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CGemIntroBG::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_UI_BackGround"),
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

HRESULT CGemIntroBG::Ready_Gems()
{
	// _uint iGemNum = m_pGameInstance->Get
	return S_OK;
}

CGemIntroBG* CGemIntroBG::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemIntroBG* pInstance = new CGemIntroBG(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemIntroBG");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemIntroBG::Clone(void* pArg)
{
	CGemIntroBG* pInstance = new CGemIntroBG(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemIntroBG");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemIntroBG::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
