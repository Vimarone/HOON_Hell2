#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	struct GAMEOBJECT_DESC : public CTransform::TRANSFORM_DESC
	{

	};

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	class CComponent*	Find_Component(const _wstring& strComponetTag);
	void				Set_ShaderPass(_uint iPass) { m_iShaderPass = iPass; }
	_uint				Get_ObjectType() const { return m_iObjectType; }
	_vector				Get_Position() { return m_pTransformCom->Get_State(CTransform::STATE_POSITION); }

	_bool				Get_Active() { return m_bActive; }
	void				Set_Active(_bool bActive = true) { m_bActive = bActive; }
	virtual	void		Hit_Process(_fvector vHitOrigin = {}, _uint iDamage = 0) {}

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

protected:
	_float4x4			m_ViewMatrix{}, m_ProjMatrix{};
	_float4				m_vCamPosition{};
	_uint				m_iShaderPass{};
	_uint				m_iObjectType{};
	_bool				m_bActive = { true };

protected:
	map<const _wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END