#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)

class CTexture;
class CShader;
class CVIBuffer_Rect;

END

BEGIN(Client)

class CBackGround final : public CUIObject
{
public:
	typedef struct
	{
		CUIObject::UIOBJECT_DESC UIObjectDesc;
	} BACKGROUND_DESC;

private:
	CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBackGround(const CBackGround& Prototype);
	virtual ~CBackGround() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

	virtual HRESULT Render() override;

private:
	CTexture*			m_pTextureCom{ nullptr };
	CShader*			m_pShaderCom{ nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom{ nullptr };
	
private:
	HRESULT Ready_Components();

public:
	static CBackGround* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CCloneable* Clone(void* pArg) override;
	virtual void Free() override;

};

END
