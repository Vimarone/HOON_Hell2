#include "pch.h"
#include "../Public/MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "Imgui_Manager.h"

CMainApp::CMainApp()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CMainApp::Initialize()
{
    ENGINE_DESC EngineDesc {};

    EngineDesc.hInstance = g_hInst;
    EngineDesc.hWnd = g_hWnd;
    EngineDesc.iNumLevels = (_uint)LEVELID::LEVEL_END;
    EngineDesc.isWindowed = TRUE;
    EngineDesc.iViewportWidth = g_iWindowSizeX;
    EngineDesc.iViewportHeight = g_iWindowSizeY;

    if (FAILED(m_pGameInstance->Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
        return E_FAIL;

#ifdef _DEBUG
    if (FAILED(CImgui_Manager::Initialize(m_pDevice, m_pContext)))
        return E_FAIL;
#endif

    if (FAILED(Ready_Prototype_For_Static()))
        return E_FAIL;

    if (FAILED(Ready_Font()))
        return E_FAIL;

    if (FAILED(Start_Level(LEVELID::LEVEL_VILLAGE)))
        return E_FAIL;

    return S_OK;
}

void CMainApp::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    CImgui_Manager::Update();
#endif

    m_pGameInstance->Update_Engine(fTimeDelta);

#ifdef _DEBUG
    m_fTimerAcc += fTimeDelta;
#endif
}

HRESULT CMainApp::Render()
{
    if (FAILED(m_pGameInstance->Render_Begin(_float4(0.f, 0.f, 1.f, 1.f))))
        return E_FAIL;

    m_pGameInstance->Bind_DefaultRenderTarget();  // temp. for imgui docking 
    m_pGameInstance->Draw();


#ifdef _DEBUG
    ++m_iDrawCount;

    if (1.f <= m_fTimerAcc)
    {
        wsprintf(m_szFPS, TEXT("fps : %d"), m_iDrawCount);
        m_fTimerAcc = 0.f;
        m_iDrawCount = 0;
    }
    //SetWindowText(g_hWnd, m_szFPS);
    //m_pGameInstance->Render_Font(TEXT("Font_LiberationSans_20"), m_szFPS, _float2(0.f, 0.f));

    POINT ptMouse;
    GetCursorPos(&ptMouse);
    ScreenToClient(g_hWnd, &ptMouse);

    _wstring strMousePos = to_wstring((_int)ptMouse.x) + L", " + to_wstring((_int)ptMouse.y);
    //m_pGameInstance->Render_Font(L"Font_IBM3270_20", strMousePos.c_str(), {0.f, 30.f});

    CImgui_Manager::Render(m_pDevice, m_pContext);
#endif

    m_pGameInstance->Render_End();

    return S_OK;
}

HRESULT CMainApp::Start_Level(LEVELID eStartLevel)
{
    if (LEVELID::LEVEL_LOADING == eStartLevel)
        return E_FAIL;

    if (FAILED(m_pGameInstance->Open_Level(LEVELID::LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, eStartLevel))))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
#pragma region SHADER
    /* For.Prototype_Component_Shader_VtxPosTex */
    if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxPosTex_UI */
    if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_UI"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex_UI.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
        return E_FAIL;
#pragma endregion

#pragma region MODEL
    /* For.Prototype_Component_VIBuffer_Rect */
    if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        return E_FAIL;
#pragma endregion

#pragma region TEXTURE
    /* For.Prototype_Component_Texture_Square */
    if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, L"Prototype_Component_Texture_Square",
        CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/UI/Square.png", 1))))
        return E_FAIL;

    /* For.Prototype_Component_Texture_Logo */
    if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_STATIC, TEXT("Prototype_Component_Texture_Logo"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Logo/LogoTextV1.png"), 1))))
        return E_FAIL;
#pragma endregion

    return S_OK;
}

HRESULT CMainApp::Ready_Font()
{
    if (FAILED(m_pGameInstance->Add_Font(L"Font_LiberationSans_20", L"../Bin/Resources/Fonts/LiberationSans_20.spritefont")))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Add_Font(L"Font_PerfectDOS_20", L"../Bin/Resources/Fonts/PerfectDOS_20.spritefont")))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Add_Font(L"Font_IBM3270_20", L"../Bin/Resources/Fonts/IBM3270_20.spritefont")))
        return E_FAIL;

    return S_OK;
}

CMainApp* CMainApp::Create()
{
    CMainApp* pInstance = new CMainApp();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CMainApp");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMainApp::Free()
{
    /* 부모의 멤버를 정리하라고 이야기한다. */
    //__super::Free();
    super::Free();

#ifdef _DEBUG
    CImgui_Manager::Release();
#endif
    m_pGameInstance->Release_Engine();

    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
