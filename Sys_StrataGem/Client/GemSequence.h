#pragma once
#include "Client_Defines.h"
#include "UIContainer.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CGemSequence final : public CUIContainer
{
public:
	typedef struct gemseq_desc : CUIObject::UIOBJECT_DESC
	{
		GEM_TYPE	eType{};
		_float*		pParentPosX = { nullptr };
	}GEMSEQ_DESC;
private:
	CGemSequence(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGemSequence(const CGemSequence& Prototype);
	virtual ~CGemSequence() = default;

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

public:
	HRESULT Ready_Components();
	HRESULT Ready_Sequence();

public:
	static CGemSequence* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END