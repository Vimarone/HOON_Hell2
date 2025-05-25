#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CStratagem_Hand final : public CPartObject
{
public:
	typedef struct strata_desc : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4*	pSocketMatrix = { nullptr };
		_uint*				pParentState = { nullptr };
		_uint*				pActivatedGem = { nullptr };
	}STRATA_DESC;
private:
	CStratagem_Hand(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStratagem_Hand(const CStratagem_Hand& Prototype);
	virtual ~CStratagem_Hand() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Priority_Update(_float fTimeDelta) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*			m_pShaderCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };

	const _float4x4*	m_pSocketMatrix = { nullptr };
	_uint*				m_pParentState = { nullptr };
	_uint*				m_pActivatedGem = { nullptr };

public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CStratagem_Hand* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END