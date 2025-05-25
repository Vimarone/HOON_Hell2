#include "framework.h"
#include "Widow.h"
#include "GameInstance.h"
#include "HighExplosive.h"

CWidow::CWidow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEventObject{ pDevice, pContext }
{
}

CWidow::CWidow(const CWidow& Prototype)
	: CEventObject{ Prototype }
{
}

HRESULT CWidow::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CWidow::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(-500.f, 1000.f, 100.f, 1.f));
	return S_OK;
}

void CWidow::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWidow::Update(_float fTimeDelta)
{

}

void CWidow::Late_Update(_float fTimeDelta)
{
	if (true == m_bActive)
	{
		if (true == m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 1.5f))
			m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
	}
}

HRESULT CWidow::Render()
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

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}

	return S_OK;
}

_bool CWidow::Event(_float fTimeDelta, _uint iEventType)
{
	_bool bEventEnd = false;
	if (ORBITAL_PRECISION == iEventType)
		bEventEnd = Orbital_Precision();
	else
		bEventEnd = Orbital_380(fTimeDelta);

	return bEventEnd;
}

_bool CWidow::Orbital_Precision()
{
	_vector vPos{}, vLook{}, vPoolLook{}, vTargetPos{};
	vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION) + vLook * 60.f;
	vPoolLook = vPos + vLook;
	vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vDestination), 1.f);
	CPoolingObject* pPoolObject = m_pGameInstance->Pool_Get(CPoolingObject::POOL_HIGHEXPL, vPoolLook, vPos, 15.f, 60.f);
	CHighExplosive* pHighExpl = static_cast<CHighExplosive*>(pPoolObject);
	pHighExpl->Set_Destination(vTargetPos);
	pHighExpl->Set_BezierTime(m_fCallDelay * 0.5f);

	return true;
}

_bool CWidow::Orbital_380(_float fTimeDelta)
{
	// µÙ∑π¿Ã 8.45√ , π¸¿ß 50m
	// 3πﬂ 5»∏, √— 15πﬂ
	// »∏¥Á ∞£∞› 3√ , ¬¯≈∫ ∞£∞› 1.5√ 

	if (true == m_bAttackEnable)
	{
		if (m_iCountAcc < m_iCount)
		{
			m_fCountTimeAcc += fTimeDelta;
			if (m_fCountTimeAcc >= m_fCountDelay)
			{
				m_iCountAcc++;
				m_fCountTimeAcc = 0.f;
				_vector vPos{}, vRight{}, vLook{}, vPoolLook{}, vTargetPos{};
				_float fRand{};
				vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
				vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION) + vLook * 60.f;
				fRand = m_pGameInstance->Compute_Random(-m_fEffectiveRange, m_fEffectiveRange);
				vPoolLook = vPos + vLook;
				vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vDestination) + vLook * fRand + vRight * fRand, 1.f);
				CPoolingObject* pPoolObject = m_pGameInstance->Pool_Get(CPoolingObject::POOL_HIGHEXPL, vPoolLook, vPos, 15.f, 60.f);
				CHighExplosive* pHighExpl = static_cast<CHighExplosive*>(pPoolObject);
				pHighExpl->Set_Destination(vTargetPos);
				pHighExpl->Set_BezierTime(m_fCallDelay * 0.25f);
			}
		}
		else
		{
			m_bAttackEnable = false;
			m_iCountAcc = 0;
			m_iFrequencyAcc++;
			m_fFrequencyTimeAcc = 0.f;
		}
	}
	else
	{
		if (m_iFrequencyAcc < m_iFrequency)
		{
			m_fFrequencyTimeAcc += fTimeDelta;
			if (m_fFrequencyTimeAcc >= m_fFrequencyDelay)
				m_bAttackEnable = true;
		}
		else
			return true;
	}
	return false;
}

HRESULT CWidow::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Widow"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CWidow::Bind_ShaderResources()
{
	m_ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	m_ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

CWidow* CWidow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWidow* pInstance = new CWidow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CWidow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWidow::Clone(void* pArg)
{
	CWidow* pInstance = new CWidow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CWidow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWidow::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
