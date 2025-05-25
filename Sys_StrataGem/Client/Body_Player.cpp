#include "framework.h"
#include "..\Public\Body_Player.h"

#include "GameInstance.h"
#include "Player.h"
#include "Camera_Follow.h"

CBody_Player::CBody_Player(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CPartObject { pDevice, pContext }
{
	//XMMatrixDecompose();

}

CBody_Player::CBody_Player(const CBody_Player & Prototype)
	: CPartObject { Prototype }
{
}

HRESULT CBody_Player::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CBody_Player::Initialize(void * pArg)
{	
	CBody_Player::BODY_PLAYER_DESC*	pDesc = static_cast<CBody_Player::BODY_PLAYER_DESC*>(pArg);

	m_pParentState = pDesc->pParentState;
	m_pParentDir = pDesc->pParentDir;
	m_pParentIdleTime = pDesc->pParentIdleTime;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pModelCom->SetUp_Animation(9, true);	

	return S_OK;
}

void CBody_Player::Priority_Update(_float fTimeDelta)
{
	if (nullptr == m_pPlayerCamera)
	{
		m_pPlayerCamera = static_cast<CCamera_Follow*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Camera"))->front());
		Safe_AddRef(m_pPlayerCamera);
	}

	if (*m_pParentState & CPlayer::STATE_SPAWN)
	{
		if(true == m_bDeadAnimEnd)
			m_bDeadAnimEnd = false;
	}
}

void CBody_Player::Update(_float fTimeDelta)
{
	_float fTime = fTimeDelta;
	if (true == m_bDeadAnimEnd)
	{
		fTime = 0.f;
		m_bIsArmStop = true;
	}
	else
	{
		_uint iAnimIndex = { 1 };
		_int iArmAnimIndex = { -1 }, iUpperAnimIndex = { -1 };
		_bool bIsLoop = { false }, bIsArmLoop = { false }, bIsUpperLoop = { false };

		if (!(*m_pParentState & CPlayer::STATE_STRATAGEM) && !(*m_pParentState & CPlayer::STATE_STRATAGEM_READY) && !(*m_pParentState & CPlayer::STATE_STRATAGEM_THROW))
		{
			Set_RifleAnimation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop, iUpperAnimIndex, bIsUpperLoop);
			Set_LauncherAnimation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop, iUpperAnimIndex, bIsUpperLoop);
			Set_ThrowAnimation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop, iUpperAnimIndex, bIsUpperLoop);
		}
		else if ((*m_pParentState & CPlayer::STATE_STRATAGEM_READY) || (*m_pParentState & CPlayer::STATE_STRATAGEM_THROW))
			Set_StrataAnimation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop);
		else
		{
			if (*m_pParentState & CPlayer::STATE_IDLE)
				iAnimIndex = CPlayer::IDLE_STATIC;
			else
				iAnimIndex = CPlayer::ROCKET_IDLE;
		}

		Set_KnockbackAnimation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop, iUpperAnimIndex, bIsUpperLoop);

		m_pModelCom->SetUp_Animation(iAnimIndex, bIsLoop, iArmAnimIndex, bIsArmLoop, iUpperAnimIndex, bIsUpperLoop, m_pPlayerCamera->Get_CalculatedMouseYAcc());
	}

	if (true == m_pModelCom->Play_Animation(fTime, m_bIsArmStop))
	{		
		if (*m_pParentState & CPlayer::STATE_DIE)
		{
			m_bDeadAnimEnd = true;
		}
		else if (*m_pParentState & CPlayer::STATE_KNOCKBACK)
		{
			*m_pParentState ^= CPlayer::STATE_KNOCKBACK;
			*m_pParentState |= CPlayer::STATE_KNOCKBACK_RECOVER;
		}
		else if (*m_pParentState & CPlayer::STATE_KNOCKBACK_RECOVER)
		{
			if(*m_pParentState & CPlayer::STATE_STRATAGEM_THROW)
				*m_pParentState ^= CPlayer::STATE_STRATAGEM_THROW;
			*m_pParentState ^= CPlayer::STATE_KNOCKBACK_RECOVER;
		}
	}

	if (*m_pParentState & CPlayer::STATE_STRATAGEM_READY)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
			m_bIsArmStop = true;
	}

	Set_MotionEnd();
	XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) * XMLoadFloat4x4(m_pParentWorldMatrix));
}

void CBody_Player::Late_Update(_float fTimeDelta)
{	
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
}

HRESULT CBody_Player::Render()
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

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, i, "g_BoneMatrices")))
			return E_FAIL;

		m_pShaderCom->Begin(0);		
		m_pModelCom->Render(i);
	}	

	return S_OK;
}

void CBody_Player::Set_RifleAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop)
{
	if (*m_pParentState & CPlayer::STATE_AIM)
	{
		if (*m_pParentState & CPlayer::STATE_IDLE)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::AIM_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_WALK)
		{
			bIsLoop = true;
			if (*m_pParentDir & CPlayer::DIR_N)
				iAnimIndex = CPlayer::AIM_WALK_N;
			else if (*m_pParentDir & CPlayer::DIR_S)
				iAnimIndex = CPlayer::AIM_WALK_S;
			else
				iAnimIndex = CPlayer::AIM_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_RUN)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::AIM_RUN_N;
		}
	}
	else 
	{
		if (*m_pParentState & CPlayer::STATE_IDLE)
		{
			bIsLoop = true;
			if (5.f <= *m_pParentIdleTime)
				iAnimIndex = CPlayer::IDLE_CASUAL;
			else
				iAnimIndex = CPlayer::IDLE_STATIC;
		}

		if (*m_pParentState & CPlayer::STATE_WALK)
		{
			bIsLoop = true;
			if (*m_pParentDir & CPlayer::DIR_N)
			{
				if (*m_pParentDir & CPlayer::DIR_E)
					iAnimIndex = CPlayer::WALK_NE;
				else if (*m_pParentDir & CPlayer::DIR_W)
					iAnimIndex = CPlayer::WALK_NW;
				else
					iAnimIndex = CPlayer::WALK_N;
			}
			else if (*m_pParentDir & CPlayer::DIR_S)
			{
				if (*m_pParentDir & CPlayer::DIR_E)
					iAnimIndex = CPlayer::WALK_SE;
				else if (*m_pParentDir & CPlayer::DIR_W)
					iAnimIndex = CPlayer::WALK_SW;
				else
					iAnimIndex = CPlayer::WALK_S;
			}
			else if (*m_pParentDir & CPlayer::DIR_E)
				iAnimIndex = CPlayer::WALK_E;
			else
				iAnimIndex = CPlayer::WALK_W;
		}

		if (*m_pParentState & CPlayer::STATE_RUN)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::RUN;
		}
	}	

	// 팔 애니메이션 재생
	if (*m_pParentState & CPlayer::STATE_RELOAD)
	{
		*m_pParentIdleTime = 0.f;
		iArmAnimIndex = CPlayer::RELOAD;
	}
	//if (*m_pParentState & CPlayer::STATE_FIRE)
	//{
	//	*m_pParentIdleTime = 0.f;
	//	iUpperAnimIndex = CPlayer::FIRE;
	//	iArmAnimIndex = CPlayer::FIRE;
	//}

	if (*m_pParentState & CPlayer::STATE_SWAP_TO_LAUNCHER)
	{
		*m_pParentIdleTime = 0.f;
		iArmAnimIndex = CPlayer::SWAP_RIFLE_TO_ROCKET;
	}
}

void CBody_Player::Set_LauncherAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop)
{
	*m_pParentIdleTime = 0.f;
	
	if (*m_pParentState & CPlayer::STATE_ROCKET_AIM)
	{
		if (*m_pParentState & CPlayer::STATE_ROCKET_IDLE)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::AIM_ROCKET_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_ROCKET_WALK)
		{
			bIsLoop = true;
			if (*m_pParentDir & CPlayer::DIR_N)
				iAnimIndex = CPlayer::AIM_ROCKET_WALK_N;
			else if (*m_pParentDir & CPlayer::DIR_S)
				iAnimIndex = CPlayer::AIM_ROCKET_WALK_S;
			else
				iAnimIndex = CPlayer::AIM_ROCKET_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_ROCKET_RUN)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::AIM_ROCKET_RUN_N;
		}
	}
	else
	{
		if (*m_pParentState & CPlayer::STATE_ROCKET_IDLE)
		{
			bIsLoop = true;
			iAnimIndex = CPlayer::ROCKET_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_ROCKET_WALK)
		{
			bIsLoop = true;
			if (*m_pParentDir & CPlayer::DIR_N)
				iAnimIndex = CPlayer::ROCKET_WALK_N;
			else if (*m_pParentDir & CPlayer::DIR_S)
				iAnimIndex = CPlayer::ROCKET_RUN_S;
			else
				iAnimIndex = CPlayer::ROCKET_IDLE;
		}

		if (*m_pParentState & CPlayer::STATE_ROCKET_RUN)
		{
			bIsLoop = true;
			if (*m_pParentDir & CPlayer::DIR_N)
				iAnimIndex = CPlayer::ROCKET_RUN_N;
			else
				iAnimIndex = CPlayer::ROCKET_IDLE;
		}
	}

	// 팔 애니메이션 재생
	if (*m_pParentState & CPlayer::STATE_ROCKET_RELOAD)
	{
		iArmAnimIndex = CPlayer::ROCKET_RELOAD;
	}

	if (*m_pParentState & CPlayer::STATE_SWAP_TO_RIFLE)
	{
		iArmAnimIndex = CPlayer::SWAP_ROCKET_TO_RIFLE;
	}
}

void CBody_Player::Set_ThrowAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop)
{
	if (*m_pParentState & CPlayer::STATE_READY_THROW)
	{
		iArmAnimIndex = CPlayer::GRENADE_PULLOUT;
		bIsArmLoop = false;
	}

	if (*m_pParentState & CPlayer::STATE_THROW)
	{
		iArmAnimIndex = CPlayer::GRENADE_THROW;
		bIsArmLoop = false;
	}
}

void CBody_Player::Set_KnockbackAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop)
{
	if (*m_pParentState & CPlayer::STATE_KNOCKBACK)
	{
		m_bIsArmStop = false;
		bIsLoop = false;
		iArmAnimIndex = -1;
		iUpperAnimIndex = -1;
		if (*m_pParentDir & CPlayer::DIR_N)
			iAnimIndex = CPlayer::KNOCKBACK_N;
		if (*m_pParentDir & CPlayer::DIR_S)
			iAnimIndex = CPlayer::KNOCKBACK_S;
		if (*m_pParentDir & CPlayer::DIR_W)
			iAnimIndex = CPlayer::KNOCKBACK_W;
		if (*m_pParentDir & CPlayer::DIR_E)
			iAnimIndex = CPlayer::KNOCKBACK_E;
	}

	if (*m_pParentState & CPlayer::STATE_KNOCKBACK_RECOVER)
	{
		bIsLoop = false;
		iArmAnimIndex = -1;
		iUpperAnimIndex = -1;
		iAnimIndex = CPlayer::KNOCKBACK_RECOVER;
	}
}

void CBody_Player::Set_StrataAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop)
{
	if (*m_pParentState & CPlayer::STATE_IDLE)
	{
		bIsLoop = true;
		if (5.f <= *m_pParentIdleTime)
			iAnimIndex = CPlayer::IDLE_CASUAL;
		else
			iAnimIndex = CPlayer::IDLE_STATIC;
	}
	if (*m_pParentState & CPlayer::STATE_WALK)
	{
		bIsLoop = true;
		if (*m_pParentDir & CPlayer::DIR_N)
		{
			if (*m_pParentDir & CPlayer::DIR_E)
				iAnimIndex = CPlayer::WALK_NE;
			else if (*m_pParentDir & CPlayer::DIR_W)
				iAnimIndex = CPlayer::WALK_NW;
			else
				iAnimIndex = CPlayer::WALK_N;
		}
		else if (*m_pParentDir & CPlayer::DIR_S)
		{
			if (*m_pParentDir & CPlayer::DIR_E)
				iAnimIndex = CPlayer::WALK_SE;
			else if (*m_pParentDir & CPlayer::DIR_W)
				iAnimIndex = CPlayer::WALK_SW;
			else
				iAnimIndex = CPlayer::WALK_S;
		}
		else if (*m_pParentDir & CPlayer::DIR_E)
			iAnimIndex = CPlayer::WALK_E;
		else
			iAnimIndex = CPlayer::WALK_W;
	}
	if (*m_pParentState & CPlayer::STATE_RUN)
	{
		bIsLoop = true;
		iAnimIndex = CPlayer::RUN;
	}

	if (*m_pParentState & CPlayer::STATE_ROCKET_IDLE)
	{
		bIsLoop = true;
		iAnimIndex = CPlayer::ROCKET_IDLE;
	}
	if (*m_pParentState & CPlayer::STATE_ROCKET_WALK)
	{
		bIsLoop = true;
		if (*m_pParentDir & CPlayer::DIR_N)
			iAnimIndex = CPlayer::ROCKET_WALK_N;
		else if (*m_pParentDir & CPlayer::DIR_S)
			iAnimIndex = CPlayer::ROCKET_RUN_S;
		else
			iAnimIndex = CPlayer::ROCKET_IDLE;
	}
	if (*m_pParentState & CPlayer::STATE_ROCKET_RUN)
	{
		bIsLoop = true;
		if (*m_pParentDir & CPlayer::DIR_N)
			iAnimIndex = CPlayer::ROCKET_RUN_N;
		else
			iAnimIndex = CPlayer::ROCKET_IDLE;
	}

	if (*m_pParentState & CPlayer::STATE_STRATAGEM_READY)
	{
		iArmAnimIndex = CPlayer::GRENADE_PULLOUT;
		bIsArmLoop = false;
	}
	else
	{
		iArmAnimIndex = CPlayer::GRENADE_THROW;
		bIsArmLoop = false;
		m_bIsArmStop = false;
	}
}

void CBody_Player::Set_MotionEnd()
{
	if (*m_pParentState & CPlayer::STATE_STRATAGEM_THROW)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			*m_pParentState ^= CPlayer::STATE_STRATAGEM_THROW;
		}
	}
	if (*m_pParentState & CPlayer::STATE_RELOAD)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			static_cast<CPlayer*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front())->Reload();
			*m_pParentState ^= CPlayer::STATE_RELOAD;
		}
	}

	if (*m_pParentState & CPlayer::STATE_ROCKET_RELOAD)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			static_cast<CPlayer*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front())->Reload();
			*m_pParentState ^= CPlayer::STATE_ROCKET_RELOAD;
		}
	}

	if (*m_pParentState & CPlayer::STATE_SWAP_TO_LAUNCHER)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			*m_pParentState ^= CPlayer::STATE_SWAP_TO_LAUNCHER;
		}
	}

	if (*m_pParentState & CPlayer::STATE_SWAP_TO_RIFLE)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			*m_pParentState ^= CPlayer::STATE_SWAP_TO_RIFLE;
		}
	}

	if (*m_pParentState & CPlayer::STATE_READY_THROW)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			*m_pParentState ^= CPlayer::STATE_READY_THROW;
			*m_pParentState |= CPlayer::STATE_THROW;
		}
	}
	else if (*m_pParentState & CPlayer::STATE_THROW)
	{
		if (false == m_pModelCom->IsPartAnimOn(CModel::PART_ARM_LEFT))
		{
			*m_pParentState ^= CPlayer::STATE_THROW;
		}
	}
}

HRESULT CBody_Player::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Jack"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	
	return S_OK;
}

HRESULT CBody_Player::Bind_ShaderResources()
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

CBody_Player * CBody_Player::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBody_Player*	pInstance = new CBody_Player(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CBody_Player");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBody_Player::Clone(void * pArg)
{
	CBody_Player*	pInstance = new CBody_Player(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CBody_Player");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBody_Player::Free()
{
	__super::Free();

	if (nullptr != m_pPlayerCamera)
		Safe_Release(m_pPlayerCamera);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	
}
