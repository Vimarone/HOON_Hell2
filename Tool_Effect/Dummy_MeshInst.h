#pragma once
#include "Client_Defines.h"
#include "MeshInstEffect.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Instancing_Mesh_SRT;
END

BEGIN(Client)

class CDummy_MeshInst final : public CMeshInstEffect
{
	using super = CMeshInstEffect;

private:
	CDummy_MeshInst(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDummy_MeshInst(const CDummy_MeshInst& Prototype);
	virtual ~CDummy_MeshInst() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Revealage() override;
	virtual HRESULT Render_Distortion() override;

	HRESULT Ready_Components(void* pArg);

private:
	CShader* m_pShaderCom{ nullptr };
	CVIBuffer_Instancing_Mesh_SRT* m_pVIBufferCom{ nullptr };

	CTexture* m_pTextureCom{ nullptr };
	CTexture* m_pMaskTextureCom{ nullptr };
	CTexture* m_pNoiseTextureCom{ nullptr };
	CTexture* m_pEmissiveTextureCom{ nullptr };


public:
	static CDummy_MeshInst* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CCloneable* Clone(void* pArg) override;
	virtual void Free() override;
};

END