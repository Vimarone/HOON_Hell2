#pragma once
#include "Client_Defines.h"
#include "EventObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CWidow final : public CEventObject
{
private:
	CWidow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWidow(const CWidow& Prototype);
	virtual ~CWidow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Priority_Update(_float fTimeDelta) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual _bool	Event(_float fTimeDelta, _uint iEventType) override;

private:
	// 		ORBITAL_PRECISION
	_bool	Orbital_Precision();
	//		ORBITAL_380
	_bool	Orbital_380(_float fTimeDelta);

private:
	CShader*	m_pShaderCom = { nullptr };
	CModel*		m_pModelCom = { nullptr };

public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CWidow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END