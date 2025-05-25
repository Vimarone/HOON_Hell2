#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemIntroBG final : public CUIObject
{
	typedef struct gemBG_desc : CUIObject::UIOBJECT_DESC
	{
		_uint*		m_pPlayerState = { nullptr };
	}GEMBG_DESC;
private:
	CGemIntroBG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemIntroBG(const CGemIntroBG& Prototype);
	virtual ~CGemIntroBG() = default;

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

public:
	HRESULT Ready_Components();
	HRESULT	Ready_Gems();

public:
	static CGemIntroBG* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END