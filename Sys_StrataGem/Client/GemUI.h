#pragma once
#include "Client_Defines.h"
#include "UIContainer.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemUI final : public CUIContainer
{
public:
	typedef struct gemui_desc : CUIObject::UIOBJECT_DESC
	{
		GEM_TYPE	eType = {};
		_uint		iIndex = {};
		_float*		pParentPosX = { nullptr };
	}GEMUI_DESC;
private:
	CGemUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemUI(const CGemUI& Prototype);
	virtual ~CGemUI() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	HRESULT Ready_Components();
	HRESULT	Ready_Gems();

private:
	CShader*			m_pShaderCom = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

	_float*				m_pParentPosX = { nullptr };

	GEM_TYPE			m_eType{};

	

public:
	static CGemUI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END