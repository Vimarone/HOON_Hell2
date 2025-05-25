#pragma once
#include "Client_Defines.h"
#include "EventObject.h"

BEGIN(Engine)
class CNavigation;
class CCollider;
class CShader;
class CModel;
class CTexture;
END

BEGIN(Client)

class CSupply final : public CEventObject
{
private:
	CSupply(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSupply(const CSupply& Prototype);
	virtual ~CSupply() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual _bool Event(_float fTimeDelta, _uint iEventType) override;

private:
	CNavigation*	m_pNavigationCom = { nullptr };
	CCollider*		m_pColliderCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };

	_bool			m_bLanded = {};

	_float			m_fDissolveAmount = {};

public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CSupply* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END