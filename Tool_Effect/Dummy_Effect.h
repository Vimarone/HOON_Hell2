#pragma once

#include "Client_Defines.h"
#include "SpriteEffect.h"

BEGIN(Engine)
class CShader;
class CTexture;
END

BEGIN(Client)
class CDummy_Effect final : public CSpriteEffect
{
	using super = CSpriteEffect;

private:
	CDummy_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDummy_Effect(const CDummy_Effect& Prototype);
	virtual ~CDummy_Effect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Priority_Update(_float fTimeDelta) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Revealage() override;
	

	HRESULT			Ready_Components(const _wstring& strTextureTag, LEVELID eLevelID, CVIBuffer_Particle_Point::DESC& Desc);

private:
	CShader*					m_pShaderCom{ nullptr };
	CShader*					m_pGodRayShaderCom{ nullptr };

	CVIBuffer_Particle_Point*   m_pVIBufferCom{ nullptr };

	CTexture*					m_pTextureCom{ nullptr };
	CTexture*					m_pMaskTextureCom{ nullptr };
	CTexture*					m_pNoiseTextureCom{ nullptr };
	CTexture*					m_pEmissiveTextureCom{ nullptr };

public:
	static CDummy_Effect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CCloneable* Clone(void* pArg) override;
	virtual void Free() override;
};
END
