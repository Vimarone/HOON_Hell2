#include "pch.h"
#include "Sky.h"

#include "GameInstance.h"

CSky::CSky(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : super(pDevice, pContext)
{
}

CSky::CSky(const CSky& Prototype)
    : super(Prototype)
{
}

HRESULT CSky::Initialize_Prototype()
{
    if (FAILED(super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CSky::Initialize(void* pArg)
{
    DESC* pDesc = static_cast<DESC*>(pArg);
    if (nullptr != pDesc)
    {
        m_iTextureIndex = pDesc->iTextureIndex;
    }
	if (FAILED(super::Initialize(nullptr)))
		return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CSky::Priority_Update(_float fTimeDelta)
{
}

void CSky::Update(_float fTimeDelta)
{
    const _float4& vCamPos = m_pGameInstance->Get_CamPosition();
    m_pTransformCom->Locate(vCamPos.x, vCamPos.y, vCamPos.z);
}

void CSky::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDERGROUP::PRIORITY, this);
}

HRESULT CSky::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    m_pShaderCom->Begin(0);

    m_pVIBufferCom->Bind_InputAssembler();

    m_pVIBufferCom->Render();

    return S_OK;
}

HRESULT CSky::Ready_Components()
{
    /* Com_Shader */
    if (FAILED(super::Add_Component(LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxCube"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* Com_VIBuffer */
    if (FAILED(super::Add_Component(LEVEL_VILLAGE, TEXT("Prototype_Component_VIBuffer_Cube"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
        return E_FAIL;

    /* Com_Texture */
    //if (FAILED(super::Add_Component(LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Stars"),
    if (FAILED(super::Add_Component(LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Sky"),
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
        return E_FAIL;


    return S_OK;
}

HRESULT CSky::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_WorldMatrix_To_Shader(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(CPipeLine::TRANSFORM::PROJ))))
        return E_FAIL;

    //if (FAILED(m_pTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_Texture", 0)))
    if (FAILED(m_pTextureCom->Bind_Texture_To_Shader(m_pShaderCom, "g_Texture", m_iTextureIndex)))
        return E_FAIL;

    return S_OK;
}

CSky* CSky::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSky* pInstance = new CSky(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed To Create : CSky");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CCloneable* CSky::Clone(void* pArg)
{
    CSky* pInstance = new CSky(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed To Clone : CSky");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSky::Free()
{
    super::Free();

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
}
