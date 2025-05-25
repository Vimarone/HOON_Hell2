#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemArrow final : public CUIObject
{
public:
	typedef struct gemarrow_desc : CUIObject::UIOBJECT_DESC
	{
		GEM_TYPE	eType{};
		_uint		iIndex{};
		_uint		iArrow{};
		_float*		pParentPosX = { nullptr };
	}GEMARROW_DESC;
private:
	CGemArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemArrow(const CGemArrow& Prototype);
	virtual ~CGemArrow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*			m_pShaderCom = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

	GEM_TYPE			m_eType{};
	_float*				m_pParentPosX = { nullptr };
	_uint				m_iIndex{};
	_float				m_fX{}, m_fY{};
	_float				m_fSizeX{}, m_fSizeY{};

public:
	HRESULT Ready_Components();

public:
	static CGemArrow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END