#include "pch.h"
#include "Camera_Free.h"

#include "GameInstance.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CCamera_Free::CCamera_Free(const CCamera_Free& Prototype)
	: CCamera { Prototype }
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	super::DESC* pSuperDesc { nullptr };

	DESC* pDesc = static_cast<DESC*>(pArg);
	if (nullptr != pDesc)
	{
		pSuperDesc = &pDesc->CameraDesc;
		m_fMouseSensitivity = pDesc->fMouseSensitivity;
	}

	if (FAILED(super::Initialize(pSuperDesc)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(_float fTimeDelta)
{
	if (GetKeyState('W') & 0x8000)
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}

	if (GetKeyState('S') & 0x8000)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (GetKeyState('A') & 0x8000)
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}

	if (GetKeyState('D') & 0x8000)
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}

	if (GetKeyState('Q') & 0x8000)
	{
		m_pTransformCom->Go_Down(fTimeDelta);
	}

	if (GetKeyState('E') & 0x8000)
	{
		m_pTransformCom->Go_Up(fTimeDelta);
	}

	POINT ptMouse { g_iWindowSizeX >> 1, g_iWindowSizeY >> 1 };


	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::DIM_MB))
	{
		_int iMouseMove { 0 };

		if (iMouseMove = m_pGameInstance->Get_DIMouseMove(DIMS_X))
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * iMouseMove * m_fMouseSensitivity);

		if (iMouseMove = m_pGameInstance->Get_DIMouseMove(DIMS_Y))
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE::RIGHT), fTimeDelta * iMouseMove * m_fMouseSensitivity);

	}

	//if (m_pGameInstance->Get_DIKeyState(DIK_Z) & 0x80)
	//{
	//	ShowCursor(FALSE);
	//}
	//if (m_pGameInstance->Get_DIKeyState(DIK_X) & 0x80)
	//{
	//	ShowCursor(TRUE);
	//}


	super::Set_Transform_To_PipeLine();
}

void CCamera_Free::Update(_float fTimeDelta)
{
}

void CCamera_Free::Late_Update(_float fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
	// ? 절대 호출 안되긴 할듯 add render group 안해줘가지고 ㅎ
	return S_OK;
}

CCamera_Free* CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Free* pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CCamera_Free");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CCloneable* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Create : CCamera_Free");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	super::Free();
}
