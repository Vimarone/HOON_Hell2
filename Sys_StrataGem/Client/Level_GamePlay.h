#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "PoolEffect.h"

BEGIN(Client)

class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Quests(const _tchar* pLayerTag);
	HRESULT Ready_Gems(const _tchar* pLayerTag);
	HRESULT Ready_Layer_BackGround(const _tchar* pLayerTag);
	HRESULT Ready_Layer_Player(const _tchar* pLayerTag);
	HRESULT Ready_Layer_Camera(const _tchar* pLayerTag);
	HRESULT Ready_Layer_Enemy(const _tchar* pLayerTag);
	HRESULT Ready_Layer_Effect(const _tchar* pLayerTag);
	HRESULT Ready_Object_Pool();

	void	Load_Objects();
	void	Load_Grass();
	HRESULT	Load_Effects();


	HRESULT	Read_EffectFile(HANDLE hFile, CPoolEffect::EFFECTPOOL_DESC& Desc);

private:
	class CTerrain*			m_pTerrain = { nullptr };

public:
	static CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END