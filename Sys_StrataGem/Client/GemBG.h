#pragma once
#include "Client_Defines.h"
#include "UIContainer.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemBG final : public CUIContainer
{
public:
	typedef struct gemBG_desc : CUIObject::UIOBJECT_DESC
	{
		_uint*		pPlayerState = { nullptr };
	}GEMBG_DESC;
private:
	CGemBG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemBG(const CGemBG& Prototype);
	virtual ~CGemBG() = default;

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

	_uint*				m_pPlayerState = { nullptr };
	_bool				m_bActive = { false };
	_float				m_fInitialX{}, m_fInActiveX{};

public:
	HRESULT Ready_Components();
	HRESULT	Ready_Gems();

private:
	void	InOutAnimation(_float fTimeDelta);

public:
	static CGemBG* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END