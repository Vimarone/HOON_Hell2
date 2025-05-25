#include "pch.h"
#include "Tutorial_Terrain.h"

#include "GameInstance.h"

CTutorial_Terrain::CTutorial_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTutorial_Terrain::CTutorial_Terrain(const CTutorial_Terrain& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CTutorial_Terrain::Initialize_Prototype()
{
    if (FAILED(super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CTutorial_Terrain::Initialize(void* pArg)
{
    CGameObject::GAMEOBJECT_DESC desc{};

    if (FAILED(super::Initialize(&desc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pTransformCom->Locate(-64.f, 0.f, -64.f);

    return S_OK;
}

void CTutorial_Terrain::Priority_Update(_float fTimeDelta)
{
}

void CTutorial_Terrain::Update(_float fTimeDelta)
{
}

void CTutorial_Terrain::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::NONBLEND, this);
}

HRESULT CTutorial_Terrain::Render()
{
    if (FAILED(Bind_ObjectID_To_Shader(m_pShaderCom, "g_iObjectID"))) // 즁요 ㅋㅋ
        return E_FAIL;

    HRESULT hr{};

    hr = Bind_ShaderResources();
    if (FAILED(hr))
        return E_FAIL;

    hr = m_pShaderCom->Begin(0); // 0 : default , 1 : brush
    if (FAILED(hr))
        return E_FAIL;

    hr = m_pVIBufferCom->Bind_InputAssembler();
    if (FAILED(hr))
        return E_FAIL;

    hr = m_pVIBufferCom->Render();
    if (FAILED(hr))
        return E_FAIL;

    return S_OK;
}

HRESULT CTutorial_Terrain::Load_HeightMap(const _tchar* pHeightMapFile)
{
    //return m_pVIBufferCom->Load_HeightMap(pHeightMapFile);
    return S_OK;
}

HRESULT CTutorial_Terrain::Ready_Components()
{
    /* Com_Texture_Diffuse */
    if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Terrain"),
        TEXT("Com_Texture_Diffuse"), reinterpret_cast<CComponent**>(&m_TextureComs[(_uint)TEXTURE_TYPE::DIFFUSE]))))
        return E_FAIL;


    {// 레벨별로 노말 거시기 하기 
       /* Com_Texture_Normal*/
        if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Normal"),
            TEXT("Com_Texture_Normal"), reinterpret_cast<CComponent**>(&m_TextureComs[(_uint)TEXTURE_TYPE::NORMAL]))))
            return E_FAIL;

        /* Com_Texture_Normal_Second */
        if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Normal_Wavy"), // 약한 웨이브ㅏ
            TEXT("Com_Texture_Normal_Second"), reinterpret_cast<CComponent**>(&m_TextureComs[(_uint)TEXTURE_TYPE::SECOND_NORMAL]))))
            return E_FAIL;

        /* Com_Texture_Normal_Third */
        if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Normal_Sand"), // 흰빨쎈거
            TEXT("Com_Texture_Normal_Third"), reinterpret_cast<CComponent**>(&m_TextureComs[(_uint)TEXTURE_TYPE::THIRD_NORMAL]))))
            return E_FAIL;

        /* Com_Texture_MetalicSmoothness */
        if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_MS_Sand"), //메탈릭스무스니스
            TEXT("Com_Texture_MetalicSmoothness"), reinterpret_cast<CComponent**>(&m_TextureComs[(_uint)TEXTURE_TYPE::MSEG]))))
            return E_FAIL;

        /* Com_Shader */
        if (FAILED(super::Add_Component(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxNorTex"),
            TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
            return E_FAIL;

        /* Com_VIBuffer */
        if (FAILED(super::Add_Component(m_pGameInstance->Get_CurrentLevelIndex(), TEXT("Prototype_Component_VIBuffer_Terrain_Tutorial"),
            TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CTutorial_Terrain::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_WorldMatrix_To_Shader(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
        m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
        m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::PROJ))))
        return E_FAIL;

    if (FAILED(m_TextureComs[(_uint)TEXTURE_TYPE::DIFFUSE]->Bind_Textures_To_Shader(m_pShaderCom, "g_DiffuseTexture")))
        return E_FAIL;
    if (FAILED(m_TextureComs[(_uint)TEXTURE_TYPE::NORMAL]->Bind_Texture_To_Shader(m_pShaderCom, "g_NormalTexture"))) // Texture
        return E_FAIL;
    if (FAILED(m_TextureComs[(_uint)TEXTURE_TYPE::SECOND_NORMAL]->Bind_Texture_To_Shader(m_pShaderCom, "g_SecondNormalTexture"))) // Texture
        return E_FAIL;
    if (FAILED(m_TextureComs[(_uint)TEXTURE_TYPE::THIRD_NORMAL]->Bind_Texture_To_Shader(m_pShaderCom, "g_ThirdNormalTexture"))) // Texture
        return E_FAIL;
    if (FAILED(m_TextureComs[(_uint)TEXTURE_TYPE::MSEG]->Bind_Texture_To_Shader(m_pShaderCom, "g_MSTexture"))) // Texture
        return E_FAIL;

    return S_OK;
}

CTutorial_Terrain* CTutorial_Terrain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTutorial_Terrain* pInstance = new CTutorial_Terrain(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed To Create : CTutorial_Terrain");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CCloneable* CTutorial_Terrain::Clone(void* pArg)
{
    CTutorial_Terrain* pInstance = new CTutorial_Terrain(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed To Clone : CTutorial_Terrain");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTutorial_Terrain::Free()
{
    super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pNavigationCom);

    for (auto& pTextureCom : m_TextureComs)
        Safe_Release(pTextureCom);
}
