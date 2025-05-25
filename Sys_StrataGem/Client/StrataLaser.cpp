#include "framework.h"
#include "StrataLaser.h"
#include "GameInstance.h"

CStrataLaser::CStrataLaser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPoolEffect{ pDevice, pContext }
{
}

CStrataLaser::CStrataLaser(const CStrataLaser& Prototype)
	: CPoolEffect{ Prototype }
{
}

HRESULT CStrataLaser::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStrataLaser::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
	pDesc->fSpeedPerSec = 5.f;
	pDesc->fRotationPerSec = XMConvertToRadians(90.f);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CStrataLaser::Priority_Update(_float fTimeDelta)
{
}

void CStrataLaser::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CStrataLaser::Late_Update(_float fTimeDelta)
{
	if (false == m_bActive)
		return;

	if (1 < m_iNumInstance)
		m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONLIGHT, this);
	else
		m_pGameInstance->Add_RenderGroup(CRenderer::RG_BLENDEFCT, this);
}

HRESULT CStrataLaser::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (1 == m_iNumInstance)
		m_pShaderCom->Begin(2);
	else if (true == m_bPlayUVAnim)
		m_pShaderCom->Begin(1);
	else
		m_pShaderCom->Begin(0);

	m_pVIBufferCom->Bind_InputAssembler();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CStrataLaser::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_WHITE"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxPointInstance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStrataLaser::Bind_ShaderResources()
{
	m_ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	m_ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (1 == m_iNumInstance)
	{
		if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
			return E_FAIL;
	}

	m_vCamPosition = m_pGameInstance->Get_CamPosition();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_vCamPosition, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vStartColor", &m_vStartColor, sizeof(_float3))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEndColor", &m_vEndColor, sizeof(_float3))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fFinalSize", &m_fFinalSize, sizeof(_float))))
		return E_FAIL;

	if (true == m_bPlayUVAnim)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vNumAtlas", &m_vNumAtlas, sizeof(_int3))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVInterval", &m_vUVInterval, sizeof(_float2))))
			return E_FAIL;
	}

	return S_OK;
}

CStrataLaser* CStrataLaser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStrataLaser* pInstance = new CStrataLaser(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CStrataLaser");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStrataLaser::Clone(void* pArg)
{
	CStrataLaser* pInstance = new CStrataLaser(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CStrataLaser");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStrataLaser::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
