#pragma once
#include "GameObject.h"
#include "VIBuffer_Particle_Instancing.h"

BEGIN(Engine)

class ENGINE_DLL CPoolEffect abstract : public CGameObject
{
public:
	struct EFFECTPOOL_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		CVIBuffer_Particle_Instancing::PARTICLE_INSTANCING_DESC ParticleDesc{};

		_bool			bPlayUVAnim = {};
		_int			iPattern = {};
		_float			fFinalSize = {};
		_float3			vStartColor = {};
		_float3			vEndColor = {};
		_int			iNumAtlas = {};
		_int2			vNumAtlas = {};
		_float2			vUVInterval = {};
	};
public:
	enum EFFECT_TYPE 
	{ 
		EXPL_BIG, EXPL_MIDDLE, EXPL_SMALL, EXPL_TINY, EXPL_EFFECT,
		EXPL_SPREAD_BIG, EXPL_SPREAD_SMALL,
		SMOKE_BIG, SMOKE_SMALL, SMOKE_TINY,
		SMOKE_FOLLOW_BIG, SMOKE_FOLLOW_SMALL, SMOKE_FOLLOW_TINY,
		SMOKE_SPREAD_BIG, SMOKE_SPREAD_MIDDLE, SMOKE_SPREAD_SMALL, SMOKE_SPREAD_TINY,
		SCATTER_BIG, SCATTER_MIDDLE, SCATTER_SMALL, SCATTER_TINY,
		SPARK_SPRITE, SPARK_PARTICLE,
		BULLET_SPARK, BULLET_CIRCLE, BULLET_DUST, BULLET_TRAIL, BULLET_TRAIL_RED, BULLET_MUZZLE,
		DUST_SPREAD_BIG, DUST_SPREAD_SMALL, DUST_SPREAD_TINY,
		ASH_GRAY_ALOT, ASH_GRAY_FEW, 
		FIRE_ONCE, FIRE_ENGINE, FIRE_ENGINE_BIG,
		STRATA_LASER, STRATA_LASER_RED, STRATA_LASER_BLUE,
		BALL_LOOP, BLOOD_SPLATTER,
		EFFECT_END 
	};
protected:
	CPoolEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoolEffect(const CPoolEffect& Prototype);
	virtual ~CPoolEffect() = default;

public:
	_float Get_CamDistance() const { return m_fCamDistance; }

public:
	virtual HRESULT	Initialize_Prototype(void* pArg);
	virtual HRESULT	Initialize(void* pArg) override;
	virtual void	Priority_Update(_float fTimeDelta) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;

protected:
	void	Compute_CamDistance();

protected:
	CVIBuffer_Particle_Instancing*	m_pVIBufferCom = { nullptr };

	_bool		m_bPlayUVAnim = {};
	_int		m_iPattern = {};
	_float		m_fFinalSize = {};
	_float3		m_vStartColor = {};
	_float3		m_vEndColor = {};
	_int3		m_vNumAtlas = {};
	_float2		m_vUVInterval = {};

	_uint		m_iNumInstance = {};

	_bool		m_bLightAdd = {};
	_float		m_fCamDistance = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END