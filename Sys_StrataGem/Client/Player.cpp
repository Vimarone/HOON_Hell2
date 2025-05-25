#include "framework.h"
#include "Player.h"

#include "GameInstance.h"

#include "Body_Player.h"
#include "Weapon.h"
#include "Launcher.h"
#include "Grenade_Hand.h"
#include "Stratagem_Hand.h"
#include "EscapePod.h"

#include "EventObject.h"


#include "Camera_Follow.h"
#include "Particle_Explosion.h"
#include "PlayerUI.h"

#include "Count_Magazine.h"
#include "Count_Heal.h"
#include "Count_Grenade.h"
#include "Bar_Magazine.h"
#include "Bar_HP.h"

#include "GemBG.h"
#include "ScreenEffect.h"

CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CContainerObject{ pDevice, pContext }
{
	

}

CPlayer::CPlayer(const CPlayer & Prototype)
	: CContainerObject{ Prototype }
	, m_fSwapTimer{Prototype.m_fSwapTimer}
	, m_bSwapOnce{Prototype.m_bSwapOnce}
{
}

HRESULT CPlayer::Reload()
{
	if (nullptr == m_pCurrentMagazineInfo || nullptr == m_pCurrentBulletInfo)
		return E_FAIL;

	if (0 < m_pCurrentMagazineInfo->x)
	{
		m_pCurrentMagazineInfo->x--;
		m_pCurrentBulletInfo->x = m_pCurrentBulletInfo->y;
	}
	else
		return E_FAIL;
	return S_OK;
}

HRESULT CPlayer::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	CGameObject::GAMEOBJECT_DESC		Desc{};

	Desc.fSpeedPerSec = 6.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	m_fWalkSpeed = Desc.fSpeedPerSec;
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;	

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	m_iDir = DIR_N;
	// m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(400.f, 800.f, 400.f, 1.f));		// 시작 지점
	// m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(850.f, 800.f, 780.f, 1.f));		// 헬기장
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(400.f, 800.f, 400.f, 1.f));		// 몹 근처
	// m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(300.f, 800.f, 650.f, 1.f));		// 퀘스트 진행 장소
	// m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(850.f, 800.f, 450.f, 1.f));		// 헬기장 <> 타워 중간 지점
	m_pNavigationCom->Set_CurrentCellIndex(m_pNavigationCom->Get_CellIndexByPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	m_iState |= STATE_SPAWN;
	m_vCurrentHealInfo.y = 4;
	m_vCurrentHealInfo.x = m_vCurrentHealInfo.y;

	return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(300.f, 800.f, 650.f, 1.f));		// 퀘스트 진행 장소
		m_pNavigationCom->Set_CurrentCellIndex(m_pNavigationCom->Get_CellIndexByPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	}

	if (m_pGameInstance->Key_Down(DIK_F2))
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(400.f, 800.f, 400.f, 1.f));		// 몹 근처
		m_pNavigationCom->Set_CurrentCellIndex(m_pNavigationCom->Get_CellIndexByPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	}

	if (m_pGameInstance->Key_Down(DIK_F3))
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(850.f, 800.f, 780.f, 1.f));		// 헬기장
		m_pNavigationCom->Set_CurrentCellIndex(m_pNavigationCom->Get_CellIndexByPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	}
	__super::Priority_Update(fTimeDelta);
	if (m_pGameInstance->Key_Down(DIK_K))
		m_pStateCom->Set_Damaged(100);

	if (m_pGameInstance->Key_Down(DIK_LCONTROL))
	{
		if (!(m_iState & STATE_STRATAGEM))
		{
			m_iState |= STATE_STRATAGEM;
			m_pGameInstance->Check_Start();
		}
		else
		{
			m_iState ^= STATE_STRATAGEM;
			m_pGameInstance->Check_End();
		}			
	}

	if (m_iState & STATE_STRATAGEM)
		CheckGemInput();

	
	if (m_iState & STATE_SPAWN)
		Spawning(fTimeDelta);
	else if (m_iState & STATE_DIE)
		return;
	else
	{
		ControlAimUI(fTimeDelta);

		// 스왑 진행 시 시간 측정
		SwapTimer(fTimeDelta);

		// 넉백 진행 시 모든 입력 무시
		if (false == KnockBack(fTimeDelta))
		{
			_uint  iNewState = {}, iNewDir = {};
			if(!(m_iState & STATE_STRATAGEM))
				CheckInputMove(fTimeDelta, iNewState, iNewDir);
			m_iDir = iNewDir;
			CheckInputAttack();
			MoveToDir(fTimeDelta, m_iDir);
		}
	}
}

void CPlayer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if(!(m_iState & STATE_RUN) && !(m_iState & STATE_ROCKET_RUN))
		m_pStateCom->Update(fTimeDelta);

	m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

	if (!(m_iState & STATE_SPAWN))
	{
		_vector		vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, m_pNavigationCom->Compute_Height(vPosition) + 0.2f));
	}
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	if (true == g_bShutDown)
		return;

	__super::Late_Update(fTimeDelta);
	m_pStateCom->Late_Update(fTimeDelta);

#ifdef _DEBUG
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_NONBLEND, this);
	m_pGameInstance->Add_DebugComponent(m_pNavigationCom);
	m_pGameInstance->Add_DebugComponent(m_pColliderCom);	
#endif
}

HRESULT CPlayer::Render()
{
	return S_OK;
}

void CPlayer::Hit_Process(_fvector vHitOrigin, _uint iDamage)
{
	static_cast<CScreenEffect*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Effect"))->front())->Set_ScreenUI(1, 1.f, 1);
	if (m_iState & STATE_KNOCKBACK || m_iState & STATE_KNOCKBACK_RECOVER)
		return;

	// 넉백
	if (XMVectorGetX(vHitOrigin) != 0.f && iDamage >= 50)
	{
		m_fKnockbackTimeAcc = 0.f;
		if (!(m_iState & STATE_KNOCKBACK))
			m_iState = STATE_KNOCKBACK;

		m_pStateCom->Set_Damaged(iDamage);
		_vector vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_vector vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
		_vector vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		// 방향 계산
		_vector vKnockbackDir = XMVector3Normalize(vPosition - vHitOrigin);

		// 상대 위치 계산
		_float fLookDot = XMVectorGetX(XMVector3Dot(vLook, vKnockbackDir));
		_float fRightDot = XMVectorGetX(XMVector3Dot(vRight, vKnockbackDir));

		// 45도 이상: 전방
		if (fLookDot > 0.7f)
			m_iDir = DIR_N;
		// 135도 이상: 후방
		else if (fLookDot < -0.7f)
			m_iDir = DIR_S;
		// 45도 이상: 우측
		else if (fRightDot > 0.7f)
			m_iDir = DIR_E;
		// 135도 이상: 좌측
		else
			m_iDir = DIR_W;
	}
	else
		m_pStateCom->Set_Damaged(20);

	if (m_pStateCom->Get_HP()->x <= 0.f)
	{
		m_iState |= STATE_DIE;
		if (!(m_iState & STATE_KNOCKBACK))
		{
			m_iState |= STATE_KNOCKBACK;
			m_iDir = DIR_N;
		}
	}
}

void CPlayer::SupplyAll()
{
	m_pCurrentMagazineInfo->x = m_pCurrentMagazineInfo->y;
	m_pCurrentGrenadeInfo->x = m_pCurrentGrenadeInfo->y;
	m_vCurrentHealInfo.x = m_vCurrentHealInfo.y;	
}

_bool CPlayer::KnockBack(_float fTimeDelta)
{
	if (!(m_iState & STATE_KNOCKBACK) && !(m_iState & STATE_KNOCKBACK_RECOVER))
		return false;

	// 해당 방향으로 보내버리기
	if (m_iState & STATE_KNOCKBACK)
	{	
		_float fSpeed = m_fWalkSpeed * (4.f - m_fKnockbackTimeAcc);
		m_fKnockbackTimeAcc += fTimeDelta * 4.f;
		if (fSpeed < 0.f)
			fSpeed = 0.f;
		m_pTransformCom->Set_Speed(fSpeed);

		if (m_iDir & DIR_E)
			m_pTransformCom->Go_Right(fTimeDelta, m_pNavigationCom);
		if (m_iDir & DIR_N)
			m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		if (m_iDir & DIR_S)
			m_pTransformCom->Go_Backward(fTimeDelta, m_pNavigationCom);
		if (m_iDir & DIR_W)
			m_pTransformCom->Go_Left(fTimeDelta, m_pNavigationCom);
	}
	else
		QuickSwap(m_ePrevWeapon);

	return true;
}

HRESULT CPlayer::Ready_Components()
{		
	
	/* Com_Collider */
	CBounding_AABB::BOUNDING_AABB_DESC	ColliderDesc{};

	ColliderDesc.vExtents = _float3(0.5f, 1.6f, 0.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vExtents.y, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	/* Com_Navigation */
	CNavigation::NAVIGATION_DESC		Desc{};

	Desc.iCurrentCellIndex = 0;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &Desc)))
		return E_FAIL;

	CState::STATE_DESC					StateDesc{};
	StateDesc.iMaxHP = 1000;
	StateDesc.iMaxStamina = 500;
	StateDesc.iStaminaConsumeSpeed = 1;
	StateDesc.iAttack = 20;
	StateDesc.fExhaustedRecoverTime = 2.f;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_State"),
		TEXT("Com_State"), reinterpret_cast<CComponent**>(&m_pStateCom), &StateDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Ready_PartObjects()
{
	/* 바디를 만든다. */
	CBody_Player::BODY_PLAYER_DESC	BodyDesc{};
	BodyDesc.pParentState = &m_iState;
	BodyDesc.pParentDir = &m_iDir;
	BodyDesc.pParentIdleTime = &m_fIdleTime;
	BodyDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	BodyDesc.fSpeedPerSec = 0.f;
	BodyDesc.fRotationPerSec = 0.f;

	if (FAILED(__super::Add_PartObject(TEXT("Part_Body"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Body_Player"), &BodyDesc)))
		return E_FAIL;

	CModel* pBodyModelCom = static_cast<CModel*>(m_PartObjects[TEXT("Part_Body")]->Find_Component(TEXT("Com_Model")));
	pBodyModelCom->Set_PosBoneIndex(pBodyModelCom->Get_BoneIndex("jx_c_delta"));
	pBodyModelCom->Set_PropBoneIndex(pBodyModelCom->Get_BoneIndex("ja_c_propGun"));
	pBodyModelCom->Set_SpineInfo(
		_int2(pBodyModelCom->Get_BoneIndex("def_c_spineA"), pBodyModelCom->Get_BoneIndex("def_c_spineC")),
		_int2(AIM_UP, AIM_DOWN));
	pBodyModelCom->Set_UpperBoneIndex(
		_int2(pBodyModelCom->Get_BoneIndex("def_c_hip"), pBodyModelCom->Get_BoneIndex("def_fc_r_toplip_4_end_end")));
	pBodyModelCom->Set_ArmBoneIndex(
		_int2(pBodyModelCom->Get_BoneIndex("def_l_clav"), pBodyModelCom->Get_BoneIndex("def_l_finPinkyC_end_end")),
		_int2(pBodyModelCom->Get_BoneIndex("def_r_clav"), pBodyModelCom->Get_BoneIndex("def_r_finPinkyC_end_end")));


	/* 무기를 만든다. */
	CWeapon::WEAPON_DESC		WeaponDesc{};
	WeaponDesc.pSocketMatrix = pBodyModelCom->Get_BoneMatrix("ja_c_propGun");
	WeaponDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	WeaponDesc.pParentState = &m_iState;
	WeaponDesc.pShot = &m_bShot;
	WeaponDesc.fFireTime = 0.2f;
	WeaponDesc.iBulletCapacity = 45;
	WeaponDesc.iMagazineCapacity = 6;
	WeaponDesc.fSpeedPerSec = 0.f;
	WeaponDesc.fRotationPerSec = 0.f;

	if (FAILED(__super::Add_PartObject(TEXT("Part_Weapon"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Weapon"), &WeaponDesc)))
		return E_FAIL;

	CLauncher::LAUNCHER_DESC		LauncherDesc{};
	LauncherDesc.pSocketMatrix = pBodyModelCom->Get_BoneMatrix("ja_c_propGun");
	LauncherDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	LauncherDesc.pParentState = &m_iState;
	LauncherDesc.fFireTime = 2.f;
	LauncherDesc.iBulletCapacity = 1;
	LauncherDesc.iMagazineCapacity = 8;
	LauncherDesc.fSpeedPerSec = 0.f;
	LauncherDesc.fRotationPerSec = 0.f;

	if (FAILED(__super::Add_PartObject(TEXT("Part_Launcher"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Launcher"), &LauncherDesc)))
		return E_FAIL;
	m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(false);

	// ja_r_propHand
	CGrenade_Hand::GRENADE_DESC		GrenadeDesc{};
	GrenadeDesc.pSocketMatrix = pBodyModelCom->Get_BoneMatrix("ja_r_propHand");
	GrenadeDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	GrenadeDesc.pParentState = &m_iState;
	GrenadeDesc.iGrenadeCapacity = 4;

	if (FAILED(__super::Add_PartObject(TEXT("Part_Grenade"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Grenade_Hand"), &GrenadeDesc)))
		return E_FAIL;

	m_PartObjects.find(TEXT("Part_Grenade"))->second->Set_Active(false);

	CEscapePod::ESCAPEPOD_DESC		EscapePodDesc{};
	EscapePodDesc.pSocketMatrix = pBodyModelCom->Get_BoneMatrix("jx_c_delta");
	EscapePodDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	EscapePodDesc.pParentState = &m_iState;

	if (FAILED(__super::Add_PartObject(TEXT("Part_EscapePod"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_EscapePod"), &EscapePodDesc)))
		return E_FAIL;

	CStratagem_Hand::STRATA_DESC		StrataDesc{};
	StrataDesc.pSocketMatrix = pBodyModelCom->Get_BoneMatrix("ja_r_propHand");
	StrataDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	StrataDesc.pParentState = &m_iState;
	StrataDesc.pActivatedGem = &m_iActivatedGem;

	if (FAILED(__super::Add_PartObject(TEXT("Part_StrataGem"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Stratagem_Hand"), &StrataDesc)))
		return E_FAIL;

	m_PartObjects.find(TEXT("Part_StrataGem"))->second->Set_Active(false);

	// 에임 UI
	CUIObject::UIOBJECT_DESC UIDesc{};
	UIDesc.fSizeX = 114;
	UIDesc.fSizeY = 48;
	UIDesc.fX = g_iWinSizeX * 0.5f;
	UIDesc.fY = g_iWinSizeY * 0.5f;
	if (FAILED(__super::Add_UIObject(TEXT("UI_AimBG"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_AimBG"), &UIDesc)))
		return E_FAIL;
	m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_Active(false);

	UIDesc.fSizeX = 114;
	UIDesc.fSizeY = 48;
	UIDesc.fX = g_iWinSizeX * 0.5f;
	UIDesc.fY = g_iWinSizeY * 0.5f;
	if (FAILED(__super::Add_UIObject(TEXT("UI_AimDot"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_AimDot"), &UIDesc)))
		return E_FAIL;
	m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_Active(false);

	UIDesc.fSizeX = 114;
	UIDesc.fSizeY = 48;
	UIDesc.fX = g_iWinSizeX * 0.5f;
	UIDesc.fY = g_iWinSizeY * 0.5f;
	if (FAILED(__super::Add_UIObject(TEXT("UI_AimCross"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_AimCross"), &UIDesc)))
		return E_FAIL;
	m_UIObjects.find(TEXT("UI_AimCross"))->second->Set_Active(false);


	// 백그라운드 UI
	CPlayerUI::PLAYER_UI_DESC  BackgroundDesc{};
	BackgroundDesc.fSizeX = 360;
	BackgroundDesc.fSizeY = 90;
	BackgroundDesc.fX = BackgroundDesc.fSizeX * 0.55f;
	BackgroundDesc.fY = g_iWinSizeY * 0.9f;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Background"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIPlayer_Background"), &BackgroundDesc)))
		return E_FAIL;

	
	UIDesc.fSizeX = 40;
	UIDesc.fSizeY = BackgroundDesc.fSizeY * 1.2f;
	UIDesc.fX = UIDesc.fSizeX * 0.7f;
	UIDesc.fY = g_iWinSizeY * 0.9f;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Divider_Left"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIDivider_Left"), &UIDesc)))
		return E_FAIL;


	// 무기 UI
	_float2 vWeaponLine = { BackgroundDesc.fX - BackgroundDesc.fSizeX * 0.4f, BackgroundDesc.fY - BackgroundDesc.fSizeY * 0.2f };
	_float2 vHPLine = { BackgroundDesc.fX - BackgroundDesc.fSizeX * 0.15f, BackgroundDesc.fY + BackgroundDesc.fSizeY * 0.2f };
	_float2 vStaminaLine = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.9f };
	CUICount::UICOUNT_DESC CountDesc{};
	m_pCurrentMagazineInfo = static_cast<CWeapon*>(m_PartObjects.find(TEXT("Part_Weapon"))->second)->Get_MagazineInfo();
	CountDesc.pCountInfo = m_pCurrentMagazineInfo;
	CountDesc.fSizeX = 30;
	CountDesc.fSizeY = 30;
	CountDesc.fX = vWeaponLine.x;
	CountDesc.fY = vWeaponLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UICount_Magazine"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UICount_Magazine"), &CountDesc)))
		return E_FAIL;

	// 수류탄 UI
	m_pCurrentGrenadeInfo = static_cast<CGrenade_Hand*>(m_PartObjects.find(TEXT("Part_Grenade"))->second)->Get_CountInfo();
	CountDesc.pCountInfo = m_pCurrentGrenadeInfo;
	CountDesc.fX = vWeaponLine.x + CountDesc.fSizeX * 7.2f;
	CountDesc.fY = vWeaponLine.y;
	CountDesc.fSizeX = 22;
	CountDesc.fSizeY = 30;
	if (FAILED(__super::Add_UIObject(TEXT("UICount_Grenade"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UICount_Grenade"), &CountDesc)))
		return E_FAIL;

	// 디바이더
	UIDesc.fSizeX = 36;
	UIDesc.fSizeY = 60;
	UIDesc.fX = CountDesc.fX - CountDesc.fSizeX * 1.6f;
	UIDesc.fY = vWeaponLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_DividerMG"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIDivider_Center"), &UIDesc)))
		return E_FAIL;

	// 힐팩 UI
	CountDesc.pCountInfo = &m_vCurrentHealInfo;
	CountDesc.fX = vHPLine.x + CountDesc.fSizeX * 6.5f;
	CountDesc.fY = vHPLine.y;
	CountDesc.fSizeX = 64;
	CountDesc.fSizeY = 30;
	if (FAILED(__super::Add_UIObject(TEXT("UICount_Heal"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UICount_Heal"), &CountDesc)))
		return E_FAIL;

	CUIBar::UIBAR_DESC		BarDesc{};
	m_pCurrentBulletInfo = static_cast<CWeapon*>(m_PartObjects.find(TEXT("Part_Weapon"))->second)->Get_BulletInfo();
	BarDesc.pBarInfo = m_pCurrentBulletInfo;
	BarDesc.fSizeX = 30;
	BarDesc.fSizeY = 30;
	BarDesc.fX = vWeaponLine.x;
	BarDesc.fY = vWeaponLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UIBar_Magazine"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIBar_Magazine"), &BarDesc)))
		return E_FAIL;

	UIDesc.fSizeX = 100;
	UIDesc.fSizeY = 30;
	UIDesc.fX = vWeaponLine.x + BarDesc.fSizeX * 2.f + UIDesc.fSizeX * 0.5f;
	UIDesc.fY = vWeaponLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Weapon_Icon"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIWeapon_Icon"), &UIDesc)))
		return E_FAIL;


	// HP UI
	UIDesc.fSizeX = BackgroundDesc.fSizeX * 0.55f;
	UIDesc.fSizeY = 22;
	UIDesc.fX = vHPLine.x;
	UIDesc.fY = vHPLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Divider_HP"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIDivider_HP"), &UIDesc)))
		return E_FAIL;

	UIDesc.fSizeX = BackgroundDesc.fSizeX * 0.5f;
	UIDesc.fSizeY = 8;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Background_HP"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Background_HP"), &UIDesc)))
		return E_FAIL;

	m_pStateCom->Set_Damaged(20);
	BarDesc.pBarInfo = m_pStateCom->Get_HP();
	BarDesc.fSizeX = BackgroundDesc.fSizeX * 0.5f;
	BarDesc.fSizeY = 8;
	BarDesc.fX = vHPLine.x;
	BarDesc.fY = vHPLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Bar_HP"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIBar_HP"), &BarDesc)))
		return E_FAIL;

	// 스태미너 UI
	UIDesc.fSizeX = 360;
	UIDesc.fSizeY = 12;
	UIDesc.fX = vStaminaLine.x;
	UIDesc.fY = vStaminaLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Divider_Stamina"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIDivider_Stamina"), &UIDesc)))
		return E_FAIL;

	UIDesc.fSizeY = 8;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Background_Stamina"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Background_Stamina"), &UIDesc)))
		return E_FAIL;

	BarDesc.pBarInfo = m_pStateCom->Get_Stamina();
	BarDesc.fSizeX = 360;
	BarDesc.fSizeY = 8;
	BarDesc.fX = vStaminaLine.x;
	BarDesc.fY = vStaminaLine.y;
	if (FAILED(__super::Add_UIObject(TEXT("UI_Bar_Stamina"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIBar_Stamina"), &BarDesc)))
		return E_FAIL;

	// 스트라타젬 UI
	CGemBG::GEMBG_DESC GemDesc{};
	GemDesc.fSizeX = 270;
	GemDesc.fSizeY = 50 * m_pGameInstance->Get_GemCount();
 	GemDesc.fX = 30 + GemDesc.fSizeX * 0.5f;
	GemDesc.fY = 50 + GemDesc.fSizeY * 0.5f;
	GemDesc.pPlayerState = &m_iState;
	
	if (FAILED(__super::Add_UIObject(TEXT("UI_StrataGem"), LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemBG"), &GemDesc)))
		return E_FAIL;

	return S_OK;
}

void CPlayer::CheckInputMove(_float fTimeDelta, _uint& iNewState, _uint& iNewDir)
{
	if (GetKeyState('S') & 0x8000)
	{
		iNewDir |= DIR_S;
		iNewState |= STATE_WALK;
	}
	if (GetKeyState('A') & 0x8000)
	{
		iNewDir |= DIR_W;
		iNewState |= STATE_WALK;
	}
	if (GetKeyState('D') & 0x8000)
	{
		iNewDir |= DIR_E;
		iNewState |= STATE_WALK;
	}
	if (GetKeyState('W') & 0x8000)
	{
		iNewDir |= DIR_N;
		iNewState |= STATE_WALK;
	}

	// 이동 판단
	if (iNewState & STATE_WALK)
	{
		m_fIdleTime = 0.f;
		// idle 상태 제거
		if (m_iState & STATE_IDLE)
		{
			RotateToCameraDir();
			m_iState ^= STATE_IDLE;
		}
		if (m_iState & STATE_ROCKET_IDLE)
		{
			RotateToCameraDir();
			m_iState ^= STATE_ROCKET_IDLE;
		}

		// 달리기
		if ((GetKeyState(VK_SHIFT) & 0x8000) && true == m_pStateCom->Use_Stamina())
		{
			if (m_iState & STATE_WALK)
				m_iState ^= STATE_WALK;
			m_iState |= STATE_RUN;
			if (iNewDir & DIR_E)
				iNewDir ^= DIR_E;
			if (iNewDir & DIR_S)
				iNewDir ^= DIR_S;
			if (iNewDir & DIR_W)
				iNewDir ^= DIR_W;
			if (!(iNewDir & DIR_N))
				iNewDir |= DIR_N;
		}
		// 걷기
		else
		{
			if (m_iState & STATE_RUN)
				m_iState ^= STATE_RUN;
			m_iState |= STATE_WALK;
		}
	}
	else
	{
		m_fIdleTime += fTimeDelta;

		m_iState |= STATE_IDLE;
		if (m_iState & STATE_WALK)
			m_iState ^= STATE_WALK;
		if (m_iState & STATE_RUN)
			m_iState ^= STATE_RUN;
	}
}

void CPlayer::CheckInputAttack()
{
	if (!(m_iState & STATE_RELOAD) && !(m_iState & STATE_FIRE)
		&& !(m_iState & STATE_SWAP_TO_RIFLE) && !(m_iState & STATE_SWAP_TO_LAUNCHER)
		&& !(m_iState & STATE_READY_THROW) && !(m_iState & STATE_THROW)
		&& !(m_iState & STATE_STRATAGEM_READY) && !(m_iState & STATE_STRATAGEM_THROW)
			&& !(m_iState & STATE_STRATAGEM))
	{
		if (m_pGameInstance->Key_Down(DIK_R))
		{
			if (0 < m_pCurrentMagazineInfo->x)
			{
				m_iState |= STATE_RELOAD;
				if (m_iState & STATE_AIM)
					m_iState ^= STATE_AIM;
				if (m_iState & STATE_ROCKET_AIM)
					m_iState ^= STATE_ROCKET_AIM;
			}
			else
			{
				// 리로드 실패 사운드 삽입
			}
		}
		else if (m_pGameInstance->Button_Down(MOUSEKEYSTATE::DIM_LB))
		{
			RotateToCameraDir();
			m_iState |= STATE_FIRE;
		}
		else if (m_pGameInstance->Key_Down(DIK_1))
		{
			if (RIFLE != m_eCurrentWeapon)
			{
				SwapWeapon(RIFLE);
				m_iState |= STATE_SWAP_TO_RIFLE;
			}
		}
		else if (m_pGameInstance->Key_Down(DIK_2))
		{
			if (LAUNCHER != m_eCurrentWeapon)
			{
				SwapWeapon(LAUNCHER);
				m_iState |= STATE_SWAP_TO_LAUNCHER;
			}
		}
		else if (m_pGameInstance->Key_Down(DIK_4) || m_pGameInstance->Key_Down(DIK_V))
		{			
			if (m_vCurrentHealInfo.x > 0)
			{
				m_vCurrentHealInfo.x--;
				m_pStateCom->Use_Healpack();
			}			
		}
		else if (m_pGameInstance->Key_Down(DIK_G))
		{
			if (m_pCurrentGrenadeInfo->x > 0)
			{
				SwapWeapon(GRENADE);
				m_iState |= STATE_READY_THROW;
			}
		}
	}

	if (m_iState & STATE_THROW)
	{
		if (m_eSwapDestWeapon != m_ePrevWeapon && m_eCurrentWeapon != m_ePrevWeapon)
		{
			SwapWeapon(m_ePrevWeapon);
			m_fSwapTimer = 0.f;
		}
	}

	if (m_pGameInstance->Button_Down(MOUSEKEYSTATE::DIM_RB) || m_iState & STATE_ROCKET_AIM)
	{
		m_iState |= STATE_AIM;
		if (m_iState & STATE_ROCKET_AIM)
			m_iState ^= STATE_ROCKET_AIM;
		else
			RotateToCameraDir();
	}

	if (m_iState & STATE_AIM)
	{
		if (m_pGameInstance->Button_Up(MOUSEKEYSTATE::DIM_RB))
			m_iState ^= STATE_AIM;

		if (m_iState & STATE_RUN)
		{
			m_iState ^= STATE_RUN;
			m_iState |= STATE_WALK;
		}
	}

	if (m_iState & STATE_FIRE)
	{
		if (m_pGameInstance->Button_Up(MOUSEKEYSTATE::DIM_LB))
			m_iState ^= STATE_FIRE;
	}

	if (LAUNCHER == m_eCurrentWeapon)
		SetLauncherState();
}

void CPlayer::MoveToDir(_float fTimeDelta, _uint iDir)
{
	if (RIFLE == m_eCurrentWeapon)
	{
		if (m_iState & STATE_AIM)
		{
			if (iDir & DIR_N)
			{
				m_pTransformCom->Set_Speed(m_fWalkSpeed * 0.6f);
				m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
			}
			if (iDir & DIR_S)
			{
				m_pTransformCom->Set_Speed(m_fWalkSpeed * 0.6f);
				m_pTransformCom->Go_Backward(fTimeDelta, m_pNavigationCom);
			}
		}
		else
		{
			_uint iDirNum = {};
			if (iDir & DIR_E)
			{
				iDirNum++;
				m_pTransformCom->Go_Right(fTimeDelta, m_pNavigationCom);
			}
			if (iDir & DIR_N)
			{
				iDirNum++;
				m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
			}
			if (iDir & DIR_S)
			{
				iDirNum++;
				m_pTransformCom->Go_Backward(fTimeDelta, m_pNavigationCom);
			}
			if (iDir & DIR_W)
			{
				iDirNum++;
				m_pTransformCom->Go_Left(fTimeDelta, m_pNavigationCom);
			}
			if (m_iState & STATE_WALK)
			{
				if (2 == iDirNum)
					m_pTransformCom->Set_Speed(m_fWalkSpeed / sqrt(iDirNum));
				else
					m_pTransformCom->Set_Speed(m_fWalkSpeed);
			}
			else if (m_iState & STATE_RUN)
				m_pTransformCom->Set_Speed(m_fWalkSpeed * 2.f);			
		}
	}
	else
	{
		if (iDir & DIR_N)
			m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		if (iDir & DIR_S && !(m_iState & STATE_ROCKET_RUN))
			m_pTransformCom->Go_Backward(fTimeDelta, m_pNavigationCom);

		if (m_iState & STATE_ROCKET_WALK)
			m_pTransformCom->Set_Speed(m_fWalkSpeed * 0.3f);
		else if(m_iState & STATE_ROCKET_RUN)
			m_pTransformCom->Set_Speed(m_fWalkSpeed * 1.5f);
	}
}

void CPlayer::RotateToCameraDir()
{		
	// 카메라 방향으로 Turn
	_matrix MyWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());
	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, MyWorld);

	_matrix CameraWorld = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);
	_vector vCameraLook = CameraWorld.r[CTransform::STATE_LOOK];
	_float3 vCameraLookF{};
	XMStoreFloat3(&vCameraLookF, vCameraLook);
	_float fYaw = atan2f(vCameraLookF.x, vCameraLookF.z);
	_vector vNewRotation = XMQuaternionRotationRollPitchYaw(0.f, fYaw, 0.f);
	m_pTransformCom->Rotation(vNewRotation);

	// 카메라 보간용
	if (0.95f >= XMVectorGetX(XMQuaternionDot(XMVector4Normalize(vRotation), XMVector4Normalize(vNewRotation))))
		m_bTurnOn = true;
}

void CPlayer::QuickSwap(WEAPON eDestWeapon)
{
	if (GRENADE == m_eCurrentWeapon || STRATAGEM == m_eCurrentWeapon)
	{
		switch (m_eCurrentWeapon)
		{
		case GRENADE:
			m_PartObjects.find(TEXT("Part_Grenade"))->second->Set_Active(false);
			break;
		case STRATAGEM:
			m_PartObjects.find(TEXT("Part_StrataGem"))->second->Set_Active(false);
			break;
		default:
			break;
		}

		switch (eDestWeapon)
		{
		case RIFLE:
			m_PartObjects.find(TEXT("Part_Weapon"))->second->Set_Active(true);
			break;
		case LAUNCHER:
			m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(true);
			break;
		default:
			break;
		}

		m_ePrevWeapon = m_eCurrentWeapon;
		ClearPrevSwapState(m_ePrevWeapon);
		m_eSwapDestWeapon = eDestWeapon;
		m_eCurrentWeapon = eDestWeapon;
	}
}

void CPlayer::SwapTimer(_float fTimeDelta)
{
	if (m_iState & STATE_SWAP_TO_RIFLE || m_iState & STATE_SWAP_TO_LAUNCHER
		|| m_iState & STATE_READY_THROW || m_iState & STATE_THROW
		|| m_iState & STATE_STRATAGEM_READY || m_iState & STATE_STRATAGEM_THROW)
		m_fSwapTimer += fTimeDelta;
	else
		m_fSwapTimer = 0.f;

	_float fInactiveTime = 0.f;
	_float fActiveTime = 0.f;
	if (m_iState & STATE_SWAP_TO_RIFLE)
	{
		if (m_eSwapDestWeapon != m_eCurrentWeapon)
		{
			fInactiveTime = 0.55f;
			fActiveTime = 0.76f;
			if (fInactiveTime <= m_fSwapTimer && m_fSwapTimer < fActiveTime && false == m_bSwapOnce)
			{
				m_bSwapOnce = true;
				m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(false);
			}
			if (fActiveTime <= m_fSwapTimer && m_eCurrentWeapon != m_eSwapDestWeapon)
			{
				m_bSwapOnce = false;
				m_PartObjects.find(TEXT("Part_Weapon"))->second->Set_Active(true);

				// 명목 스왑
				ClearPrevSwapState(m_eCurrentWeapon);
				m_eCurrentWeapon = m_eSwapDestWeapon;
			}
			if ((fInactiveTime + fActiveTime) * 0.5f <= m_fSwapTimer)
			{
				CPartObject* pCurrObject = m_PartObjects.find(TEXT("Part_Launcher"))->second;
				CPartObject* pDestObject = m_PartObjects.find(TEXT("Part_Weapon"))->second;
				CWeapon* pWeapon = nullptr;
				_uint2* pCountInfo = nullptr;
				_int2* pBarInfo = nullptr;

				pWeapon = static_cast<CWeapon*>(pDestObject);
				pCountInfo = pWeapon->Get_MagazineInfo();
				pBarInfo = pWeapon->Get_BulletInfo();

				// UICount
				CUIObject* pUIObject = m_UIObjects.find(TEXT("UICount_Magazine"))->second;
				// Texture 변경
				pUIObject->Set_TextureNum(m_eSwapDestWeapon);
				// UICount_Magazine -> Set_CountInfo 변경
				CUICount* pUICount = static_cast<CUICount*>(pUIObject);
				pUICount->Set_CountInfo(pCountInfo);
				// m_pCurrentMagazineInfo 변경
				m_pCurrentMagazineInfo = pCountInfo;

				// UIBar
				pUIObject = m_UIObjects.find(TEXT("UIBar_Magazine"))->second;
				// Texture 변경
				pUIObject->Set_TextureNum(m_eSwapDestWeapon);
				// UIBar_Magazine -> Set_BarInfo 변경
				CUIBar* pUIBar = static_cast<CUIBar*>(pUIObject);
				pUIBar->Set_BarInfo(pBarInfo);
				// m_pCurrentBulletInfo 변경
				m_pCurrentBulletInfo = pBarInfo;

				// UIWeapon_Icon Texture 변경
				m_UIObjects.find(TEXT("UI_Weapon_Icon"))->second->Set_TextureNum(m_eSwapDestWeapon);
			}
		}
	}
	else if(m_iState & STATE_SWAP_TO_LAUNCHER)
	{
		if (m_eSwapDestWeapon != m_eCurrentWeapon)
		{
			fInactiveTime = 0.5f;
			fActiveTime = 0.66f;
			if (fInactiveTime <= m_fSwapTimer && m_fSwapTimer < fActiveTime && false == m_bSwapOnce)
			{
				m_bSwapOnce = true;
				m_PartObjects.find(TEXT("Part_Weapon"))->second->Set_Active(false);
			}
			if (fActiveTime <= m_fSwapTimer && m_eCurrentWeapon != m_eSwapDestWeapon)
			{
				m_bSwapOnce = false;
				m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(true);

				// 명목 스왑
				ClearPrevSwapState(m_eCurrentWeapon);
				m_eCurrentWeapon = m_eSwapDestWeapon;
			}
			if ((fInactiveTime + fActiveTime) * 0.5f <= m_fSwapTimer)
			{
				CPartObject* pCurrObject = m_PartObjects.find(TEXT("Part_Weapon"))->second;
				CPartObject* pDestObject = m_PartObjects.find(TEXT("Part_Launcher"))->second;
				CLauncher* pLauncher = nullptr;
				_uint2* pCountInfo = nullptr;
				_int2* pBarInfo = nullptr;

				// 목표(스왑) 무기 가져오기
				pLauncher = static_cast<CLauncher*>(pDestObject);
				pCountInfo = pLauncher->Get_MagazineInfo();
				pBarInfo = pLauncher->Get_BulletInfo();

				// UICount
				CUIObject* pUIObject = m_UIObjects.find(TEXT("UICount_Magazine"))->second;
				// Texture 변경
				pUIObject->Set_TextureNum(m_eSwapDestWeapon);
				// UICount_Magazine -> Set_CountInfo 변경
				CUICount* pUICount = static_cast<CUICount*>(pUIObject);
				pUICount->Set_CountInfo(pCountInfo);
				// m_pCurrentMagazineInfo 변경
				m_pCurrentMagazineInfo = pCountInfo;

				// UIBar
				pUIObject = m_UIObjects.find(TEXT("UIBar_Magazine"))->second;
				// Texture 변경
				pUIObject->Set_TextureNum(m_eSwapDestWeapon);
				// UIBar_Magazine -> Set_BarInfo 변경
				CUIBar* pUIBar = static_cast<CUIBar*>(pUIObject);
				pUIBar->Set_BarInfo(pBarInfo);
				// m_pCurrentBulletInfo 변경
				m_pCurrentBulletInfo = pBarInfo;

				// UIWeapon_Icon Texture 변경
				m_UIObjects.find(TEXT("UI_Weapon_Icon"))->second->Set_TextureNum(m_eSwapDestWeapon);
			}
		}
	}	
	else if (m_iState & STATE_READY_THROW || m_iState & STATE_STRATAGEM_READY)
	{
		if (m_eSwapDestWeapon != m_eCurrentWeapon)
		{
			fInactiveTime = 0.1f;
			fActiveTime = 0.15f;

			if (fInactiveTime <= m_fSwapTimer && m_fSwapTimer < fActiveTime && false == m_bSwapOnce)
			{
				m_bSwapOnce = true;
				switch (m_eCurrentWeapon)
				{
				case RIFLE:
					m_PartObjects.find(TEXT("Part_Weapon"))->second->Set_Active(false);
					break;
				case LAUNCHER:
					m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(false);
					break;
				default:
					break;
				}				
			}
			if (fActiveTime <= m_fSwapTimer && m_eCurrentWeapon != m_eSwapDestWeapon)
			{
				m_bSwapOnce = false;
				switch (m_eSwapDestWeapon)
				{
				case GRENADE:
					m_PartObjects.find(TEXT("Part_Grenade"))->second->Set_Active(true);
					break;
				case STRATAGEM:
					m_PartObjects.find(TEXT("Part_StrataGem"))->second->Set_Active(true);
					break;
				default:
					break;
				}			

				// 명목 스왑
				m_ePrevWeapon = m_eCurrentWeapon;
				ClearPrevSwapState(m_eCurrentWeapon);
				m_eCurrentWeapon = m_eSwapDestWeapon;
			}
		}
	}
	else if (m_iState & STATE_THROW || m_iState & STATE_STRATAGEM_THROW)
	{
		if (m_eSwapDestWeapon != m_eCurrentWeapon)
		{
			fInactiveTime = 0.1f;
			fActiveTime = 0.6f;
			if (STRATAGEM == m_eCurrentWeapon && LAUNCHER == m_eSwapDestWeapon)
				fActiveTime = 1.7f;
			
			if (fInactiveTime <= m_fSwapTimer && m_fSwapTimer < fActiveTime && false == m_bSwapOnce)
			{
				m_bSwapOnce = true;
				CPartObject* pCurrentWeapon = nullptr;
				_float4x4* pCombinedWorldMatrix = nullptr;
				_float fMouseYAcc = 0.f;
				switch (m_eCurrentWeapon)
				{
				case GRENADE:
					m_pCurrentGrenadeInfo->x--;
					pCurrentWeapon = m_PartObjects.find(TEXT("Part_Grenade"))->second;
					pCurrentWeapon->Set_Active(false);
					pCombinedWorldMatrix = pCurrentWeapon->Get_CombinedWorldMatrixPtr();
					_vector vStartPos = XMVectorSet(pCombinedWorldMatrix->_41, pCombinedWorldMatrix->_42, pCombinedWorldMatrix->_43, 1.f);
					_vector vUpDir = m_pTransformCom->Get_State(CTransform::STATE_UP);
					_vector vLookDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
					fMouseYAcc = static_cast<CCamera_Follow*>(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Camera"))->front())->Get_CalculatedMouseYAcc();
					_vector vTargetPos = XMVectorSetW(vStartPos + vLookDir - vUpDir * fMouseYAcc * 3.f, 1);
					if(fMouseYAcc < -0.05f)
						m_pGameInstance->Pool_Get(CPoolingObject::POOL_GRENADE, vTargetPos, vStartPos, 20.f, 10.f, true);
					else
						m_pGameInstance->Pool_Get(CPoolingObject::POOL_GRENADE, vTargetPos, vStartPos, 20.f, 10.f, false);

					break;
				case STRATAGEM:
					m_PartObjects.find(TEXT("Part_StrataGem"))->second->Set_Active(false);
					break;
				default:
					break;
				}
			}
			if (fActiveTime <= m_fSwapTimer && m_eCurrentWeapon != m_eSwapDestWeapon)
			{
				m_bSwapOnce = false;
				switch (m_eSwapDestWeapon)
				{
				case RIFLE:
					m_PartObjects.find(TEXT("Part_Weapon"))->second->Set_Active(true);
					break;
				case LAUNCHER:
					m_PartObjects.find(TEXT("Part_Launcher"))->second->Set_Active(true);
					break;
				default:
					break;
				}

				// 명목 스왑
				m_ePrevWeapon = m_eCurrentWeapon;
				ClearPrevSwapState(m_eCurrentWeapon);
				m_eCurrentWeapon = m_eSwapDestWeapon;
			}
		}
	}
}

void CPlayer::SetLauncherState()
{
	// 각 모션 변경
	if (m_iState & STATE_IDLE)
	{
		m_iState ^= STATE_IDLE;
		m_iState |= STATE_ROCKET_IDLE;
	}
	else if(m_iState & STATE_ROCKET_IDLE)
		m_iState ^= STATE_ROCKET_IDLE;

	if (m_iState & STATE_WALK)
	{
		m_iState ^= STATE_WALK;
		m_iState |= STATE_ROCKET_WALK;
	}
	else if (m_iState & STATE_ROCKET_WALK)
		m_iState ^= STATE_ROCKET_WALK;

	if (m_iState & STATE_RUN)
	{
		m_iState ^= STATE_RUN;
		m_iState |= STATE_ROCKET_RUN;
	}
	else if (m_iState & STATE_ROCKET_RUN)
		m_iState ^= STATE_ROCKET_RUN;

	if (m_iState & STATE_RELOAD)
	{
		m_iState ^= STATE_RELOAD;
		m_iState |= STATE_ROCKET_RELOAD;
	}

	if (m_iState & STATE_AIM)
	{
		m_iState ^= STATE_AIM;
		m_iState |= STATE_ROCKET_AIM;
	}
	else if (m_iState & STATE_ROCKET_AIM)
		m_iState ^= STATE_ROCKET_AIM;

	if (m_iState & STATE_FIRE)
	{
		m_iState ^= STATE_FIRE;
		m_iState |= STATE_ROCKET_FIRE;
	}
	else if (m_iState & STATE_ROCKET_FIRE)
		m_iState ^= STATE_ROCKET_FIRE;
}

void CPlayer::ClearPrevSwapState(WEAPON ePrevWeapon)
{
	switch (ePrevWeapon)
	{
	case RIFLE:
		for (size_t i = STATE_IDLE; i <= STATE_FIRE; i *= 2)
		{
			if (m_iState & i)
				m_iState ^= i;
		}
		break;
	case LAUNCHER:
		for (size_t i = STATE_ROCKET_IDLE; i <= STATE_ROCKET_FIRE; i *= 2)
		{
			if (m_iState & i)
				m_iState ^= i;
		}
		break;
	case GRENADE:
		for (size_t i = STATE_READY_THROW; i <= STATE_THROW; i *= 2)
		{
			if (m_iState & i)
				m_iState ^= i;
		}
		break;
	default:
		break;
	}
}

void CPlayer::ControlAimUI(_float fTimeDelta)
{
	if (m_iState & STATE_AIM)
	{
		m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_Active();
		m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_Active();

		if (true == m_bShot)
		{
			if (false == m_bShotTimerOn)
			{
				m_bShotTimerOn = true;
				m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_ShaderPass(1);
				m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_ShaderPass(2);
			}
			m_fShotTimer = 0.f;
			m_UIObjects.find(TEXT("UI_AimCross"))->second->Set_Active();
		}
		else
		{
			m_UIObjects.find(TEXT("UI_AimCross"))->second->Set_Active(false);
		}

		if (true == m_bShotTimerOn)
		{
			m_fShotTimer += fTimeDelta;
			if (m_fShotTimer >= 2.f)
			{
				m_bShotTimerOn = false;
				m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_ShaderPass(0);
				m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_ShaderPass(0);
			}
		}
	}
	else
	{
		if (true == m_bShotTimerOn)
		{
			m_bShotTimerOn = false;

			m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_Active(false);
			m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_Active(false);
			m_UIObjects.find(TEXT("UI_AimCross"))->second->Set_Active(false);

			m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_ShaderPass(0);
			m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_ShaderPass(0);
		}
		else
		{
			m_UIObjects.find(TEXT("UI_AimBG"))->second->Set_Active(false);
			m_UIObjects.find(TEXT("UI_AimDot"))->second->Set_Active(false);
			m_UIObjects.find(TEXT("UI_AimCross"))->second->Set_Active(false);
		}
	}
}

void CPlayer::Spawning(_float fTimeDelta)
{
	_float fNavY{}, fPosY{};
	_vector	vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	if (m_iState & STATE_DIE)
	{
		m_iState ^= STATE_DIE;
		m_pStateCom->Use_Healpack();
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, 800.f));
	}
	vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	fNavY = m_pNavigationCom->Compute_Height(vPosition);
	fPosY = XMVectorGetY(vPosition);
	if (fNavY <= fPosY)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, fPosY - fTimeDelta * m_fWalkSpeed * 20.f));
	else
	{
		m_pGameInstance->Effect_Get(CPoolEffect::SMOKE_SPREAD_SMALL, vPosition);
		m_pGameInstance->Effect_Get(CPoolEffect::SMOKE_SPREAD_MIDDLE, vPosition);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, fNavY));
		m_iState = STATE_IDLE;
		QuickSwap(RIFLE);
	}
}

void CPlayer::CheckGemInput()
{
	_int iCheckActivatedGem = -1;
	if (m_pGameInstance->Key_Down(DIK_W))
	{
		iCheckActivatedGem = m_pGameInstance->Check_Gems(ARROW_UP);
		m_pGameInstance->Play(L"Button6.wav", SOUND_SFX, 1.f);
		m_pGameInstance->Play(L"Button5.wav", SOUND_SFX, 1.f);
	}
	if (m_pGameInstance->Key_Down(DIK_A))
	{
		iCheckActivatedGem = m_pGameInstance->Check_Gems(ARROW_LEFT);
		m_pGameInstance->Play(L"Button6.wav", SOUND_SFX, 1.f);
		m_pGameInstance->Play(L"Button5.wav", SOUND_SFX, 1.f);
	}
	if (m_pGameInstance->Key_Down(DIK_D))
	{
		iCheckActivatedGem = m_pGameInstance->Check_Gems(ARROW_RIGHT);
		m_pGameInstance->Play(L"Button6.wav", SOUND_SFX, 1.f);
		m_pGameInstance->Play(L"Button5.wav", SOUND_SFX, 1.f);
	}
	if (m_pGameInstance->Key_Down(DIK_S))
	{
		iCheckActivatedGem = m_pGameInstance->Check_Gems(ARROW_DOWN);
		m_pGameInstance->Play(L"Button6.wav", SOUND_SFX, 1.f);
		m_pGameInstance->Play(L"Button5.wav", SOUND_SFX, 1.f);
	}

	if (-1 != iCheckActivatedGem)
	{
		m_pGameInstance->Check_End();
		m_iState ^= STATE_STRATAGEM;
		m_iState |= STATE_STRATAGEM_READY;
		m_iActivatedGem = iCheckActivatedGem;
		if (REINFORCEMENT == m_iActivatedGem)
		{
			CEventObject* pEventObject = m_pGameInstance->Get_EventObject(m_iActivatedGem);
			pEventObject->Execute_Event(m_iActivatedGem, XMVectorZero());
		}
		else
			SwapWeapon(STRATAGEM);
	}
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*	pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CPlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*	pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CPlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pStateCom);
}
