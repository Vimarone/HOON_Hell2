#include "framework.h"
#include "Supply.h"
#include "GameInstance.h"
#include "Player.h"

CSupply::CSupply(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEventObject{ pDevice, pContext }
{
}

CSupply::CSupply(const CSupply& Prototype)
	: CEventObject{ Prototype }
{
}

HRESULT CSupply::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSupply::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bActive = false;
	m_bEventCall = false;
	m_bActivatedFirstFrame = true;
	m_bLanded = false;
	m_fDissolveAmount = 0.f;

	return S_OK;
}

void CSupply::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSupply::Update(_float fTimeDelta)
{
	if (false == m_bEventCall && true == m_bActive)// 디졸브 후 액티브 해제
	{
		m_fDissolveAmount += fTimeDelta * 0.5f;
		if (1.f <= m_fDissolveAmount)
		{
			m_bActive = false;
			m_fDissolveAmount = 0.f;
		}
	}
}

void CSupply::Late_Update(_float fTimeDelta)
{
	if (false == m_bActive || true == m_bActivatedFirstFrame)
		return;

	if (true == m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 10.f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
#ifdef _DEBUG
		m_pGameInstance->Add_DebugComponent(m_pNavigationCom);
		m_pGameInstance->Add_DebugComponent(m_pColliderCom);
#endif
	}
}

HRESULT CSupply::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint			iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, i, aiTextureType_DIFFUSE, "g_DiffuseTexture", 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, i, aiTextureType_NORMALS, "g_NormalTexture", 0)))
			return E_FAIL;

		if (0.f == m_fDissolveAmount)
			m_pShaderCom->Begin(0);
		else
			m_pShaderCom->Begin(3);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

_bool CSupply::Event(_float fTimeDelta, _uint iEventType)
{
	if (true == m_bActivatedFirstFrame)
	{
		m_bLanded = false;
		m_bActivatedFirstFrame = false;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMVectorSetY(XMLoadFloat3(&m_vDestination), 800.f), 1.f));
		m_pNavigationCom->Set_CurrentCellIndex(m_pNavigationCom->Get_CellIndexByPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	}

	if (false == m_bLanded)
	{
		_float fNavY{}, fPosY{};
		_vector	vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		fNavY = m_pNavigationCom->Compute_Height(vPosition);
		fPosY = XMVectorGetY(vPosition);
		if (fNavY <= fPosY)
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, fPosY - fTimeDelta * m_pTransformCom->Get_Speed()));
		else
		{
			m_bLanded = true;
			m_pGameInstance->Effect_Get(CPoolEffect::SMOKE_SPREAD_SMALL, vPosition);
			m_pGameInstance->Effect_Get(CPoolEffect::SMOKE_SPREAD_MIDDLE, vPosition);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, fNavY - 5.f));
		}
	}
	else
	{
		m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

		CCollider* pTargetCollider = static_cast<CCollider*>(m_pGameInstance->Find_Component(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Com_Collider")));
		if (nullptr == pTargetCollider)
			return false;

		_bool isColl = m_pColliderCom->Intersect(pTargetCollider);

		if (true == isColl)
		{
			static_cast<CPlayer*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front())->SupplyAll();
			return true;
		}
	}

	return false;
}

HRESULT CSupply::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Noise"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_Collider */
	CBounding_AABB::BOUNDING_AABB_DESC	ColliderDesc{};

	ColliderDesc.vExtents = _float3(2.f, 10.f, 2.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vExtents.y, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_EscapePod"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* Com_Navigation */
	CNavigation::NAVIGATION_DESC		Desc{};

	Desc.iCurrentCellIndex = 0;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSupply::Bind_ShaderResources()
{
	m_ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	m_ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (m_fDissolveAmount != 0.f)
	{
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture", 0)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_DissolveAmount", &m_fDissolveAmount, sizeof(_float))))
			return E_FAIL;
	}

	return S_OK;
}

CSupply* CSupply::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSupply* pInstance = new CSupply(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CSupply");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSupply::Clone(void* pArg)
{
	CSupply* pInstance = new CSupply(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CSupply");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSupply::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
