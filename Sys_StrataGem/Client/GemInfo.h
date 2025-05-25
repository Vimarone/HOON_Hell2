#pragma once
#include "Client_Defines.h"
#include "UIContainer.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemInfo final : public CUIContainer
{
public:
	typedef struct geminfo_desc : CUIObject::UIOBJECT_DESC
	{
		GEM_TYPE	eType = {};
		_float*		pParentPosX = { nullptr };
	}GEMINFO_DESC;
private:
	CGemInfo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemInfo(const CGemInfo& Prototype);
	virtual ~CGemInfo() = default;

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

	_tchar				m_szName[MAX_PATH] = {};
	_tchar				m_szCool[MAX_PATH] = {};

public:
	HRESULT Ready_Components();
	HRESULT Ready_GemSequence();

private:
	void	Set_Name(GEM_TYPE eType);

public:
	static CGemInfo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END