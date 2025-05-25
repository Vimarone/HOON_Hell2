#pragma once

#include "Client_Defines.h"
#include "Level.h"

#include "EffectObject.h"

BEGIN(Engine)
class CTexture;
class CVIBuffer_Particle_Instancing;
class CGameObject;
class CModel;
END

BEGIN(Client)

class CLevel_EffectTool final : public CLevel
{
	using super = CLevel;

private:
	enum EFFECT_TEXTURE_TYPE
	{
		TEX_MASK, TEX_NOISE, TEX_DISSOLVE, TEX_EMISSIVE, TEX_DISTORTION, TEX_END
	};

private:
	CLevel_EffectTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_EffectTool() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Environment(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Effect(const _wstring& strLayerTag);
	HRESULT Ready_Lights();
	HRESULT	Ready_Images();
	HRESULT Ready_Meshes();

private:
	void	Save_Effect();
	void	Load_Effect();

	void	Save_Buffer_Data(HANDLE hFile, _ulong& dwByte);
	void    Save_InstancingBuffer_Data(HANDLE hFile, _ulong& dwByte);

	void	Adjust_Effect();
	//void	Add_To_Container();

private:
	//CGameObject*				m_pEffectContainer = { nullptr };
	//_uint						m_iEffectNum = {};

	_int						m_iImguiSelectedType = {};
	_bool						m_bBlendEnable = {};

	vector<_wstring>			m_ParticleFilePath;
	vector<CTexture*>			m_pParticleTextures;
	vector<_wstring>			m_ParticleTexPrototypeName;

	vector<_wstring>			m_SpriteFilePath;
	vector<CTexture*>			m_pSpriteTextures;
	vector<_wstring>			m_SpriteTexPrototypeName;

	_uint						m_iTextureIndex = {};

	/* 쉐이딩 옵션 */
	vector<_wstring>			m_OptionTextureFilePath[TEX_END];
	vector<CTexture*>			m_pOptionTextures[TEX_END];

	_uint						m_iShadingOption = {};
	_bool						m_bOptionEnable[TEX_END];
	_uint						m_iOptionTextureIndex[TEX_END];

	_float2						m_vDissolveStart = {}; // 나타날 때 디졸브, 사라질 때 디졸브


	/* 메쉬 전용 */
	vector<_wstring>			m_BaseTextureFilePath;
	vector<_wstring>			m_BaseTexPrototypeName;
	vector<CTexture*>			m_pBaseTextures;
	vector<_wstring>			m_ModelPrototypeName;
	vector<CModel*>				m_pModels;
	vector<_wstring>			m_TrailPrototypeName;
	vector<CModel*>				m_pTrails;

	_int						m_iSelectedMeshType = {};
	_bool						m_bTranslationAnim = {};
	_bool						m_bRotationAnim = {};
	_bool						m_bScalingAnim = {};

	_bool						m_bUVAnim = {};
	_float2						m_vUVStart = {};
	_float2						m_vUVSpeed = {}; // x, y 스피드
	_float2						m_vUVTileCount = {};

	/* 트레일 메쉬 전용 */
	_float						m_fMaxTrailLength = {};

	/* 메쉬 트랜스폼 */
	_float3						m_vMeshTransform = {};
	_float3						m_vMeshScale = {};


	_int						m_iRotationAxis = {};
	_float						m_fParticleRotation = {};
	_float						m_fParticleFinalRotation = {};

	_bool						m_bBillboard = {};

	/* 파티클 전용 */
	_int						m_iNumInstance = {};

	/* 공용 */
	_float						m_fActiveDelay = {};
	_int						m_iSelectedPattern = {};
	_bool						m_bIsLoop = {};
	_float3						m_vRange = {};
	_float3						m_vCenter = {};
	_float2						m_vSize = {};
	_float						m_fFinalSize = {};
	
	_float2						m_vSpeed = {};
	_float3						m_vPivot = {};
	_float2						m_vLifeTime = {};

	_float3						m_vMulColor = {};
	_float3						m_vAddColor = {};

	_int						m_iNumAtlas = {};		// 아틀라스 총 개수
	_int2						m_vNumAtlas = {};		// 아틀라스 가로, 세로 개수


public:
	static CLevel_EffectTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
