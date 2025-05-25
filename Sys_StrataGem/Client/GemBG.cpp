#include "framework.h"
#include "GameInstance.h"

#include "GemBG.h"
#include "GemUI.h"
#include "Player.h"

CGemBG::CGemBG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIContainer{ pDevice, pContext }
{
}

CGemBG::CGemBG(const CGemBG& Prototype)
	:CUIContainer{ Prototype }
{
}

HRESULT CGemBG::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemBG::Initialize(void* pArg)
{
	GEMBG_DESC* pDesc = static_cast<GEMBG_DESC*>(pArg);

	m_pPlayerState = pDesc->pPlayerState;

	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;

	_uint2	vViewportSize = m_pGameInstance->Get_ViewportSize();
	m_fInitialX = pDesc->fX - (vViewportSize.x * 0.5f);
	m_fInActiveX = m_fInitialX - pDesc->fSizeX * 2.f;

	pDesc->fX = m_fInActiveX;
	m_fX = pDesc->fX;

	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 3000.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fY = pDesc->fY;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Gems()))
		return E_FAIL;

	m_bActive = false;

	return S_OK;
}

void CGemBG::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fX = vPos.m128_f32[0];
}

void CGemBG::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	// 플레이어 상태에 따라 애니메이션 넣기
	if (*m_pPlayerState & CPlayer::STATE_STRATAGEM)
	{
		if (false == m_bActive)
			InOutAnimation(fTimeDelta);
	}
	else
	{
		if (true == m_bActive)
			InOutAnimation(fTimeDelta);
	}
}

void CGemBG::Late_Update(_float fTimeDelta)
{
	if (true == g_bShutDown)
		return;

	__super::Late_Update(fTimeDelta);
	// m_pGameInstance->Add_RenderGroup(CRenderer::RG_UIBG, this);
}

HRESULT CGemBG::Render()
{
	__super::Render();

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

HRESULT CGemBG::Ready_Components()
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

HRESULT CGemBG::Ready_Gems()
{
	// 젬 매니저로부터 정보 받아와서 GemUI 생성
	CGemUI* pGemUI = nullptr;
	CGemUI::GEMUI_DESC Desc{};
	_uint iGemCount = m_pGameInstance->Get_GemCount();

	for (size_t i = 0; i < iGemCount; i++)
	{
		Desc.eType = (GEM_TYPE)m_pGameInstance->Get_KeyByIndex(i);
		Desc.iIndex = i;
		Desc.pParentPosX = &m_fX;
		Desc.fX = m_fX;
		Desc.fY = m_fY;
		Desc.fSizeX = m_fSizeX;
		Desc.fSizeY = m_fSizeY;
		pGemUI = static_cast<CGemUI*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemUI"), &Desc));
		m_PartUIs.push_back(pGemUI);
	}
	return S_OK;
}

void CGemBG::InOutAnimation(_float fTimeDelta)
{
	// m_fInActiveX -> m_fInitialX
	if (false == m_bActive)
	{
		m_fX += fTimeDelta * m_pTransformCom->Get_Speed();
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));

		if (m_fX > m_fInitialX)
		{
			m_fX = m_fInitialX;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
			m_bActive = true;
		}
	}
	// m_fInitialX -> m_fInActiveX
	else
	{
		m_fX -= fTimeDelta * m_pTransformCom->Get_Speed();
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));

		if (m_fX < m_fInActiveX)
		{
			m_fX = m_fInActiveX;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
			m_bActive = false;
		}
	}
}

CGemBG* CGemBG::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemBG* pInstance = new CGemBG(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemBG");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemBG::Clone(void* pArg)
{
	CGemBG* pInstance = new CGemBG(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemBG");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemBG::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
