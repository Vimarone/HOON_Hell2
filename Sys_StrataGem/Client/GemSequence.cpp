#include "framework.h"
#include "GameInstance.h"

#include "GemSequence.h"
#include "GemArrow.h"

CGemSequence::CGemSequence(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIContainer{ pDevice, pContext }
{
}

CGemSequence::CGemSequence(const CGemSequence& Prototype)
	:CUIContainer{ Prototype }
{
}

HRESULT CGemSequence::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGemSequence::Initialize(void* pArg)
{
	GEMSEQ_DESC* pDesc = static_cast<GEMSEQ_DESC*>(pArg);
	//SeqDesc.fX = m_fX + m_fSizeX * 0.5f + 20.f;
	//SeqDesc.fY = m_fY + m_fSizeY * 0.5f;

	m_pParentPosX = pDesc->pParentPosX;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;

	m_fX = *m_pParentPosX - pDesc->fSizeX - 10.f;
	pDesc->fSizeX = pDesc->fSizeY - 5.f;
	pDesc->fSizeY = pDesc->fSizeY - 5.f;

	pDesc->fNear = 0.f;
	pDesc->fFar = 1.f;

	pDesc->fSpeedPerSec = 0.f;
	pDesc->fRotationPerSec = XMConvertToRadians(0.f);

	m_eType = pDesc->eType;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fY = vPos.m128_f32[1];

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Sequence()))
		return E_FAIL;


	return S_OK;
}

void CGemSequence::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGemSequence::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fX = *m_pParentPosX - m_fSizeX - 10.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX, m_fY, 0.f, 1.f));
}

void CGemSequence::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RG_UI, this);
}

HRESULT CGemSequence::Render()
{
	__super::Render();
	//if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
	//	return E_FAIL;

	//if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
	//	return E_FAIL;

	//m_pShaderCom->Begin(0);

	//m_pVIBufferCom->Bind_InputAssembler();

	//m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CGemSequence::Ready_Components()
{
	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_OrangeDot"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;


	return S_OK;
}

HRESULT CGemSequence::Ready_Sequence()
{
	CUIObject* pGemUI = nullptr;
	CGemArrow::GEMARROW_DESC SeqDesc{};
	vector<_uint> Sequence = m_pGameInstance->Get_SeqByKey(m_eType);
	_uint iSeqLength = Sequence.size();
	for (size_t i = 0; i < iSeqLength; i++)
	{
		SeqDesc.eType = m_eType;
		SeqDesc.iIndex = i;
		SeqDesc.iArrow = Sequence[i];
		SeqDesc.pParentPosX = &m_fX;
		SeqDesc.fX = m_fX;
		SeqDesc.fY = m_fY;
		SeqDesc.fSizeX = m_fSizeX;
		SeqDesc.fSizeY = m_fSizeY;
		pGemUI = static_cast<CGemSequence*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_GemArrow"), &SeqDesc));
		m_PartUIs.push_back(pGemUI);
	}
	return S_OK;
}

CGemSequence* CGemSequence::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGemSequence* pInstance = new CGemSequence(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Create : CGemSequence");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGemSequence::Clone(void* pArg)
{
	CGemSequence* pInstance = new CGemSequence(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Clone : CGemSequence");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGemSequence::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
