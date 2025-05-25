#pragma once
#include "Client_Defines.h"
#include "PoolingObject.h"

BEGIN(Engine)
class CPoolEffect;
class CCollider;
class CShader;
class CModel;
END

BEGIN(Client)

class CStratagem final : public CPoolingObject
{
private:
	CStratagem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStratagem(const CStratagem& Prototype);
	virtual ~CStratagem() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void Set_Explode(_fvector vPos) override;

public:
	void	Set_GemType(_uint iType) { m_iGemType = iType; }

private:
	void    Laser(_float fTimeDelta);

private:
	CPoolEffect*	m_pCurrentLaser = { nullptr };
	CCollider*		m_pColliderCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };

	_uint			m_iGemType = {};

	_bool			m_bMarkOn = { false };
	_float			m_fMarkTime = {};
	_float			m_fMarkTimeAcc = {};
	_float4			m_vGroundPosition = {};


public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CStratagem* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};


END