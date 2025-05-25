#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 1. 다음 레벨이 필요로하는 자원을 준비한다. */
/* 2. 로딩화면을 구성(로딩화면에 필요한 객체를 만들고 갱신 렌더를 수행한다)한다. */

BEGIN(Client)

class CLoader;

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVELID eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	LEVELID m_eNextLevelID{ LEVELID::LEVEL_END };
	CLoader* m_pLoader{ nullptr };

	_float m_fCurrentProgress { 0.f };

private:
	HRESULT Ready_Layer_BackGround(const _tchar* pLayerTag);

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVELID eNextLevelID);
	virtual void Free() override;

};

END
