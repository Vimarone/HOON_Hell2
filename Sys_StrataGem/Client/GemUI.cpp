#include "framework.h"
#include "GameInstance.h"

#include "GemUI.h"
#include "GemIcon.h"
#include "GemInfo.h"

CGemUI::CGemUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIContainer{ pDevice, pContext }
{
}

CGemUI::CGemUI(const CGemUI& Prototype)
	:CUIContainer{ Prototype }
{
}

HRESULT CGemUI::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemUI::Initialize(void* pArg)
{
	GEMUI_DESC* pDesc = static_cast<GEMUI_DESC*>(pArg);

	pDesc->fSizeY = pDesc->fSizeY / m_pGameInstance->Get_GemCount();

	if (0 == m_pGameInstance->Get_GemCount() % 2)
		pDesc->fY = pDesc->fY + (pDesc->iIndex - m_pGameInstance->Get_GemCount() * 0.5f) * pDesc->fSizeY + (pDesc->fSizeY * 0.5f);
	else
		pDesc->fY = pDesc->fY + (pDesc->iIndex - (m_pGameInstance->Get_GemCount() - 1) * 0.5f) * pDesc->fSizeY;
	
	m_pParentPosX = pDesc->pParentPosX;
	pDesc->fX = *m_pParentPosX;

	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	m_eType = pDesc->eType;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fX = vPos.m128_f32[0];
	m_fY = pDesc->fY;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Gems()))
		return E_FAIL;

	return S_OK;
}

void CGemUI::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGemUI::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	m_fX = *m_pParentPosX;
	Set_TransformX(m_fX);

	if (m_pGameInstance->Button_Down(MOUSEKEYSTATE::DIM_LB))
		_int i = 10;
}

void CGemUI::Late_Update(_float fTimeDelta)
{
	// X값 업데이트
	 __super::Late_Update(fTimeDelta);
	 m_pGameInstance->Add_RenderGroup(CRenderer::RG_UIBG, this);
}

HRESULT CGemUI::Render()
{

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;


	m_pShaderCom->Begin(2);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();

	__super::Render();
	return S_OK;
}

HRESULT CGemUI::Ready_Components()
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

HRESULT CGemUI::Ready_Gems()
{
	CUIObject* pGemUI = nullptr;
	CGemIcon::GEMICON_DESC IconDesc{};
	IconDesc.eType = m_eType;
	IconDesc.pParentPosX = &m_fX;
	IconDesc.fX = m_fX;
	IconDesc.fY = m_fY;
	IconDesc.fSizeX = m_fSizeX;
	IconDesc.fSizeY = m_fSizeY;
	pGemUI = static_cast<CGemIcon*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemIcon"), &IconDesc));
	m_PartUIs.push_back(pGemUI);

	CGemInfo::GEMINFO_DESC InfoDesc{};
	InfoDesc.eType = m_eType;
	InfoDesc.pParentPosX = &m_fX;
	InfoDesc.fX = m_fX;
	InfoDesc.fY = m_fY;
	InfoDesc.fSizeX = m_fSizeX;
	InfoDesc.fSizeY = m_fSizeY;
	pGemUI = static_cast<CGemInfo*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemInfo"), &InfoDesc));
	m_PartUIs.push_back(pGemUI);

	return S_OK;
}

CGemUI* CGemUI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemUI* pInstance = new CGemUI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemUI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemUI::Clone(void* pArg)
{
	CGemUI* pInstance = new CGemUI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemUI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemUI::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
