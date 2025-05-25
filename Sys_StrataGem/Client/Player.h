#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"

BEGIN(Engine)
class CCollider;
class CNavigation;
class CState;
END

BEGIN(Client)

class CPlayer final : public CContainerObject
{
public:
	enum STATE {
		STATE_IDLE = 0x00000001,
		STATE_WALK = 0x00000002,
		STATE_RUN = 0x00000004,
		STATE_RELOAD = 0x00000008,
		STATE_AIM = 0x00000010,
		STATE_FIRE = 0x00000020,
		STATE_SWAP_TO_RIFLE = 0x00000040,
		STATE_SWAP_TO_LAUNCHER = 0x00000080,
		STATE_ROCKET_IDLE = 0x00000100,
		STATE_ROCKET_WALK = 0x00000200,
		STATE_ROCKET_RUN = 0x00000400,
		STATE_ROCKET_RELOAD = 0x00000800,
		STATE_ROCKET_FIRE = 0x00001000,
		STATE_ROCKET_AIM = 0x00002000,
		STATE_READY_THROW = 0x00004000,
		STATE_THROW = 0x00008000,
		STATE_KNOCKBACK = 0x00010000,
		STATE_KNOCKBACK_RECOVER = 0x00020000,
		STATE_SPAWN = 0x00040000,
		STATE_STRATAGEM = 0x00080000,
		STATE_STRATAGEM_READY = 0x00100000,
		STATE_STRATAGEM_THROW = 0x00200000,
		STATE_DIE = 0x00400000,
	};
	enum DIRECTION
	{
		DIR_E = 0x00000001,
		DIR_N = 0x00000002,
		DIR_S = 0x00000004,
		DIR_W = 0x00000008
	};
	enum WEAPON
	{
		RIFLE, LAUNCHER, GRENADE, STRATAGEM, WEAPON_END
	};
	enum ANIMATION
	{
		IDLE_CASUAL,
		IDLE_STATIC,
		WALK_E,
		WALK_N,
		WALK_NE,
		WALK_NW,
		WALK_S,
		WALK_SE,
		WALK_SW,
		WALK_W,
		RUN,
		RELOAD,
		FIRE,
		SWAP_RIFLE_TO_ROCKET,
		SWAP_ROCKET_TO_RIFLE,
		ROCKET_IDLE,
		ROCKET_WALK_N,
		ROCKET_WALK_S,
		ROCKET_RUN_N,
		ROCKET_RUN_S,
		ROCKET_RELOAD,
		AIM_IDLE,
		AIM_WALK_N,
		AIM_WALK_S,
		AIM_RUN_N,
		AIM_RUN_S,
		AIM_ROCKET_IDLE,
		AIM_ROCKET_WALK_N,
		AIM_ROCKET_WALK_S,
		AIM_ROCKET_RUN_N,
		AIM_ROCKET_RUN_S,
		GRENADE_PULLOUT,
		GRENADE_THROW,
		AIM_UP,
		AIM_DOWN,
		KNOCKBACK_N,
		KNOCKBACK_S,
		KNOCKBACK_W,
		KNOCKBACK_E,
		KNOCKBACK_RECOVER,

		RADOLL = 99
	};

private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;

public:
	_uint*			Get_State_Ptr() { return &m_iState; }
	_bool*			Get_Turn_On() { return &m_bTurnOn; }
	HRESULT			Reload();
	_uint			Get_ActivatedGem() { return m_iActivatedGem; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Priority_Update(_float fTimeDelta) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual	void	Hit_Process(_fvector vHitOrigin = {}, _uint iDamage = 0) override;
	void			SwapWeapon(WEAPON eDestWeapon) 
	{ if (eDestWeapon == WEAPON_END) 
		m_eSwapDestWeapon = m_ePrevWeapon;
	else
		m_eSwapDestWeapon = eDestWeapon; }
	void			SupplyAll();

private:
	_bool	KnockBack(_float fTimeDelta);

public:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();

private:
	void	CheckInputMove(_float fTimeDelta, _uint& iNewState, _uint& iNewDir);
	void	CheckInputAttack();
	void	MoveToDir(_float fTimeDelta, _uint iDir);
	void	RotateToCameraDir();
	void	QuickSwap(WEAPON eDestWeapon);
	void	SwapTimer(_float fTimeDelta);
	void	SetLauncherState();
	void	ClearPrevSwapState(WEAPON ePrevWeapon);
	void	ControlAimUI(_float fTimeDelta);
	void	Spawning(_float fTimeDelta);
	void	CheckGemInput();

private:
	CCollider*					m_pColliderCom = { nullptr };
	CNavigation*				m_pNavigationCom = { nullptr };
	CState*						m_pStateCom = { nullptr };

	_uint						m_iState = {};
	_uint						m_iDir = {};

	_float						m_fIdleTime = {};

	_bool						m_bTurnOn = {};
	_float						m_fTurnRadian = {};
	_float						m_fCurrentRadian = {};

	_uint2*						m_pCurrentMagazineInfo = { nullptr };
	_int2*						m_pCurrentBulletInfo = { nullptr };
	_uint2*						m_pCurrentGrenadeInfo = { nullptr };

	_uint2						m_vCurrentHealInfo = {};
	_float2						m_vCurrentHPBarInfo = {};

	WEAPON						m_eCurrentWeapon = {};
	WEAPON						m_ePrevWeapon = {};
	WEAPON						m_eSwapDestWeapon = {};
	_float						m_fSwapTimer = {};
	_bool						m_bSwapOnce = { false };

	_uint						m_iActivatedGem = {};

	_float						m_fWalkSpeed = {};
	_float						m_fKnockbackTimeAcc = {};

	_bool						m_bShot = { false };
	_bool						m_bShotTimerOn = { false };
	_float						m_fShotTimer = {};

	GEM_TYPE					m_eGemType = {};

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END