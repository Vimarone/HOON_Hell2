#include "framework.h"
#include "..\Public\Level_GamePlay.h"

#include "GameInstance.h"
#include "Camera_Follow.h"
#include "Terrain.h"
#include "MapObject.h"
#include "EventObject.h"
#include "Enemy_Reaper.h"
#include "Enemy_Spectre.h"
#include "Enemy_Stalker.h"
#include "Grass.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Gems(TEXT("Layer_Gem"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Enemy(TEXT("Layer_Enemy"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	if (FAILED(Ready_Object_Pool()))
		return E_FAIL;

	if (FAILED(Ready_Quests(TEXT("Layer_QuestUI"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	m_pGameInstance->Collision_Projectiles_Terrain(static_cast<CVIBuffer_Terrain*>(m_pTerrain->Find_Component(TEXT("Com_VIBuffer"))));
	m_pGameInstance->Collision_Projectiles_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Enemy")));
	m_pGameInstance->Collision_Projectiles_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Quest")));
	// m_pGameInstance->Collision_Projectiles_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Collide")));
	m_pGameInstance->Collision_Enemy_Projectiles_Player(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front());
	m_pGameInstance->Collision_Exploded_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Enemy")));
	m_pGameInstance->Collision_Exploded_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player")));
	m_pGameInstance->Collision_Exploded_List(m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Quest")));
	m_pGameInstance->Collision_End();
}

HRESULT CLevel_GamePlay::Render()
{
#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));
#endif

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
	HANDLE hFile = CreateFile(L"../Bin/DataFiles/Lights.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	_ulong dwByte = {};
	_uint iCount{};

	ReadFile(hFile, &iCount, sizeof(_int), &dwByte, nullptr);
	while (true)
	{
		LIGHT_DESC Desc{};
		for (size_t i = 0; i < iCount; i++)
		{
			ReadFile(hFile, &Desc.eType, sizeof(_int), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vDirection, sizeof(_float4), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vPosition, sizeof(_float4), &dwByte, nullptr);
			ReadFile(hFile, &Desc.fRange, sizeof(_float), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vDiffuse, sizeof(_float4), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vAmbient, sizeof(_float4), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vSpecular, sizeof(_float4), &dwByte, nullptr);

			m_pGameInstance->Add_Light(Desc);
		}

		if (dwByte == 0)
			break;
	}

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Quests(const _tchar* pLayerTag)
{
	if (FAILED(m_pGameInstance->Add_Quest(TEXT("불법 방송 종료하기"), TEXT(""))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Quest(TEXT("탈출 가능."), TEXT(""))))
		return E_FAIL;

	m_pGameInstance->Activate_Quest(0);
	CUIObject::UIOBJECT_DESC Desc{};
	Desc.fSizeX = 400.f;
	Desc.fSizeY = 10.f;
	Desc.fX = g_iWinSizeX - Desc.fSizeX * 0.5f - 30.f;
	Desc.fY = 50.f;
	m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_QuestBG"), LEVEL_GAMEPLAY, pLayerTag, &Desc);

	if (nullptr == (m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Dropship"), LEVEL_GAMEPLAY, TEXT("Layer_Dropship"), nullptr)))
		return E_FAIL;

	if (nullptr == m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_QuestClear"), LEVEL_GAMEPLAY, pLayerTag))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const _tchar* pLayerTag)
{
	CTerrain::TERRAIN_DESC Desc{};
	Desc.eLevelID = LEVEL_GAMEPLAY;

	m_pTerrain = static_cast<CTerrain*>(m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, pLayerTag, &Desc));
	if (nullptr == m_pTerrain)
		return E_FAIL;

	if (nullptr == (m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Sky"), LEVEL_GAMEPLAY, pLayerTag, nullptr)))
		return E_FAIL;

	Load_Objects();
	Load_Grass();

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player(const _tchar* pLayerTag)
{
	if (nullptr == (m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Player"), LEVEL_GAMEPLAY, pLayerTag, nullptr)))
		return E_FAIL;

	if (nullptr == (m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_DeadUI"), LEVEL_GAMEPLAY, TEXT("Layer_PlayerUI"), nullptr)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const _tchar* pLayerTag)
{
	CCamera_Follow::FOLLOW_CAMERA_DESC		Desc = {};

	Desc.vEye = _float3(0.f, 10.f, -7.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);

	Desc.fFovy = XMConvertToRadians(60.f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1800.f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.f);

	Desc.fMouseSensor = 0.1f;
	Desc.pFollowTarget = m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Player"))->front();

	if (nullptr == (m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Camera_Follow"), LEVEL_GAMEPLAY, pLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Enemy(const _tchar* pLayerTag)
{
	_ulong dwByte = {};
	HANDLE hFile = CreateFile(L"../Bin/DataFiles/Enemies.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	_uint iType{}, iSize{};
	_int iCellIndex{};

	CGameObject* pGameObject{ nullptr };

	while (true)
	{
		ReadFile(hFile, &iType, sizeof(_uint), &dwByte, nullptr);
		switch (iType)
		{
		case ENEMY_SPECTRE:
			pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Spectre"), LEVEL_GAMEPLAY, pLayerTag);
			break;
		case ENEMY_STALKER:
			pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Stalker"), LEVEL_GAMEPLAY, pLayerTag);
			break;
		case ENEMY_REAPER:
			pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Reaper"), LEVEL_GAMEPLAY, pLayerTag);
			break;
		}
		ReadFile(hFile, &iSize, sizeof(_uint), &dwByte, nullptr);
		for (size_t i = 0; i < iSize; i++)
		{
			ReadFile(hFile, &iCellIndex, sizeof(_int), &dwByte, nullptr);
			switch (iType)
			{
			case ENEMY_SPECTRE:
				static_cast<CEnemy_Spectre*>(pGameObject)->Add_PatrolList(iCellIndex);
				break;
			case ENEMY_STALKER:
				static_cast<CEnemy_Stalker*>(pGameObject)->Add_PatrolList(iCellIndex);
				break;
			case ENEMY_REAPER:
				static_cast<CEnemy_Reaper*>(pGameObject)->Add_PatrolList(iCellIndex);
				break;
			}
		}

		if (dwByte == 0)
		{
			if (pGameObject != nullptr)
			{
				list<CGameObject*>* pObjectList = m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, pLayerTag);
				pObjectList->pop_back();
				Safe_Release(pGameObject);
			}
			break;
		}
	}
	CloseHandle(hFile);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Effect(const _tchar* pLayerTag)
{
	// 개별 이펙트 불러오기
	m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_UIScreen"), LEVEL_GAMEPLAY, pLayerTag);

	// 파일 입출력하여 정보 입력 후 이펙트 풀링
	if (FAILED(Load_Effects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Object_Pool()
{
	CTransform::TRANSFORM_DESC	Desc{};
	Desc.fSpeedPerSec = 2000.f;
	Desc.fRotationPerSec = 0.f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_BULLET, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_ARBullet_Player"), 2000, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 240.f;
	Desc.fRotationPerSec = 0.5f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_ROCKET, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rocket"), 30, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 20.f;
	Desc.fRotationPerSec = 0.5f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_MISSILE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Missile"), 50, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 160.f;
	Desc.fRotationPerSec = 0.5f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_GRENADE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Grenade"), 10, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = 0.f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_INVISIBLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Invisible"), 30, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = 0.f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_EFFECTBALL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_EffectBall"), 400, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 160.f;
	Desc.fRotationPerSec = 0.5f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_GEM, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Stratagem"), 10, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 400.f;
	Desc.fRotationPerSec = 0.5f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_HIGHEXPL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_HighExplosive"), 30, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 240.f;
	Desc.fRotationPerSec = 0.f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_HALFTON, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_HalfTon"), 10, &Desc)))
		return E_FAIL;

	Desc.fSpeedPerSec = 500.f;
	Desc.fRotationPerSec = 0.f;
	if (FAILED(m_pGameInstance->Pool_Set(CPoolingObject::POOL_AUTOCANNON, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_AutoCannon"), 200, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Gems(const _tchar* pLayerTag)
{
	CGameObject* pGameObject = { nullptr };
	CEventObject::EVENT_DESC pDesc{};
	vector<_uint> GemSequence;
	_float fCoolTime{};

	GemSequence.reserve(10);

	// Reinforcements
	pDesc.fCallDelay = 0.f;
	pDesc.fEffectiveRange = 0.f;
	pDesc.iCount = 1;
	pDesc.fCountDelay = 0.f;
	pDesc.iFrequency = 1;
	pDesc.fFrequencyDelay = 0.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Reinforcement"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	CEventObject* pReinforcement = static_cast<CEventObject*>(pGameObject);

	GemSequence.clear();
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_LEFT);
	GemSequence.push_back(ARROW_UP);
	fCoolTime = 5.f;
	m_pGameInstance->Add_Gem(REINFORCEMENT, GemSequence, fCoolTime, pReinforcement);

	// Supply
	// 딜레이 12.25초
	pDesc.fCallDelay = 12.25f;
	pDesc.fEffectiveRange = 0.f;
	pDesc.iCount = 1;
	pDesc.fCountDelay = 0.f;
	pDesc.iFrequency = 1;
	pDesc.fFrequencyDelay = 0.f;

	pDesc.fSpeedPerSec = 200.f;
	pDesc.fRotationPerSec = 0.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Supply"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	CEventObject* pSupply = static_cast<CEventObject*>(pGameObject);

	GemSequence.clear();
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_RIGHT);
	fCoolTime = 60.f;
	m_pGameInstance->Add_Gem(SUPPLY, GemSequence, fCoolTime, pSupply);


	// HORNET_STRAFE
	// 딜레이 2.4초, 범위 6m
	pDesc.fSpeedPerSec = 750.f;
	pDesc.fRotationPerSec = 0.05f;

	pDesc.fCallDelay = 5.f;
	pDesc.fEffectiveRange = 10.f;
	pDesc.iCount = 20;
	pDesc.fCountDelay = 0.02f;
	pDesc.iFrequency = 1;
	pDesc.fFrequencyDelay = 0.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Hornet"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	CEventObject* pHornet = static_cast<CEventObject*>(pGameObject);

	GemSequence.clear();
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_RIGHT);
	fCoolTime = 30.f;
	m_pGameInstance->Add_Gem(HORNET_STRAFE, GemSequence, fCoolTime, pHornet);

	// HORNET_HALF
	// 딜레이 3.45초, 범위 35m
	pDesc.fCallDelay = 14.45f;
	pDesc.fEffectiveRange = 0.f;
	pDesc.iCount = 1;
	pDesc.fCountDelay = 0.f;
	pDesc.iFrequency = 1;
	pDesc.fFrequencyDelay = 0.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Hornet"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	pHornet = static_cast<CEventObject*>(pGameObject);

	GemSequence.clear();
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_DOWN);
	fCoolTime = 30.f;
	m_pGameInstance->Add_Gem(HORNET_HALF, GemSequence, fCoolTime, pHornet);

	// ORBITAL_PRECISION
	// 딜레이 4.5초, 범위 12m
	pDesc.fSpeedPerSec = 0.f;
	pDesc.fRotationPerSec = 0.f;

	pDesc.fCallDelay = 9.5f;
	pDesc.fEffectiveRange = 0.f;
	pDesc.iCount = 1;
	pDesc.fCountDelay = 0.f;
	pDesc.iFrequency = 1;
	pDesc.fFrequencyDelay = 0.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Widow"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	CEventObject* pWidow = static_cast<CEventObject*>(pGameObject);

	GemSequence.clear();
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_UP);
	fCoolTime = 90.f;
	m_pGameInstance->Add_Gem(ORBITAL_PRECISION, GemSequence, fCoolTime, pWidow);

	// ORBITAL_380
	// 딜레이 8.45초, 범위 50m
	// 3발 5회, 총 15발
	// 회당 간격 3초, 착탄 간격 1.5초
	pDesc.fCallDelay = 16.45f;
	pDesc.fEffectiveRange = 50.f;
	pDesc.iCount = 5;
	pDesc.fCountDelay = 1.5f;
	pDesc.iFrequency = 3;
	pDesc.fFrequencyDelay = 3.f;
	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Widow"), LEVEL_GAMEPLAY, pLayerTag, &pDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	pWidow = static_cast<CEventObject*>(pGameObject);
	pGameObject->Set_Active(false);

	GemSequence.clear();
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_LEFT);
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_DOWN);
	fCoolTime = 240.f;
	m_pGameInstance->Add_Gem(ORBITAL_380, GemSequence, fCoolTime, pWidow);


	// SENTRY_GATLING
	// 딜레이 7.75초, 사거리 75m
	// 탄약 수 500, 선회 속도 96
	GemSequence.clear();
	GemSequence.push_back(ARROW_DOWN);
	GemSequence.push_back(ARROW_UP);
	GemSequence.push_back(ARROW_RIGHT);
	GemSequence.push_back(ARROW_LEFT);
	fCoolTime = 150.f;
	m_pGameInstance->Add_Gem(SENTRY_GATLING, GemSequence, fCoolTime, pHornet);


	return S_OK;
}

void CLevel_GamePlay::Load_Objects()
{
	_ulong dwByte = {};
	HANDLE hFile = CreateFile(L"../Bin/DataFiles/Map_Objects.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	MAP_OBJECT_TYPE eObject{};
	_uint iType{};
	_float4x4 tempWorldMatrix{};
	CGameObject* pGameObject{ nullptr };
	const _tchar* pLayerTag = {};

	while (true)
	{
		ReadFile(hFile, &eObject, sizeof(_uint), &dwByte, nullptr);
		ReadFile(hFile, &iType, sizeof(_uint), &dwByte, nullptr);
		ReadFile(hFile, &tempWorldMatrix, sizeof(_float4x4), &dwByte, nullptr);

		if (dwByte == 0)
			break;

		if (OBJ_TYPE_INSTALL == eObject)
		{
			pLayerTag = TEXT("Layer_Installation");
			CMapObject::MAPOBJECT_DESC Desc = {};
			Desc.iLevelID = LEVEL_GAMEPLAY;
			Desc.fSpeedPerSec = 0.f;
			Desc.fRotationPerSec = 0.f;
			switch (iType)
			{
			case INSTALL_ROCK:
				break;
			case INSTALL_HELIPAD:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Helipad"), LEVEL_GAMEPLAY, TEXT("Layer_Evacuate"), &Desc);
				break;
				//case INSTALL_DROPPED_AIRCRAFT:
				//	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_DroppedAircraft"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				//	break;
			case INSTALL_ILLEGAL_BUILDING:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_IllegalBuilding"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_ILLEGAL_RADAR:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_IllegalRadar"), LEVEL_GAMEPLAY, TEXT("Layer_Quest"), &Desc);
				break;

			case INSTALL_FB_ROCKWALL0:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_FB_RockWall_0"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_FB_ROCKWALL1:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_FB_RockWall_1"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_FB_ROCKWALL2:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_FB_RockWall_2"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_FB_ROCKWALL3:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_FB_RockWall_3"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;

			case INSTALL_GL_BRICK:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Brick"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_GL_BRICKS0:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Bricks0"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
				//case INSTALL_GL_BRICKS1:
				//	pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Bricks1"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				//	break;
			case INSTALL_GL_ROCK1:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock1"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_GL_ROCK2:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock2"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_GL_ROCK3:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock3"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;

			case INSTALL_GR_BUILDING:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Building"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_GR_RUINED:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Ruined"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_GR_IRONWALL0:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_IronWall0"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_GR_IRONWALL1:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_IronWall1"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_GR_IRONWALL2:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_IronWall2"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;

			case INSTALL_HS_CLIFF0:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Cliff0"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_CLIFF1:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Cliff1"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_CLIFF2:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Cliff2"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_ROCK4:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock4"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_ROCK5:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock5"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_ROCK6:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Rock6"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;

			case INSTALL_HS_BIGROCK:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_BigRock"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_SMALLROCK:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_SmallRock"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_LOWROCKWALL0:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_LowRockWall0"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_LOWROCKWALL1:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_LowRockWall1"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_LOWROCKWALL2:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_LowRockWall2"), LEVEL_GAMEPLAY, pLayerTag, &Desc);
				break;
			case INSTALL_HS_FLAGPOLE:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_FlagPole"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			case INSTALL_HS_TOWER:
				pGameObject = m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Tower"), LEVEL_GAMEPLAY, TEXT("Layer_Collide"), &Desc);
				break;
			}

			CModel* pModel = static_cast<CModel*>(pGameObject->Find_Component(L"Com_Model"));
			if (nullptr == pModel)
			{
				list<CGameObject*>* pGameObjectList = m_pGameInstance->Find_GameObject_List_In_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Installation"));
				for (auto iter = pGameObjectList->begin(); iter != pGameObjectList->end();)
				{
					if (*iter == pGameObject)
					{
						Safe_Release(*iter);
						*iter = nullptr;
						pGameObject = nullptr;
						iter = pGameObjectList->erase(iter);
					}
					else
						iter++;
				}
				break;
			}
			CTransform* pTransform = static_cast<CTransform*>(pGameObject->Find_Component(Engine::g_strTransformTag));
			for (size_t i = 0; i < CTransform::STATE_END; i++)
			{
				_float4 vState{};
				memcpy(&vState, &tempWorldMatrix.m[i], sizeof(_float4));
				pTransform->Set_State((CTransform::STATE)i, XMLoadFloat4(&vState));
			}
		}
	}

	CloseHandle(hFile);
}

void CLevel_GamePlay::Load_Grass()
{
	_ulong dwByte = {};
	HANDLE hFile = CreateFile(L"../Bin/DataFiles/Grass.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	_int iGrassNum{};
	ReadFile(hFile, &iGrassNum, sizeof(_int), &dwByte, nullptr);

	CGrass::GRASS_DESC Desc{};

	for (size_t i = 0; i < iGrassNum; i++)
	{
		ReadFile(hFile, &Desc.iNumTexture, sizeof(_uint), &dwByte, nullptr);
		ReadFile(hFile, &Desc.iPattern, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.isLoop, sizeof(_bool), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.iNumInstance, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vRange, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vCenter, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vSize, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.fFinalSize, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vSpeed, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vPivot, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vLifeTime, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.vStartColor, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.vEndColor, sizeof(_float3), &dwByte, nullptr);

		if (nullptr == m_pGameInstance->Add_GameObject_To_Layer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Grass"), LEVEL_GAMEPLAY, TEXT("Layer_Grass"), &Desc))
			return;
	}
	CloseHandle(hFile);

}

HRESULT CLevel_GamePlay::Load_Effects()
{
	HANDLE hFile = CreateFile(L"../Bin/DataFiles/Effects/ExplBig.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	CPoolEffect::EFFECTPOOL_DESC Desc{};
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::EXPL_BIG, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ExplBig"), 10, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ExplMiddle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::EXPL_MIDDLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ExplMiddle"), 50, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ExplSmall.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::EXPL_SMALL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ExplSmall"), 100, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ExplTiny.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::EXPL_TINY, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ExplTiny"), 40, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SmokeTrailTiny.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SMOKE_FOLLOW_TINY, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SmokeFollowTiny"), 50, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SmokeTrailSmall.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SMOKE_FOLLOW_SMALL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SmokeFollowSmall"), 50, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/LaserStrata.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::STRATA_LASER, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_StrataLaser"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/LaserStrata_Blue.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::STRATA_LASER_BLUE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_StrataLaser_Blue"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/LaserStrata_Red.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::STRATA_LASER_RED, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_StrataLaser_Red"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SmokeSpreadBig.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SMOKE_SPREAD_BIG, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SmokeSpreadBig"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SmokeSpreadMiddle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SMOKE_SPREAD_MIDDLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SmokeSpreadMiddle"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SmokeSpreadSmall.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SMOKE_SPREAD_SMALL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SmokeSpreadSmall"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/AshGrayAlot.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::ASH_GRAY_ALOT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_AshGrayAlot"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ScatterBig.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SCATTER_BIG, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ScatterBig"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ScatterMiddle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SCATTER_MIDDLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ScatterMiddle"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletCircle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_CIRCLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletCircle"), 200, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletSpark.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_SPARK, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletSpark"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletDust.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_DUST, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletDust"), 200, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletMuzzle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_MUZZLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletMuzzle"), 40, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/FireEngine.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::FIRE_ENGINE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_FireEngine"), 10, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/DropshipEngine.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::FIRE_ENGINE_BIG, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_FireDropship"), 10, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SparkSprite.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SPARK_SPRITE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SparkSprite"), 10, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/SparkParticle.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::SPARK_PARTICLE, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_SparkParticle"), 10, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletTrail.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_TRAIL, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletTrail"), 20, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/BulletTrail_Red.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BULLET_TRAIL_RED, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BulletTrail_Red"), 100, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ReaperBall.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BALL_LOOP, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BallEffect"), 200, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/ExplEffect.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::EXPL_EFFECT, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_ExplEffect"), 100, &Desc)))
		return E_FAIL;

	hFile = CreateFile(L"../Bin/DataFiles/Effects/Blood.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Read_EffectFile(hFile, Desc);
	if (FAILED(m_pGameInstance->Effect_Set(CPoolEffect::BLOOD_SPLATTER, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect_BloodParticle"), 10, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Read_EffectFile(HANDLE hFile, CPoolEffect::EFFECTPOOL_DESC& Desc)
{
	_ulong dwByte = {};
	_int iType{};
	_uint iTextureIndex{};
	while (true)
	{
		ReadFile(hFile, &iType, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &iTextureIndex, sizeof(_uint), &dwByte, nullptr);
		ReadFile(hFile, &Desc.iPattern, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.isLoop, sizeof(_bool), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.iNumInstance, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vRange, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vCenter, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vSize, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.fFinalSize, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vSpeed, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vPivot, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.ParticleDesc.vLifeTime, sizeof(_float2), &dwByte, nullptr);
		ReadFile(hFile, &Desc.vStartColor, sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &Desc.vEndColor, sizeof(_float3), &dwByte, nullptr);

		if (0 == iType)
			Desc.bPlayUVAnim = false;
		else
		{
			Desc.bPlayUVAnim = true;
			ReadFile(hFile, &Desc.iNumAtlas, sizeof(_int), &dwByte, nullptr);
			ReadFile(hFile, &Desc.vNumAtlas, sizeof(_int2), &dwByte, nullptr);
		}

		if (dwByte == 0)
			break;
	}
	CloseHandle(hFile);
	return S_OK;
}

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed To Created : CLevel_GamePlay");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_GamePlay::Free()
{
	__super::Free();

	if (nullptr != m_pTerrain)
		Safe_Release(m_pTerrain);

}
