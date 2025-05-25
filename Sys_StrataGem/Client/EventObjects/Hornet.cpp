#include "framework.h"
#include "Hornet.h"
#include "GameInstance.h"

CHornet::CHornet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEventObject{ pDevice, pContext }
{
}

CHornet::CHornet(const CHornet& Prototype)
	: CEventObject{ Prototype }
{
}

HRESULT CHornet::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CHornet::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bActive = false;
	m_bEventCall = false;
	m_iGenerateDistance = 1500.f;

	return S_OK;
}

void CHornet::Priority_Update(_float fTimeDelta)
{
	if (true == m_bActive)
		Sortie(fTimeDelta);

	__super::Priority_Update(fTimeDelta);
}

void CHornet::Update(_float fTimeDelta)
{

}

void CHornet::Late_Update(_float fTimeDelta)
{
	if (false == m_bActive || true == m_bActivatedFirstFrame)
		return;

	if (true == m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 1.5f))
		m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
}

HRESULT CHornet::Render()
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

_bool CHornet::Event(_float fTimeDelta, _uint iEventType)
{
	_bool bEventEnd = false;

	if (HORNET_STRAFE == iEventType)
		bEventEnd = Strafe(fTimeDelta);
	else
		bEventEnd = HalfTon(fTimeDelta);

	return bEventEnd;
}

void CHornet::Sortie(_float fTimeDelta)
{
	// 최초 1회만 실행
	if (true == m_bActivatedFirstFrame)
	{
		m_bActivatedFirstFrame = false;
		m_bAttackEnable = false;
		// 목표 위치에서 y축 랜덤 회전
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&m_vDestination), 1.f));
		_float fRand = m_pGameInstance->Compute_Random(0, 360);
		m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fRand));
		
		// Look 방향으로 활성화 해제 지점 설정
		_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_vector vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		XMStoreFloat4(&m_vSortieDestination, XMVectorSetY(vPos + vLook * m_iGenerateDistance, 400.f));
		
		// Look 반대 방향으로 현재 위치 설정
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPos - vLook * m_iGenerateDistance, 400.f));
		m_pTransformCom->LookAt(XMLoadFloat4(&m_vSortieDestination));
	}
	// 활성화 해제 지점에 근접하면 활성화 해제
	_vector vCurrentPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float fLastDistance = XMVectorGetX(XMVector3Length(vCurrentPos - XMLoadFloat4(&m_vSortieDestination)));
	if (m_iGenerateDistance > fLastDistance)
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), -fTimeDelta * 4.f);
	else
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta);

	m_fSortieTimeAcc += fTimeDelta;
	if (m_fSortieTimeAcc >= 6.f)
	{
		m_bActive = false;
		m_fSortieTimeAcc = 0.f;
		m_bActivatedFirstFrame = true;
	}

	if(true == m_bActive)
		m_pTransformCom->Go_Straight(fTimeDelta);
}

_bool CHornet::Strafe(_float fTimeDelta)
{
	if (true == m_bActivatedFirstFrame)
		return false;

	if (true == m_bActive)
		m_fCountTimeAcc += fTimeDelta;

	if (false == m_bAttackEnable)
	{
		_vector vCurrentPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float fLastDistance = XMVectorGetX(XMVector3Length(vCurrentPos - XMLoadFloat3(&m_vDestination)));

		if (fLastDistance < m_iGenerateDistance * 0.65f)
		{
			m_bAttackEnable = true;
			m_fCountTimeAcc = 0.f;
		}
	}
	else
	{
		if (m_fCountTimeAcc >= m_fCountDelay)
		{
			m_fCountTimeAcc = 0.f;
			if (m_iCountAcc >= m_iCount)
			{
				m_bAttackEnable = false;
				m_iCountAcc = 0;
				return true;
			}
			else
			{
				_vector vPos{}, vRight{}, vLook{}, vTargetPos{};
				_float fRand{};

				vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vDestination), 1.f);

				vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
				vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

				// vRight, vLook 써서 랜덤 좌표로 쏘기
				fRand = m_pGameInstance->Compute_Random(10.f, 60.f);
				vTargetPos += fRand * vRight;

				fRand = m_pGameInstance->Compute_Random(10.f, 120.f);
				vTargetPos += fRand * vLook;

				m_pGameInstance->Pool_Get(CPoolingObject::POOL_AUTOCANNON, vTargetPos, vPos, 15.f, 30.f);
			}
			m_iCountAcc++;
		}
	}
	
	
	return false;
}

_bool CHornet::HalfTon(_float fTimeDelta)
{
	if (true == m_bActivatedFirstFrame)
		return false;

	if (false == m_bAttackEnable)
	{
		_vector vCurrentPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float fLastDistance = XMVectorGetX(XMVector3Length(vCurrentPos - XMLoadFloat3(&m_vDestination)));

		if (fLastDistance < m_iGenerateDistance * 0.25f)
			m_bAttackEnable = true;
	}
	else
	{		
		_vector vPos{}, vTargetPos{};
		vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vDestination), 1.f);
		m_pGameInstance->Pool_Get(CPoolingObject::POOL_HALFTON, vTargetPos, vPos, 15.f, 100.f);
		return true;
	}


	return false;
}

HRESULT CHornet::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Hornet"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CHornet::Bind_ShaderResources()
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

CHornet* CHornet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHornet* pInstance = new CHornet(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CHornet");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHornet::Clone(void* pArg)
{
	CHornet* pInstance = new CHornet(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CHornet");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHornet::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

}
