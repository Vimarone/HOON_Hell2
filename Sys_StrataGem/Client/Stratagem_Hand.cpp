#include "framework.h"
#include "..\Public\Stratagem_Hand.h"

#include "GameInstance.h"
#include "Player.h"
#include "Camera_Follow.h"
#include "Stratagem.h"

CStratagem_Hand::CStratagem_Hand(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{


}

CStratagem_Hand::CStratagem_Hand(const CStratagem_Hand& Prototype)
	: CPartObject{ Prototype }
{
}

HRESULT CStratagem_Hand::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CStratagem_Hand::Initialize(void* pArg)
{
	STRATA_DESC* pDesc = static_cast<STRATA_DESC*>(pArg);

	m_pSocketMatrix = pDesc->pSocketMatrix;
	m_pParentState = pDesc->pParentState;
	m_pActivatedGem = pDesc->pActivatedGem;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, 0.1f, 1.f));

	return S_OK;
}

void CStratagem_Hand::Priority_Update(_float fTimeDelta)
{

}

void CStratagem_Hand::Update(_float fTimeDelta)
{
	if (false == m_bActive)
		return;

	if (*m_pParentState & CPlayer::STATE_STRATAGEM_READY)
	{
		if (m_pGameInstance->Button_Down(MOUSEKEYSTATE::DIM_LB) || *m_pParentState & CPlayer::STATE_KNOCKBACK)
		{
			*m_pParentState ^= CPlayer::STATE_STRATAGEM_READY;
			*m_pParentState |= CPlayer::STATE_STRATAGEM_THROW;
			CPlayer* pPlayer = static_cast<CPlayer*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front());
			pPlayer->SwapWeapon(CPlayer::WEAPON_END);
			CPoolingObject* pPoolObject = nullptr;
			// KNOCKBACK 상태면 드랍, 아니면 던지기
			_float4x4* pCombinedWorldMatrix = nullptr;
			_float fMouseYAcc = 0.f;
			_vector vStartPos{}, vUpDir{}, vLookDir{}, vTargetPos{};
			pCombinedWorldMatrix = Get_CombinedWorldMatrixPtr();
			vStartPos = XMLoadFloat4(reinterpret_cast<_float4*>(pCombinedWorldMatrix->m[CTransform::STATE_POSITION]));
			CTransform* pPlayerTransform = static_cast<CTransform*>(pPlayer->Find_Component(g_strTransformTag));
			vUpDir = pPlayerTransform->Get_State(CTransform::STATE_UP);
			vLookDir = pPlayerTransform->Get_State(CTransform::STATE_LOOK);
			fMouseYAcc = static_cast<CCamera_Follow*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Camera"))->front())->Get_CalculatedMouseYAcc();
			if (*m_pParentState & CPlayer::STATE_KNOCKBACK)
				vTargetPos = XMVectorSetW(vStartPos - vUpDir, 1);
			else
				vTargetPos = XMVectorSetW(vStartPos + vLookDir - vUpDir * fMouseYAcc * 3.f, 1);
			if(fMouseYAcc < -0.03f)
				pPoolObject = m_pGameInstance->Pool_Get(CPoolingObject::POOL_GEM, vTargetPos, vStartPos, 20.f, 1.f, true);
			else
				pPoolObject = m_pGameInstance->Pool_Get(CPoolingObject::POOL_GEM, vTargetPos, vStartPos, 20.f, 1.f, false);

			if (nullptr == pPoolObject)
				return;

			static_cast<CStratagem*>(pPoolObject)->Set_GemType(pPlayer->Get_ActivatedGem());

			m_bActive = false;
		}
	}

	_matrix			SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);

	for (size_t i = 0; i < 3; i++)
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);

	XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr())
		* SocketMatrix * XMLoadFloat4x4(m_pParentWorldMatrix));
}

void CStratagem_Hand::Late_Update(_float fTimeDelta)
{
	if (false == m_bActive)
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
}

HRESULT CStratagem_Hand::Render()
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

HRESULT CStratagem_Hand::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Stratagem"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStratagem_Hand::Bind_ShaderResources()
{
	m_ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	m_ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

CStratagem_Hand* CStratagem_Hand::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStratagem_Hand* pInstance = new CStratagem_Hand(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CStratagem_Hand");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStratagem_Hand::Clone(void* pArg)
{
	CStratagem_Hand* pInstance = new CStratagem_Hand(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CStratagem_Hand");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStratagem_Hand::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
