#pragma once

#include "Client_Defines.h"
#include "Base.h"

BEGIN(Engine)

class CGameInstance;

END

BEGIN(Client)

class CLoader final : public Engine::CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:
	HRESULT Initialize(LEVELID eNextLevelID);
	HRESULT Start_Loading();
	_bool IsFinisihed() const { return m_bIsFinished; }
	_float Get_CurrnetProgress() const;

#ifdef _DEBUG
public:
	void SetUp_WindowText();
#endif

private:
	ID3D11Device*			m_pDevice{ nullptr };
	ID3D11DeviceContext*	m_pContext{ nullptr };

	CGameInstance*			m_pGameInstance{ nullptr };

	LEVELID					m_eNextLevelID{ LEVELID::LEVEL_END };

	HANDLE					m_hThread{};
	CRITICAL_SECTION		m_CriticalSection{};

	_bool					m_bIsFinished{ FALSE };
	_tchar					m_szLoadingText[MAX_PATH]{};

	_uint					m_iNumTotalLine { 0 };
	_uint					m_iCurrentLine { 0 };

private:
	HRESULT Loading_For_Level_GamePlay();

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVELID eNextLevelID);
	virtual void Free() override;
};

END
