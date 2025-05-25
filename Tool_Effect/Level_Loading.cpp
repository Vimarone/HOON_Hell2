#include "pch.h"
#include "Level_Loading.h"
#include "GameInstance.h"
#include "Loader.h"
#include "Level_EffectTool.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVELID eNextLevelID)
{
    m_eNextLevelID = eNextLevelID;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
    if (nullptr == m_pLoader)
        return E_FAIL;

    if (FAILED(Ready_Layer_BackGround(L"Layer_Background")))
        return E_FAIL;

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
    if (GetKeyState(VK_SPACE) < 0)
    {
        m_fCurrentProgress = m_pLoader->Get_CurrnetProgress();
        if (m_pLoader->IsFinisihed())
        {
            //m_pLoader->Get_LevelData();
            //Open_Level 에서 로딩 레벨 release하는 시점보다 Create 하는 시점이 더 앞서있음-> 로더 살아있음 -> 데이터 넘겨도 됨
            switch (m_eNextLevelID)
            {
            case LEVELID::LEVEL_VILLAGE:
                m_pGameInstance->Open_Level(m_eNextLevelID, CLevel_EffectTool::Create(m_pDevice, m_pContext));
                break;

            }
        }
    }
}

HRESULT CLevel_Loading::Render()
{
    m_pLoader->SetUp_WindowText();
    return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_BackGround(const _tchar* pLayerTag)
{

    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVELID eNextLevelID)
{
    CLevel_Loading* pInstance = new CLevel_Loading{ pDevice, pContext };

    if (FAILED(pInstance->Initialize(eNextLevelID))) // 로딩은 initalize 해도 상관없음
    {
        MSG_BOX("Failed to Create : CLevel_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    CLevel::Free();

    Safe_Release(m_pLoader);
}
