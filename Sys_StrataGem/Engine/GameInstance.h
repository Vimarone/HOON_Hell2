#pragma once

#include "PipeLine.h"
#include "Renderer.h"
#include "Prototype_Manager.h"
#include "Pool_Manager.h"
#include "Quest_Manager.h"

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance);
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	_uint2 Get_ViewportSize() const {
		return _uint2(m_iViewportWidth, m_iViewportHeight);
	}

public:
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);
	void Update_Engine(_float fTimeDelta);
	HRESULT Render_Begin(const _float4 & vClearColor);
	HRESULT Draw();
	HRESULT Render_End();
	void Clear(_uint iLevelIndex);

	_float Compute_Random_Normal() { return static_cast<_float>(rand()) / RAND_MAX; }
	_float Compute_Random(_float fMin, _float fMax) { return fMin + Compute_Random_Normal() * (fMax - fMin); }

#pragma region TIMER_MANAGER
public:
	_float			Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT			Add_Timer(const _wstring& strTimerTag);
	void			Compute_TimeDelta(const _wstring& strTimerTag);
#pragma endregion

#pragma region LEVEL_MANAGER
	HRESULT Open_Level(_uint iNewLevelIndex, class CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
#pragma endregion

#pragma region OBJECT_MANAGER
	class CGameObject* Add_GameObject_To_Layer(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr);
	list<class CGameObject*>* Find_GameObject_List_In_Layer(_uint iLevelIndex, const _wstring& strLayerTag);
	class CComponent* Find_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponetTag, _uint iIndex = 0);
	class CComponent* Find_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strPartTag, const _wstring& strComponetTag, _uint iIndex = 0);
#pragma endregion

#pragma region RENDERER
	HRESULT Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroupID, class CGameObject* pGameObject);
#ifdef _DEBUG
public:
	HRESULT Add_DebugComponent(class CComponent* pDebugCom);
#endif
#pragma endregion

#pragma region PIPELINE
	_float4x4 Get_Transform_Float4x4(CPipeLine::D3DTRANSFORMSTATE eState) const;
	_matrix Get_Transform_Matrix(CPipeLine::D3DTRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4_Inverse(CPipeLine::D3DTRANSFORMSTATE eState) const;
	_matrix Get_Transform_Matrix_Inverse(CPipeLine::D3DTRANSFORMSTATE eState) const;
	_float4 Get_CamPosition() const;
	void Set_Transform(CPipeLine::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix);
#pragma endregion

#pragma region INPUT_DEVICE
	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse);	
	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState);
	_bool   Key_Down(_ubyte byKeyID);
	_bool   Key_Up(_ubyte byKeyID);
	_bool   Button_Down(MOUSEKEYSTATE eMouseState);
	_bool   Button_Up(MOUSEKEYSTATE eMouseState);
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC* Get_LightDesc(_uint iIndex) const;
	class CLight* Add_Light(const LIGHT_DESC& LightDesc, _float fLifeTime = 1000.f);
	void Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#ifdef _DEBUG
	void Delete_Light();
	_uint Get_LightCount();
#endif
#pragma endregion

#pragma region CALCULATE_MANAGER
	_float3	Picking_Terrain(HWND hWnd, class CVIBuffer_Terrain* pBufferCom);
	_float	Picking_Model(HWND hWnd, class CModel* pModel, _fmatrix WorldMatrix);
	_vector Picking_Model_Pos(HWND hWnd, class CModel* pModel, _fmatrix WorldMatrix);
	HRESULT	Get_Ray(HWND hWnd, _float3* pRayPos, _float3* pRayDir);
#pragma endregion

#pragma region POOL_MANAGER
	HRESULT							Pool_Set(CPoolingObject::POOL_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg = nullptr);
	class CPoolingObject*			Pool_Get(CPoolingObject::POOL_TYPE eType, _fvector vTargetPos, _fvector vStartPos, _float fLifeTime, _float fDistance = 0.f, _bool bIsBallistic = false);
	vector<class CPoolingObject*>*	Get_Pools(CPoolingObject::POOL_TYPE eType);

	HRESULT						Effect_Set(CPoolEffect::EFFECT_TYPE eType, _uint ePrototypeLevelID, const _wstring& strPrototypeTag, _uint iNum, void* pArg);
	CPoolEffect*				Effect_Get(CPoolEffect::EFFECT_TYPE eType, _fvector vStartPos, _fvector vTargetPos = {});
#pragma endregion

#pragma region COLLISION_MANAGER
	_bool	Collision_Ray_List(_fvector vOrigin, _fvector vDir, _float* pDist, list<class CGameObject*>* GameObjects);
	void	Collision_Enemy_Projectiles_Player(class CGameObject* pPlayer);
	void	Collision_Projectiles_List(list<class CGameObject*>* DstObjects);
	void	Collision_Projectiles_Terrain(class CVIBuffer_Terrain* pTerrainBuffer);
	void	Collision_Exploded_List(list<class CGameObject*>* DstObjects);
	HRESULT	Add_Projectiles(class CPoolingObject* pProjectiles);
	HRESULT	Add_EnemyProjectiles(class CPoolingObject* pProjectile);
	HRESULT Add_ExplodedProjectiles(class CPoolingObject* pExploded);
	void	Collision_End();
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT Render_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), const _float fScale = 1.f);
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Bind_RT_ShaderResource(const _wstring& strRenderTargetTag, class CShader* pShader, const _char* pConstantName);
	HRESULT Copy_RT_Resource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pTexture2D);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strRenderTargetTag);
	HRESULT Begin_MRT(const _wstring& strMRTTag, _bool isClear = true, ID3D11DepthStencilView* pDSV = nullptr);
	HRESULT End_MRT();

#ifdef _DEBUG
public:
	HRESULT Ready_RT_Debug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif
#pragma endregion

#pragma region PICKING
	_bool Compute_PickPos(_float3* pOut);
#pragma endregion

#pragma region GEM_MANAGER
	void				Check_Start();
	_int				Check_Gems(_uint iKeyInput);
	void				Check_End();
	HRESULT				Get_CoolInfo(_uint iGemKey, _float& fOutCoolTime, _float& fOutCoolRate);
	HRESULT				Add_Gem(_uint iGemKey, vector<_uint>& GemSequence, _float fCoolTime, class CEventObject* pEventObject);
	_int				Get_KeyByIndex(_uint iIndex);
	_uint				Get_GemCount();
	vector<_uint>&		Get_SeqByKey(_uint iKey);
	_uint				Get_CurrentArrowIndex();
	_bool				Get_SequeningByKey(_uint iKey);
	_bool				Get_GemCheck();
	class CEventObject* Get_EventObject(_uint iKey);
#pragma endregion

#pragma region QUEST_MANAGER
	HRESULT												Add_Quest(const _wstring& strQuestTag, const _wstring& strQuestDesc);
	HRESULT												Clear_Quest(_uint iIndex);
	HRESULT												Activate_Quest(_uint iIndex);
	_uint												Get_ActivatedQuestCount();
	CQuest_Manager::QUEST_DESC*							Find_Quest(_uint iIndex);
	const vector<CQuest_Manager::QUEST_DESC*>&			Find_Quests();
#pragma endregion

#pragma region FRUSTUM
	void	Transform_Frustum_LocalSpace(_fmatrix WorldMatrix);
	_bool	isIn_Frustum_WorldSpace(_fvector vWorldPoint, _float fRange = 0.f);
	_bool	isIn_Frustum_LocalSpace(_fvector vLocalPoint, _float fRange = 0.f);
#pragma endregion

#pragma region SOUND_MANAGER
	void	Play(const TCHAR* pSoundKey, CHANNELID eID, float fVolume);
	void	PlayOnce(const TCHAR* pSoundKey, CHANNELID eID, float fVolume);
	void	PlayBGM(const TCHAR* pSoundKey, float fVolume);

	void	StopSound(CHANNELID eID);
	void	StopAll();
	void	SetChannelVolume(CHANNELID eID, float fVolume);
#pragma endregion

private:
	_uint								m_iViewportWidth{}, m_iViewportHeight{};

private:
	class CTimer_Manager*				m_pTimer_Manager = { nullptr };
	class CGraphic_Device*				m_pGraphic_Device = { nullptr };
	class CInput_Device*				m_pInput_Device = { nullptr };
	class CLevel_Manager*				m_pLevel_Manager = { nullptr };	
	class CObject_Manager*				m_pObject_Manager = { nullptr };
	class CPrototype_Manager*			m_pPrototype_Manager = { nullptr };
	class CRenderer*					m_pRenderer = { nullptr };
	class CPipeLine*					m_pPipeLine = { nullptr };
	class CLight_Manager*				m_pLight_Manager = { nullptr };
	class CCalculate_Manager*			m_pCalculate_Manager = { nullptr };
	CPool_Manager*						m_pPool_Manager = { nullptr };
	class CCollision_Manager*			m_pCollision_Manager = { nullptr };
	class CFont_Manager*				m_pFont_Manager = { nullptr };
	class CTarget_Manager*				m_pTarget_Manager = { nullptr };
	class CPicking*						m_pPicking = { nullptr };
	class CGem_Manager*					m_pGem_Manager = { nullptr };
	CQuest_Manager*						m_pQuest_Manager = { nullptr };
	class CFrustum*						m_pFrustum = { nullptr };
	class CSound_Manager*				m_pSound_Manager = { nullptr };

public:
	void Release_Engine();
	virtual void Free() override;
	
};

END