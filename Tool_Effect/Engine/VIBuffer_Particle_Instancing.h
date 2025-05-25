#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Particle_Instancing final : public CVIBuffer
{
public:
	enum PARTICLE_PATTERN { EXPLOSION, DROP, RISE, FRACTION, HOLD };

public:
	typedef struct particle_desc
	{
		_uint			iNumInstance = {};
		_float3			vRange = {};
		_float3			vCenter = {};
		_float2			vSize = {};
		_float2			vSpeed = {};
		_float3			vPivot = {};
		_float2			vLifeTime = {};
		_bool			isLoop = {};
	}PARTICLE_INSTANCING_DESC;

protected:
	CVIBuffer_Particle_Instancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Particle_Instancing(const CVIBuffer_Particle_Instancing& Prototype);
	virtual ~CVIBuffer_Particle_Instancing() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Bind_InputAssembler() override;
	virtual HRESULT Render() override;

public:
	_bool	Drop(_float fTimeDelta);
	_bool	Rise(_float fTimeDelta);
	_bool	Explosion(_float fTimeDelta);
	_bool	Fraction(_float fTimeDelta);
	_bool	Hold(_float fTimeDelta);

	void	Loop_End() { m_isLoopEnd = true; }
	void	Set_Pivot(_fvector vDirection) { XMStoreFloat3(&m_vPivot, vDirection); }
	void	Reset_Life();

#ifdef _DEBUG
public:
	void	Set_Desc(void* pArg);
#endif

protected:
	D3D11_BUFFER_DESC				m_InstanceBufferDesc = {};
	D3D11_SUBRESOURCE_DATA			m_InstanceInitialData = {};
	_uint							m_iInstanceVertexStride = {};
	_uint							m_iNumIndexPerInstance = {};
	_uint							m_iNumInstance = {};


	VTX_PARTICLE_INSTANCE*			m_pInstanceVertices = { nullptr };
	ID3D11Buffer*					m_pVBInstance = { nullptr };

	_float*							m_pSpeeds = { nullptr };

	_bool							m_isLoop = { false };
	_bool							m_isLoopEnd = { false };

	_float3							m_vPivot = {};


public:
	static CVIBuffer_Particle_Instancing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END