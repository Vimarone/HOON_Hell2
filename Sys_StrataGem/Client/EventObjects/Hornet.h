#pragma once
#include "Client_Defines.h"
#include "EventObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CHornet final : public CEventObject
{
private:
	CHornet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHornet(const CHornet& Prototype);
	virtual ~CHornet() = default;

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
	void			Sortie(_float fTimeDelta);
	_bool			Strafe(_float fTimeDelta);
	_bool			HalfTon(_float fTimeDelta);

private:
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };

	_float4			m_vSortieDestination = {};
	_float			m_fSortieTimeAcc = {};
	_int 			m_iGenerateDistance = {};



public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CHornet* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END