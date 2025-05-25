#pragma once
#include "Client_Defines.h"
#include "EventObject.h"

BEGIN(Client)

class CReinforcement final : public CEventObject
{
private:
	CReinforcement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CReinforcement(const CReinforcement& Prototype);
	virtual ~CReinforcement() = default;

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
	_uint*			m_pPlayerState = { nullptr };

public:
	static CReinforcement* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END