#include "pch.h"
#include "Dummy_MeshInst.h"
#include "GameInstance.h"

CDummy_MeshInst::CDummy_MeshInst(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: super(pDevice, pContext)
{
}

CDummy_MeshInst::CDummy_MeshInst(const CDummy_MeshInst& Prototype)
	: super(Prototype)
{
}

HRESULT CDummy_MeshInst::Initialize_Prototype()
{
	if (FAILED(super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDummy_MeshInst::Initialize(void* pArg)
{
	super::DESC* pDesc = static_cast<super::DESC*>(pArg);
	if (nullptr == pDesc)
		return E_FAIL;

	if (FAILED(super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	m_isActive = true;
	m_fLifeTime = pDesc->fLifeTime;

	m_pTransformCom->Set_State(CTransform::STATE::RIGHT, XMVectorSet(1.f, 0.f, 0.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE::UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vOriginLocation), 1.f));

	return S_OK;
}

void CDummy_MeshInst::Priority_Update(_float fTimeDelta)
{
	Compute_CamDistance();
}

void CDummy_MeshInst::Update(_float fTimeDelta)
{
	if (false == m_isActive)
		return;

	m_fActiveDelayAcc -= fTimeDelta;
	if (m_fActiveDelayAcc > 0.f)
		return;

	if (false == m_bIsLoop || (true == m_bIsLoop && ((m_fLifeTimeAcc + fTimeDelta) < m_vDissolveStart.y)))
		m_fLifeTimeAcc += fTimeDelta;

	if (m_fLifeTimeAcc >= m_fLifeTime)
	{
		if (true == m_bIsLoop)
			m_fLifeTimeAcc = 0.f;
		else
			m_isActive = false;
	}

	if (true == m_bUVAnim)
	{
		m_vUVProgress.x += m_vUVSpeed.x * fTimeDelta;
		m_vUVProgress.y += m_vUVSpeed.y * fTimeDelta;
	}

	super::Update(fTimeDelta);
}

void CDummy_MeshInst::Late_Update(_float fTimeDelta)
{
	if (false == m_isActive)
		return;

	if (m_fActiveDelayAcc > 0.f)
		return;

	m_pVIBufferCom->Progress(fTimeDelta);
	super::Late_Update(fTimeDelta);

	if (true == m_bBlendEnable)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::WEIGHTEDBLEND, this);
	else
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::NONLIGHT, this);
	if (m_iShadingOption & EFFECT_OPTION::DISTORTION)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::DISTORTION, this);

}

HRESULT CDummy_MeshInst::Render()
{
	if (FAILED(m_pTransformCom->Bind_WorldMatrix_To_Shader(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_BaseTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMulColor", &m_vMulColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vAddColor", &m_vAddColor, sizeof(_float3))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVProgress", &m_vUVProgress, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTileCount", &m_vUVTileCount, sizeof(_float2))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iShadingOption", &m_iShadingOption, sizeof(_uint))))
		return E_FAIL;

	if (m_iShadingOption & EFFECT_OPTION::MASK)
	{
		if (FAILED(m_pMaskTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_MaskTexture", m_iMaskNum)))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::DISSOLVE)
	{
		if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_DissolveTexture", m_iDissolveNum)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveStart", &m_vDissolveStart, sizeof(_float2))))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::EMISSIVE)
	{
		if (FAILED(m_pEmissiveTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_EmissiveTexture", m_iEmissiveNum)))
			return E_FAIL;
	}

	if (true == m_bBlendEnable)
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESHINST::BLENDACC);
	else
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESHINST::DEFAULT);
	
	m_pVIBufferCom->Bind_InputAssembler();
	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CDummy_MeshInst::Render_Revealage()
{
	if (FAILED(m_pTransformCom->Bind_WorldMatrix_To_Shader(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_BaseTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMulColor", &m_vMulColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vAddColor", &m_vAddColor, sizeof(_float3))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iShadingOption", &m_iShadingOption, sizeof(_uint))))
		return E_FAIL;

	if (m_iShadingOption & EFFECT_OPTION::MASK)
	{
		if (FAILED(m_pMaskTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_MaskTexture", m_iMaskNum)))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::DISSOLVE)
	{
		if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_NoiseTexture", m_iNoiseNum)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveStart", &m_vDissolveStart, sizeof(_float2))))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::EMISSIVE)
	{
		if (FAILED(m_pEmissiveTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_EmissiveTexture", m_iEmissiveNum)))
			return E_FAIL;
	}

	m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESHINST::REVEAL);

	m_pVIBufferCom->Bind_InputAssembler();
	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CDummy_MeshInst::Render_Distortion()
{
	if (m_pAdoptedMatrix == nullptr)
	{
		if (FAILED(m_pTransformCom->Bind_WorldMatrix_To_Shader(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_BaseTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iShadingOption", &m_iShadingOption, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_DistortionTexture", m_iDistortionNum)))
		return E_FAIL;

	if (m_iShadingOption & EFFECT_OPTION::MASK)
	{
		if (FAILED(m_pMaskTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_MaskTexture", m_iMaskNum)))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::DISSOLVE)
	{
		if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_NoiseTexture", m_iDissolveNum)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveStart", &m_vDissolveStart, sizeof(_float2))))
			return E_FAIL;
	}

	m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESHINST::DISTORTION);

	m_pVIBufferCom->Bind_InputAssembler();
	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CDummy_MeshInst::Ready_Components(void* pArg)
{
	super::DESC* pDesc = static_cast<super::DESC*>(pArg);
	_uint eLevelID = pDesc->BufferDesc.eLevelID;

	/* Com_Shader */
	if (FAILED(Add_Component(eLevelID, L"Prototype_Component_Shader_VtxMesh_Particle",
		L"Com_Shader", reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, L"Prototype_Component_VIBuffer_MeshInstancing_SRT",
		L"Com_VIBuffer", reinterpret_cast<CComponent**>(&m_pVIBufferCom), &pDesc->BufferDesc)))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, pDesc->strTextureTag, 
		L"Com_Texture", reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, TEXT("Prototype_Component_Texture_EffMask"),
		TEXT("Com_MaskTexture"), reinterpret_cast<CComponent**>(&m_pMaskTextureCom))))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, TEXT("Prototype_Component_Texture_EffNoise"),
		TEXT("Com_NoiseTexture"), reinterpret_cast<CComponent**>(&m_pNoiseTextureCom))))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, TEXT("Prototype_Component_Texture_EffEmissive"),
		TEXT("Com_EmissiveTexture"), reinterpret_cast<CComponent**>(&m_pEmissiveTextureCom))))
		return E_FAIL;

	return S_OK;
}

CDummy_MeshInst* CDummy_MeshInst::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDummy_MeshInst* pInstance = new CDummy_MeshInst(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CDummy_MeshInst");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CCloneable* CDummy_MeshInst::Clone(void* pArg)
{
	CDummy_MeshInst* pInstance = new CDummy_MeshInst(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CDummy_MeshInst");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDummy_MeshInst::Free()
{
	super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pMaskTextureCom);
	Safe_Release(m_pNoiseTextureCom);
	Safe_Release(m_pEmissiveTextureCom);
}
