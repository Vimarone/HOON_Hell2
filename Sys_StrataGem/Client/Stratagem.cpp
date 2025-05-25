#include "framework.h"
#include "Stratagem.h"
#include "GameInstance.h"
#include "EventObject.h"
#include "Terrain.h"
#include "PoolEffect.h"

CStratagem::CStratagem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPoolingObject{ pDevice, pContext }
{


}

CStratagem::CStratagem(const CStratagem& Prototype)
	: CPoolingObject{ Prototype }
	, m_bMarkOn {Prototype.m_bMarkOn}
{
}

HRESULT CStratagem::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;


	return S_OK;
}

HRESULT CStratagem::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	CTransform::TRANSFORM_DESC* pDesc = static_cast<CTransform::TRANSFORM_DESC*>(pArg);
	m_fFirstSpeed = pDesc->fSpeedPerSec;

	m_eType = CPoolingObject::POOL_GEM;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fExplodeTime = 1.f;
	return S_OK;
}

void CStratagem::Priority_Update(_float fTimeDelta)
{
	if (false == m_bActive && false == m_bMarkOn)
		return;

	if (false == m_bMarkOn)
		m_pTransformCom->Go_Straight(fTimeDelta);
	else
		Laser(fTimeDelta);
}

void CStratagem::Update(_float fTimeDelta)
{
	if (false == m_bActive || true == m_bMarkOn)
		return;

	m_fLaunchTimeAcc += fTimeDelta;

	if (false == m_bAdded)
	{
		if (m_fLaunchTimeAcc >= m_fExplodeTime)
		{
			m_bAdded = true;
			m_pGameInstance->Add_Projectiles(this);
		}
	}
	_float fSpeed{};
	if (true == m_bBallistic)
	{
		_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float fSpeedPow{}, fBallisticY{};
		fSpeed = m_fFirstSpeed * sin(m_fRadian) - GRAVITY * m_fLaunchTimeAcc;
		m_pTransformCom->Set_Speed(fSpeed);
		m_fBallisticDistAcc += fTimeDelta * fSpeed;
		fSpeedPow = pow(fSpeed, 2);
		fBallisticY = m_fFirstY + m_fBallisticDistAcc * tan(m_fRadian) - (GRAVITY / (2.f * fSpeedPow * pow(cosf(m_fRadian), 2))) * pow(m_fBallisticDistAcc, 2);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPos, fBallisticY));
	}
	else
	{
		fSpeed = m_fFirstSpeed * pow((m_fLifeTime - m_fLaunchTimeAcc) / m_fLifeTime, 2) * 0.2f;
		m_pTransformCom->Set_Speed(fSpeed);

		if (XMVectorGetX(XMVector3Dot(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMVectorSet(0.f, 1.f, 0.f, 0.f))) > -0.9f)
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta);
	}
	m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));
}

void CStratagem::Late_Update(_float fTimeDelta)
{
	if (true == m_bMarkOn || false == m_bActive)
		return;

	 
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
#ifdef _DEBUG
	m_pGameInstance->Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CStratagem::Render()
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

void CStratagem::Set_Explode(_fvector vPos)
{
	__super::Set_Explode(vPos);
	// 부딪힌 지점(현재 좌표)의 x, z 좌표 + 터레인의 y 좌표 계산
	// _vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	CTerrain* pTerrain = static_cast<CTerrain*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_BackGround"))->front());
	_vector vGroundPos = XMVectorSetY(vPos, pTerrain->Get_Height(vPos));
	XMStoreFloat4(&m_vGroundPosition, vGroundPos);
	// 게임 인스턴스의 Get_EventObject로 타입에 맞는 오브젝트 받아와서 이벤트 호출 : Execute_Event(m_iType, y값 갱신 좌표)
	CEventObject* pEventObject = m_pGameInstance->Get_EventObject(m_iGemType);
	pEventObject->Execute_Event(m_iGemType, vGroundPos);	

	if (nullptr == pEventObject)
		return;

	// 광선 유지 시간 = 가져온 이벤트 오브젝트의 딜레이 시간
	m_pCurrentLaser = m_pGameInstance->Effect_Get(CPoolEffect::STRATA_LASER, Get_Position());
	if (nullptr == m_pCurrentLaser)
		return;
	Safe_AddRef(m_pCurrentLaser);
	m_bMarkOn = true;
	m_fMarkTimeAcc = 0.f;
	if(ORBITAL_PRECISION < m_iGemType)
		m_fMarkTime = pEventObject->Get_CallDelay() * 0.5f;
	else
		m_fMarkTime = pEventObject->Get_CallDelay() * 0.25f;
}

void CStratagem::Laser(_float fTimeDelta)
{
	m_fMarkTimeAcc += fTimeDelta;

	// 광선 종료 시 완전 비활성화
	if (m_fMarkTimeAcc >= m_fMarkTime)
	{
		static_cast<CVIBuffer_Particle_Instancing*>(m_pCurrentLaser->Find_Component(g_strVIBufferTag))->Loop_End();
		Safe_Release(m_pCurrentLaser);
		m_bMarkOn = false;
		m_bActive = false;
		m_fMarkTimeAcc = 0.f;
	}
}

HRESULT CStratagem::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Stratagem"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC	SphereDesc{};

	SphereDesc.fRadius = 0.5f;
	SphereDesc.vCenter = _float3(0.f, 0.f, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &SphereDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CStratagem::Bind_ShaderResources()
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

CStratagem* CStratagem::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStratagem* pInstance = new CStratagem(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CStratagem");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStratagem::Clone(void* pArg)
{
	CStratagem* pInstance = new CStratagem(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CStratagem");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStratagem::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pColliderCom);

	if (nullptr != m_pCurrentLaser)
		Safe_Release(m_pCurrentLaser);
}
