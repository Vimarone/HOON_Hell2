#include "pch.h"
#include "Dummy_Mesh.h"
#include "GameInstance.h"

CDummy_Mesh::CDummy_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: super(pDevice, pContext)
{
}

CDummy_Mesh::CDummy_Mesh(const CDummy_Mesh& Prototype)
	: super(Prototype)
{
}

HRESULT CDummy_Mesh::Initialize_Prototype()
{
	if (FAILED(super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDummy_Mesh::Initialize(void* pArg)
{
	super::DESC* pDesc = static_cast<super::DESC*>(pArg);
	if (nullptr == pDesc)
		return E_FAIL;

	if (FAILED(super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc->strModelTag, pDesc->strTextureTag, (LEVELID)pDesc->eLevelID)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vTranslation), 1.f));
	m_pTransformCom->Set_Scale(pDesc->vScale);
	m_isActive = true;

	return S_OK;
}

void CDummy_Mesh::Priority_Update(_float fTimeDelta)
{
	Compute_CamDistance();
}

void CDummy_Mesh::Update(_float fTimeDelta)
{
	if (false == m_isActive)
		return;

	m_fActiveDelayAcc -= fTimeDelta;
	if (m_fActiveDelayAcc > 0.f)
		return;

	switch (m_iEffectType)
	{
	case MESH_TRAIL:
		if (m_vUVProgress.x == 1.f)
			m_fLifeTimeAcc += fTimeDelta;
		break;
	case MESH:
		if (true == m_bIsLoopEnd || false == m_bIsLoop || (true == m_bIsLoop && ((m_fLifeTimeAcc + fTimeDelta) / m_fLifeTime < m_vDissolveStart.y)))
			m_fLifeTimeAcc += fTimeDelta;
		break;
	}

	if (m_fLifeTimeAcc >= m_fLifeTime)
	{
		if (true == m_bIsLoop)
			m_fLifeTimeAcc = 0.f;
		else
			m_isActive = false;
	}
	
	if (m_iMeshAnimOption & super::ANIM_TRANS)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (m_iMeshAnimOption & super::ANIM_ROTATE)
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE::UP), fTimeDelta);
	if (m_iMeshAnimOption & super::ANIM_SCALE)
	{
		_float fLerpScale = m_fFinalSize * (1 + (m_fLifeTimeAcc / m_fLifeTime));
		m_pTransformCom->Set_Scale(fLerpScale, fLerpScale, fLerpScale);
	}

	if (true == m_bUVAnim)
	{
		if (MESH_TRAIL == m_iEffectType)
		{
			if (m_vUVProgress.x + m_vUVSpeed.x * fTimeDelta < 1.f)
			{
				m_vUVProgress.x += m_vUVSpeed.x * fTimeDelta;
				m_vUVProgress.y += m_vUVSpeed.y * fTimeDelta;
			}
			else
				m_vUVProgress = _float2(1.f, 1.f);
		}
		else
		{
			m_vUVProgress.x += m_vUVSpeed.x * fTimeDelta;
			m_vUVProgress.y += m_vUVSpeed.y * fTimeDelta;
		}
	}

	super::Update(fTimeDelta);
}

void CDummy_Mesh::Late_Update(_float fTimeDelta)
{
	if (false == m_isActive)
		return;

	if (m_fActiveDelayAcc > 0.f)
		return;

	super::Late_Update(fTimeDelta);

	if(true == m_bBlendEnable)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::WEIGHTEDBLEND, this);
	else
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::NONLIGHT, this);
	if (m_iShadingOption & EFFECT_OPTION::DISTORTION)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::DISTORTION, this);
	
}

HRESULT CDummy_Mesh::Render()
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
		if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_NoiseTexture", m_iDissolveNum)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveStart", &m_vDissolveStart, sizeof(_float2))))
			return E_FAIL;

		_float fLifeRate = m_fLifeTimeAcc / m_fLifeTime;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fLifeRate", &fLifeRate, sizeof(_float))))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::EMISSIVE)
	{
		if (FAILED(m_pEmissiveTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_EmissiveTexture", m_iEmissiveNum)))
			return E_FAIL;
	}

	if (MESH_TRAIL == m_iEffectType)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaxTrailLength", &m_fMaxTrailLength, sizeof(_float))))
			return E_FAIL;
		if (true == m_bBlendEnable)
			m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::BLENDACC_TRAIL);
		else
			m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::TRAIL);
	}
	else
	{
		if (true == m_bBlendEnable)
			m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::BLENDACC);
		else
			m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::DEFAULT);
	}

	for (_uint i = 0; i < (_uint)m_pModelCom->Get_NumMeshes(); ++i)
		m_pModelCom->Render(i);

	return S_OK;
}

HRESULT CDummy_Mesh::Render_Revealage()
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
		if (FAILED(m_pNoiseTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_NoiseTexture", m_iNoiseNum)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveStart", &m_vDissolveStart, sizeof(_float2))))
			return E_FAIL;

		_float fLifeRate = m_fLifeTimeAcc / m_fLifeTime;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fLifeRate", &fLifeRate, sizeof(_float))))
			return E_FAIL;
	}

	if (m_iShadingOption & EFFECT_OPTION::EMISSIVE)
	{
		if (FAILED(m_pEmissiveTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_EmissiveTexture", m_iEmissiveNum)))
			return E_FAIL;
	}

	if (MESH_TRAIL == m_iEffectType)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaxTrailLength", &m_fMaxTrailLength, sizeof(_float))))
			return E_FAIL;
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::REVEAL_TRAIL);
	}
	else
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::REVEAL);

	for (_uint i = 0; i < (_uint)m_pModelCom->Get_NumMeshes(); ++i)
		m_pModelCom->Render(i);

	return S_OK;
}

HRESULT CDummy_Mesh::Render_Distortion()
{
	//_float3 vOriginScale = m_pTransformCom->Get_Scale();
	//_vector vScaledScale = XMLoadFloat3(&vOriginScale) * 1.1f;
	//_float3 vStoredScaledScale = {};
	//XMStoreFloat3(&vStoredScaledScale, vScaledScale);
	//m_pTransformCom->Set_Scale(vStoredScaledScale);
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

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVProgress", &m_vUVProgress, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTileCount", &m_vUVTileCount, sizeof(_float2))))
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

		_float fLifeRate = m_fLifeTimeAcc / m_fLifeTime;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fLifeRate", &fLifeRate, sizeof(_float))))
			return E_FAIL;
	}

	if (MESH_TRAIL == m_iEffectType)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaxTrailLength", &m_fMaxTrailLength, sizeof(_float))))
			return E_FAIL;
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::DISTORTION_TRAIL);
	}
	else
		m_pShaderCom->Begin((_uint)SHADER_PASS_EFFECTMESH::DISTORTION);
	
	for (_uint i = 0; i < (_uint)m_pModelCom->Get_NumMeshes(); ++i)
		m_pModelCom->Render(i);

	// m_pTransformCom->Set_Scale(vOriginScale);
	return S_OK;
}

HRESULT CDummy_Mesh::Ready_Components(const _wstring& strModelTag, const _wstring& strTextureTag, LEVELID eLevelID)
{
	/* Com_Shader */
	if (FAILED(Add_Component(eLevelID, L"Prototype_Component_Shader_VtxMesh_Effect",
		L"Com_Shader", reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(Add_Component(eLevelID, strModelTag, L"Com_Model", reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(Add_Component(eLevelID, strTextureTag, L"Com_Texture", reinterpret_cast<CComponent**>(&m_pTextureCom))))
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

CDummy_Mesh* CDummy_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDummy_Mesh* pInstance = new CDummy_Mesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CDummy_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CCloneable* CDummy_Mesh::Clone(void* pArg)
{
	CDummy_Mesh* pInstance = new CDummy_Mesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CDummy_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDummy_Mesh::Free()
{
	super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pMaskTextureCom);
	Safe_Release(m_pNoiseTextureCom);
	Safe_Release(m_pEmissiveTextureCom);
}
