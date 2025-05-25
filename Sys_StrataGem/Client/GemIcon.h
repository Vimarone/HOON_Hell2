#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemIcon final : public CUIObject
{
public:
	typedef struct gemicon_desc : CUIObject::UIOBJECT_DESC
	{
		GEM_TYPE	eType = {};
		_float*		pParentPosX = { nullptr };
	}GEMICON_DESC;
private:
	CGemIcon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemIcon(const CGemIcon& Prototype);
	virtual ~CGemIcon() = default;

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
	_float				m_fX{}, m_fY{};
	_float				m_fSizeX{}, m_fSizeY{};

public:
	HRESULT Ready_Components();

public:
	static CGemIcon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END