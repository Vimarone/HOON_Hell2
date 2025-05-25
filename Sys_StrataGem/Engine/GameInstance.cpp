#include "..\Public\GameInstance.h"

#include "Font_Manager.h"
#include "Timer_Manager.h"
#include "Level_Manager.h"
#include "Light_Manager.h"
#include "Object_Manager.h"
#include "Input_Device.h"
#include "Graphic_Device.h"
#include "Prototype_Manager.h"
#include "Calculate_Manager.h"
#include "Collision_Manager.h"
#include "Target_Manager.h"
#include "Picking.h"
#include "Gem_Manager.h"
#include "Frustum.h"
#include "Sound_Manager.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{

}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC & EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext)
{
	m_iViewportWidth = EngineDesc.iViewportWidth;
	m_iViewportHeight = EngineDesc.iViewportHeight;

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.isWindowed, EngineDesc.iViewportWidth, EngineDesc.iViewportHeight, ppDevice, ppContext);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	m_pTarget_Manager = CTarget_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pCalculate_Manager = CCalculate_Manager::Create();
	if (nullptr == m_pCalculate_Manager)
		return E_FAIL;
	
	m_pPool_Manager = CPool_Manager::Create();
	if (nullptr == m_pCalculate_Manager)
		return E_FAIL;

	m_pCollision_Manager = CCollision_Manager::Create();
	if (nullptr == m_pCollision_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd);
	if (nullptr == m_pPicking)
		return E_FAIL;

	m_pGem_Manager = CGem_Manager::Create();
	if (nullptr == m_pGem_Manager)
		return E_FAIL;

	m_pQuest_Manager = CQuest_Manager::Create();
	if (nullptr == m_pQuest_Manager)
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pSound_Manager = CSound_Manager::Create();
	if (nullptr == m_pSound_Manager)
		return E_FAIL;

	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	if (nullptr == m_pLevel_Manager)
		return;

	m_pGem_Manager->Update(fTimeDelta);
	m_pInput_Device->Update_InputDev();

	m_pPicking->Update();
	m_pObject_Manager->Priority_Update(fTimeDelta);
	m_pPool_Manager->Priority_Update(fTimeDelta);
	m_pPipeLine->Update();

	m_pFrustum->Update();

	m_pLight_Manager->Update(fTimeDelta);
	m_pObject_Manager->Update(fTimeDelta);
	m_pPool_Manager->Update(fTimeDelta);

	m_pObject_Manager->Late_Update(fTimeDelta);
	m_pPool_Manager->Late_Update(fTimeDelta);

	m_pLevel_Manager->Update(fTimeDelta);
}

HRESULT CGameInstance::Render_Begin(const _float4& vClearColor)
{
	m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();

	return S_OK;
}

HRESULT CGameInstance::Draw()
{
	m_pRenderer->Render();

	m_pLevel_Manager->Render();

	return S_OK;
}

HRESULT CGameInstance::Render_End()
{
	m_pGraphic_Device->Present();

	return S_OK;
}

void CGameInstance::Clear(_uint iLevelIndex)
{
	m_pPrototype_Manager->Clear(iLevelIndex);
	m_pObject_Manager->Clear(iLevelIndex);
}

#pragma region TIMER_MANAGER
_float CGameInstance::Get_TimeDelta(const _wstring & strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const _wstring & strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Compute_TimeDelta(const _wstring & strTimerTag)
{
	m_pTimer_Manager->Compute_TimeDelta(strTimerTag);
}
#pragma endregion

#pragma region LEVEL_MANAGER

HRESULT CGameInstance::Open_Level(_uint iNewLevelIndex, CLevel * pNewLevel)
{
	return m_pLevel_Manager->Open_Level(iNewLevelIndex, pNewLevel);	
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER
HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const _wstring & strPrototypeTag, CBase * pPrototype)
{

	return m_pPrototype_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}
CBase * CGameInstance::Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring & strPrototypeTag, void * pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(eType, iLevelIndex, strPrototypeTag, pArg);
}

#pragma endregion

#pragma region OBJECT_MANAGER
CGameObject* CGameInstance::Add_GameObject_To_Layer(_uint iPrototypeLevelIndex, const _wstring & strPrototypeTag, _uint iLevelIndex, const _wstring & strLayerTag, void * pArg)
{
	return m_pObject_Manager->Add_GameObject_To_Layer(iPrototypeLevelIndex, strPrototypeTag, iLevelIndex, strLayerTag, pArg);
}

list<CGameObject*>* CGameInstance::Find_GameObject_List_In_Layer(_uint iLevelIndex, const _wstring& strLayerTag)
{
	return m_pObject_Manager->Find_GameObject_List_In_Layer(iLevelIndex, strLayerTag);
}

CComponent* CGameInstance::Find_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponetTag, _uint iIndex)
{
	return m_pObject_Manager->Find_Component(iLevelIndex, strLayerTag, strComponetTag, iIndex);
}

CComponent* CGameInstance::Find_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strPartTag, const _wstring& strComponetTag, _uint iIndex)
{
	return m_pObject_Manager->Find_Component(iLevelIndex, strLayerTag, strPartTag, strComponetTag, iIndex);
}

#pragma endregion


#pragma region RENDERER
HRESULT CGameInstance::Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroupID, CGameObject * pGameObject)
{
	return m_pRenderer->Add_RenderGroup(eRenderGroupID, pGameObject);
}
HRESULT CGameInstance::Add_DebugComponent(CComponent* pDebugCom)
{
	return m_pRenderer->Add_DebugComponent(pDebugCom);;
}
#pragma endregion

#pragma region PIPELINE
_float4x4 CGameInstance::Get_Transform_Float4x4(CPipeLine::D3DTRANSFORMSTATE eState) const
{
	return m_pPipeLine->Get_Transform_Float4x4(eState);
}

_matrix CGameInstance::Get_Transform_Matrix(CPipeLine::D3DTRANSFORMSTATE eState) const
{
	return m_pPipeLine->Get_Transform_Matrix(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4_Inverse(CPipeLine::D3DTRANSFORMSTATE eState) const
{
	return m_pPipeLine->Get_Transform_Float4x4_Inverse(eState);
}

_matrix CGameInstance::Get_Transform_Matrix_Inverse(CPipeLine::D3DTRANSFORMSTATE eState) const
{
	return m_pPipeLine->Get_Transform_Matrix_Inverse(eState);
}

_float4 CGameInstance::Get_CamPosition() const
{
	return m_pPipeLine->Get_CamPosition();
}

void CGameInstance::Set_Transform(CPipeLine::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	return m_pPipeLine->Set_Transform(eState, TransformMatrix);
}
#pragma endregion

#pragma region INPUT_DEVICE

_byte	CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}
_byte	CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	return m_pInput_Device->Get_DIMouseState(eMouse);
}
_long	CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

_bool CGameInstance::Key_Down(_ubyte byKeyID)
{
	return m_pInput_Device->Key_Down(byKeyID);
}

_bool CGameInstance::Key_Up(_ubyte byKeyID)
{
	return m_pInput_Device->Key_Up(byKeyID);
}

_bool CGameInstance::Button_Down(MOUSEKEYSTATE eMouseState)
{
	return m_pInput_Device->Button_Down(eMouseState);
}

_bool CGameInstance::Button_Up(MOUSEKEYSTATE eMouseState)
{
	return m_pInput_Device->Button_Up(eMouseState);
}

#pragma endregion

#pragma region LIGHT_MANAGER

const LIGHT_DESC * CGameInstance::Get_LightDesc(_uint iIndex) const
{
	return m_pLight_Manager->Get_LightDesc(iIndex);
}

CLight* CGameInstance::Add_Light(const LIGHT_DESC & LightDesc, _float fLifeTime)
{
	return m_pLight_Manager->Add_Light(LightDesc, fLifeTime);
}
void CGameInstance::Render_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pLight_Manager->Render_Lights(pShader, pVIBuffer);
}
#ifdef _DEBUG
void CGameInstance::Delete_Light()
{
	m_pLight_Manager->Delete_Light();
}
_uint CGameInstance::Get_LightCount()
{
	return m_pLight_Manager->Get_LightCount();
}
#endif
#pragma endregion

#pragma region CALCULATE_MANAGER


_float3 CGameInstance::Picking_Terrain(HWND hWnd, CVIBuffer_Terrain* pBufferCom)
{
	return m_pCalculate_Manager->Picking_Terrain(hWnd, pBufferCom);
}

_float CGameInstance::Picking_Model(HWND hWnd, CModel* pModel, _fmatrix WorldMatrix)
{
	return m_pCalculate_Manager->Picking_Model(hWnd, pModel, WorldMatrix);
}

_vector CGameInstance::Picking_Model_Pos(HWND hWnd, CModel* pModel, _fmatrix WorldMatrix)
{
	return m_pCalculate_Manager->Picking_Model_Pos(hWnd, pModel, WorldMatrix);
}

HRESULT CGameInstance::Get_Ray(HWND hWnd, _float3* pRayPos, _float3* pRayDir)
{
	return m_pCalculate_Manager->Get_Ray(hWnd, pRayPos, pRayDir);
}

#pragma endregion

#pragma region POOL_MANAGER
HRESULT CGameInstance::Pool_Set(CPoolingObject::POOL_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg)
{
	return m_pPool_Manager->Pool_Set(eType, ePrototypeLevelID, strPrototypeTag, iNum, pArg);
}

CPoolingObject* CGameInstance::Pool_Get(CPoolingObject::POOL_TYPE eType, _fvector vTargetPos, _fvector vStartPos, _float fLifeTime, _float fDistance, _bool bIsBallistic)
{
	return m_pPool_Manager->Pool_Get(eType, vTargetPos, vStartPos, fLifeTime, fDistance, bIsBallistic);
}
vector<class CPoolingObject*>* CGameInstance::Get_Pools(CPoolingObject::POOL_TYPE eType)
{
	return m_pPool_Manager->Get_Pools(eType);
}
HRESULT CGameInstance::Effect_Set(CPoolEffect::EFFECT_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg)
{
	return m_pPool_Manager->Effect_Set(eType, ePrototypeLevelID, strPrototypeTag, iNum, pArg);
}
CPoolEffect* CGameInstance::Effect_Get(CPoolEffect::EFFECT_TYPE eType, _fvector vStartPos, _fvector vTargetPos)
{
	return m_pPool_Manager->Effect_Get(eType, vStartPos, vTargetPos);
}
#pragma endregion

#pragma region COLLISION_MANAGER
_bool CGameInstance::Collision_Ray_List(_fvector vOrigin, _fvector vDir, _float* pDist, list<CGameObject*>* GameObjects)
{
	return m_pCollision_Manager->Collision_Ray_List(vOrigin, vDir, pDist, GameObjects);
}
void CGameInstance::Collision_Enemy_Projectiles_Player(CGameObject* pPlayer)
{
	m_pCollision_Manager->Collision_Enemy_Projectiles_Player(pPlayer);
}
void CGameInstance::Collision_Projectiles_List(list<CGameObject*>* DstObjects)
{
	m_pCollision_Manager->Collision_Projectiles_List(DstObjects);
}
void CGameInstance::Collision_Projectiles_Terrain(CVIBuffer_Terrain* pTerrainBuffer)
{
	m_pCollision_Manager->Collision_Projectiles_Terrain(pTerrainBuffer);
}
void CGameInstance::Collision_Exploded_List(list<CGameObject*>* DstObjects)
{
	m_pCollision_Manager->Collision_Exploded_List(DstObjects);
}
HRESULT CGameInstance::Add_Projectiles(CPoolingObject* pProjectiles)
{
	m_pCollision_Manager->Add_Projectiles(pProjectiles);
	return S_OK;
}
HRESULT CGameInstance::Add_EnemyProjectiles(CPoolingObject* pProjectile)
{
	m_pCollision_Manager->Add_EnemyProjectiles(pProjectile);
	return S_OK;
}
HRESULT CGameInstance::Add_ExplodedProjectiles(CPoolingObject* pExploded)
{
	m_pCollision_Manager->Add_ExplodedProjectiles(pExploded);
	return S_OK;
}
void CGameInstance::Collision_End()
{
	m_pCollision_Manager->Collision_End();
}
#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}

HRESULT CGameInstance::Render_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, const _float fScale)
{
	return m_pFont_Manager->Render(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale);
}
#pragma endregion

#pragma region TARGET_MANAGER
HRESULT CGameInstance::Add_RenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strRenderTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}

HRESULT CGameInstance::Bind_RT_ShaderResource(const _wstring& strRenderTargetTag, CShader* pShader, const _char* pConstantName)
{
	return m_pTarget_Manager->Bind_ShaderResource(strRenderTargetTag, pShader, pConstantName);
}

HRESULT CGameInstance::Copy_RT_Resource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pTexture2D)
{
	return m_pTarget_Manager->Copy_Resource(strRenderTargetTag, pTexture2D);
}

HRESULT CGameInstance::Add_MRT(const _wstring& strMRTTag, const _wstring& strRenderTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strRenderTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const _wstring& strMRTTag, _bool isClear, ID3D11DepthStencilView* pDSV)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, isClear, pDSV);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

#ifdef _DEBUG
HRESULT CGameInstance::Ready_RT_Debug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTarget_Manager->Ready_RT_Debug(strRenderTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Render_RT_Debug(const _wstring& strMRTTag, CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pTarget_Manager->Render_RT_Debug(strMRTTag, pShader, pVIBuffer);
}
#endif
#pragma endregion

#pragma region PICKING
_bool CGameInstance::Compute_PickPos(_float3* pOut)
{
	return m_pPicking->Compute_PickPos(pOut);
}
#pragma endregion

#pragma region GEM_MANAGER
void CGameInstance::Check_Start()
{
	m_pGem_Manager->Check_Start();
}
_int CGameInstance::Check_Gems(_uint iKeyInput)
{
	return m_pGem_Manager->Check_Gems(iKeyInput);
}
void CGameInstance::Check_End()
{
	m_pGem_Manager->Check_End();
}
HRESULT CGameInstance::Get_CoolInfo(_uint iGemKey, _float& fOutCoolTime, _float& fOutCoolRate)
{
	return m_pGem_Manager->Get_CoolInfo(iGemKey, fOutCoolTime, fOutCoolRate);
}
HRESULT CGameInstance::Add_Gem(_uint iGemKey, vector<_uint>& GemSequence, _float fCoolTime, CEventObject* pEventObject)
{
	return m_pGem_Manager->Add_Gem(iGemKey, GemSequence, fCoolTime, pEventObject);
}
_int CGameInstance::Get_KeyByIndex(_uint iIndex)
{
	return m_pGem_Manager->Get_KeyByIndex(iIndex);
}
_uint CGameInstance::Get_GemCount()
{
	return m_pGem_Manager->Get_GemCount();
}
vector<_uint>& CGameInstance::Get_SeqByKey(_uint iKey)
{
	return m_pGem_Manager->Get_SeqByKey(iKey);
}
_uint CGameInstance::Get_CurrentArrowIndex()
{
	return m_pGem_Manager->Get_CurrentArrowIndex();
}
_bool CGameInstance::Get_SequeningByKey(_uint iKey)
{
	return m_pGem_Manager->Get_SequeningByKey(iKey);
}
_bool CGameInstance::Get_GemCheck()
{
	return m_pGem_Manager->Get_GemCheck();
}
CEventObject* CGameInstance::Get_EventObject(_uint iKey)
{
	return m_pGem_Manager->Get_EventObject(iKey);
}
#pragma endregion


#pragma region QUEST_MANAGER
HRESULT CGameInstance::Add_Quest(const _wstring& strQuestTag, const _wstring& strQuestDesc)
{
	return m_pQuest_Manager->Add_Quest(strQuestTag, strQuestDesc);
}
HRESULT CGameInstance::Clear_Quest(_uint iIndex)
{
	return m_pQuest_Manager->Clear_Quest(iIndex);
}
HRESULT CGameInstance::Activate_Quest(_uint iIndex)
{
	return m_pQuest_Manager->Activate_Quest(iIndex);
}
CQuest_Manager::QUEST_DESC* CGameInstance::Find_Quest(_uint iIndex)
{
	return m_pQuest_Manager->Find_Quest(iIndex);
}
const vector<CQuest_Manager::QUEST_DESC*>& CGameInstance::Find_Quests()
{
	return m_pQuest_Manager->Find_Quests();
}
_uint CGameInstance::Get_ActivatedQuestCount()
{
	return m_pQuest_Manager->Get_ActivatedQuestCount();
}
#pragma endregion


#pragma region FRUSTUM
void CGameInstance::Transform_Frustum_LocalSpace(_fmatrix WorldMatrix)
{
	return m_pFrustum->Transform_LocalSpace(WorldMatrix);
}
_bool CGameInstance::isIn_Frustum_WorldSpace(_fvector vWorldPoint, _float fRange)
{
	return m_pFrustum->isIn_WorldSpace(vWorldPoint, fRange);
}

_bool CGameInstance::isIn_Frustum_LocalSpace(_fvector vLocalPoint, _float fRange)
{
	return m_pFrustum->isIn_LocalSpace(vLocalPoint, fRange);
}
#pragma endregion

#pragma region SOUND_MANAGER
void CGameInstance::Play(const TCHAR* pSoundKey, CHANNELID eID, float fVolume)
{
	m_pSound_Manager->Play(pSoundKey, eID, fVolume);
}
void CGameInstance::PlayOnce(const TCHAR* pSoundKey, CHANNELID eID, float fVolume)
{
	m_pSound_Manager->PlayOnce(pSoundKey, eID, fVolume);
}
void CGameInstance::PlayBGM(const TCHAR* pSoundKey, float fVolume)
{
	m_pSound_Manager->PlayBGM(pSoundKey, fVolume);
}
void CGameInstance::StopSound(CHANNELID eID)
{
	m_pSound_Manager->StopSound(eID);
}
void CGameInstance::StopAll()
{
	m_pSound_Manager->StopAll();
}
void CGameInstance::SetChannelVolume(CHANNELID eID, float fVolume)
{
	m_pSound_Manager->SetChannelVolume(eID, fVolume);
}
#pragma endregion

void CGameInstance::Release_Engine()
{
	Safe_Release(m_pGraphic_Device);	
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pLevel_Manager);	
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pPrototype_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pCalculate_Manager);
	Safe_Release(m_pPool_Manager);
	Safe_Release(m_pCollision_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pPicking);
	Safe_Release(m_pGem_Manager);
	Safe_Release(m_pQuest_Manager);
	Safe_Release(m_pFrustum);
	Safe_Release(m_pSound_Manager);

	CGameInstance::GetInstance()->DestroyInstance();
}

void CGameInstance::Free()
{
	__super::Free();


}
