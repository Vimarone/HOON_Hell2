#include "pch.h"
#include "Level_EffectTool.h"

#include "GameInstance.h"

#include "Camera_Free.h"
#include "Sky.h"

#include "Tutorial_Terrain.h"
#include "Dummy_Effect.h"
#include "Dummy_Mesh.h"
#include "Dummy_MeshInst.h"

CLevel_EffectTool::CLevel_EffectTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_EffectTool::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Environment(TEXT("Layer_Environment"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	if (FAILED(Ready_Images()))
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	// m_pGameInstance->Set_IsRenderFog(true);

	m_fActiveDelay = 0.f;
	m_iNumInstance = 1000;
	m_vRange = _float3{ 10.f, 10.f, 10.f };
	m_vSize = _float2(1.f, 4.f);
	m_vSpeed = _float2(2.f, 4.f);
	m_vLifeTime = _float2(1.f, 1.f);
	m_vMulColor = _float3(1.f, 1.f, 1.f);
	m_vAddColor = _float3(0.f, 0.f, 0.f);
	m_fFinalSize = 1.f;

	for (size_t i = 0; i < TEX_END; i++)
	{
		m_bOptionEnable[i] = false;
		m_iOptionTextureIndex[i] = 0;
	}
	m_vUVStart = _float2(0.f, 0.f);
	m_vUVTileCount = _float2(1.f, 1.f);
	m_vNumAtlas = _int2(1, 1);
	m_iNumAtlas = 1;
	m_vMeshTransform = _float3(0.f, 0.f, 0.f);
	m_vMeshScale = _float3(1.f, 1.f, 1.f);

	return S_OK;
}

void CLevel_EffectTool::Update(_float fTimeDelta)
{
	if (ImGui::Begin("Editor", NULL, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Effect File"))
			{
				if (ImGui::MenuItem("Save File"))
					Save_Effect();

				ImGui::Separator();

				if (ImGui::MenuItem("Load File"))
					Load_Effect();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		_bool bGuiHover = { false };
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered())
			bGuiHover = true;

		ImGui::Separator();
		// 라디오 버튼(2) : 파티클 & 스프라이트
		ImGui::Text("Type :"); ImGui::SameLine();
		ImGui::RadioButton("Particle", &m_iImguiSelectedType, 0); ImGui::SameLine();
		ImGui::RadioButton("Sprite", &m_iImguiSelectedType, 1); ImGui::SameLine();
		ImGui::RadioButton("Mesh", &m_iImguiSelectedType, 2); ImGui::SameLine();
		ImGui::RadioButton("Trail", &m_iImguiSelectedType, 3); ImGui::SameLine();
		ImGui::RadioButton("MeshInstance", &m_iImguiSelectedType, 4);

		// 파티클 선택 시
		if (CEffectObject::PARTICLE == m_iImguiSelectedType)
		{
			ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30);
			ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
			for (_int i = 0; i < m_pParticleTextures.size(); i++)
			{
				_char szName[100];
				sprintf_s(szName, sizeof(szName), "Particle%d", i);
				if (true == ImGui::ImageButton(szName, (ImTextureID)m_pParticleTextures[i]->Get_SRV(0), ImVec2(64, 64)))
					m_iTextureIndex = i;
				if (i % 4 < 3)
					ImGui::SameLine();
			}
			ImGui::EndChild();
		}
		// 스프라이트 선택 시
		else if (CEffectObject::SPRITE == m_iImguiSelectedType)
		{
			ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30);
			ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
			for (_int i = 0; i < m_pSpriteTextures.size(); i++)
			{
				_char szName[100];
				sprintf_s(szName, sizeof(szName), "Sprite%d", i);
				if (true == ImGui::ImageButton(szName, (ImTextureID)m_pSpriteTextures[i]->Get_SRV(0), ImVec2(64, 64)))
					m_iTextureIndex = i;
				if (i % 4 < 3)
					ImGui::SameLine();
			}
			ImGui::EndChild();
		}
		// 메쉬 선택 시
		else if(CEffectObject::MESH == m_iImguiSelectedType || CEffectObject::MESH_INSTANCE == m_iImguiSelectedType)
		{
			const char* items[] =
			{
				"Blade", "DNA", "Plane_Circle", "Plane_Rect", "Ring00", "Ring01", "Roll00",
				"Crystal_Part00", "Crystal_Part01","Crystal_Part02",
				"Dome_Plane00", "Dome_Plane01","Dome_Plane02","Dome_Plane03",
				"Dome00", "Dome01", "Dome02", "Dome03", "Dome04", "Dome05",
				"MoonShell", "Barnacle", "Pedestal",
				"Screw00", "Screw01", "Screw02", "Screw03", "Screw04",
				"Hand", "Scary_Face", "Topoda_Punch",
				"Tentacle00", "Tentacle01", "Tentacle02",
				"Sphere00", "Sphere01",
				"ShellWind00", "ShellWind01",
				"WindImpact", "DoomScrew",
				"Impact", "WindImpact_Ground",
				"Broken_Shell", "Broken_Glass",
				"Wind_Cannon001", "Wind_Cannon003",
				"ButterflyFish","SurgeonFish", /* 환경 */
				"Cylinder_Steam", "Cylinder_Drop",
				"Magnet_Tube", "Rusty_Metal",
				"Crystal_Shard0", "Crystal_Shard1",
				"Urchin_Bomb", "Tornado", "Water_Splash",
			};
			ImGui::Combo("Pattern", &m_iSelectedMeshType, items, IM_ARRAYSIZE(items));
		}
		// 트레일 선택 시
		else
		{
			const char* items[] =
			{
				"Trail_Hammer",
				"Trail00", "Trail01", "Trail02", "Trail03", "Trail04", 
				"Trail05", "Trail06", "Trail07", "Trail08", "Trail09", 
				"Trail10", "Trail11", "Trail12", "Trail13", 
				"Trail_Golf",
				"Daoguang05", "Daoguang005", "Blade_DoubleSide", "Blade_SingleSide", "Blade001"

			};
			ImGui::Combo("Pattern", &m_iSelectedMeshType, items, IM_ARRAYSIZE(items));
		}

		ImGui::NewLine();
		ImGui::Separator();
		// 베이스(메쉬), 마스크, 노이즈, Emissive 선택 기능
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
		{
			ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30);
			if (CEffectObject::MESH == m_iImguiSelectedType || CEffectObject::MESH_TRAIL == m_iImguiSelectedType || CEffectObject::MESH_INSTANCE == m_iImguiSelectedType)
			{
				if (ImGui::BeginTabItem("Diffuse"))
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pBaseTextures.size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Base%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pBaseTextures[i]->Get_SRV(0), ImVec2(64, 64)))
							m_iTextureIndex = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Mask"))
			{
				ImGui::Checkbox("Mask Enable", &m_bOptionEnable[TEX_MASK]);
				if (true == m_bOptionEnable[TEX_MASK])
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pOptionTextures[TEX_MASK].size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Mask%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pOptionTextures[TEX_MASK][i]->Get_SRV(0), ImVec2(64, 64)))
							m_iOptionTextureIndex[TEX_MASK] = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Noise"))
			{
				ImGui::Checkbox("Noise Enable", &m_bOptionEnable[TEX_NOISE]);
				if (true == m_bOptionEnable[TEX_NOISE])
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pOptionTextures[TEX_NOISE].size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Noise%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pOptionTextures[TEX_NOISE][i]->Get_SRV(0), ImVec2(64, 64)))
							m_iOptionTextureIndex[TEX_NOISE] = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Dissolve"))
			{
				ImGui::Checkbox("Dissolve Enable", &m_bOptionEnable[TEX_DISSOLVE]);
				if (true == m_bOptionEnable[TEX_DISSOLVE])
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pOptionTextures[TEX_NOISE].size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Dissolve%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pOptionTextures[TEX_NOISE][i]->Get_SRV(0), ImVec2(64, 64)))
							m_iOptionTextureIndex[TEX_DISSOLVE] = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Emissive"))
			{

				ImGui::Checkbox("Emissive(Bloom&Glow) Enable", &m_bOptionEnable[TEX_EMISSIVE]);
				if (true == m_bOptionEnable[TEX_EMISSIVE])
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pOptionTextures[TEX_EMISSIVE].size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Emissive%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pOptionTextures[TEX_EMISSIVE][i]->Get_SRV(0), ImVec2(64, 64)))
							m_iOptionTextureIndex[TEX_EMISSIVE] = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Distortion"))
			{
				ImGui::Checkbox("Distortion Enable", &m_bOptionEnable[TEX_DISTORTION]);
				if (true == m_bOptionEnable[TEX_DISTORTION])
				{
					_char szName[100];
					ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
					for (_int i = 0; i < m_pOptionTextures[TEX_NOISE].size(); i++)
					{
						sprintf_s(szName, sizeof(szName), "Distortion%d", i);
						if (true == ImGui::ImageButton(szName, (ImTextureID)m_pOptionTextures[TEX_NOISE][i]->Get_SRV(0), ImVec2(64, 64)))
							m_iOptionTextureIndex[TEX_DISTORTION] = i;
						if (i % 4 < 3)
							ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
	}
	ImGui::End();

	if (ImGui::Begin("Editor2", NULL, ImGuiWindowFlags_MenuBar))
	{
		ImGui::Checkbox("Blend Enable", &m_bBlendEnable);
		ImGui::SliderFloat("Active Delay", &m_fActiveDelay, 0.1f, 10.f);
		ImGui::ColorEdit3("Multiply Color", reinterpret_cast<_float*>(&m_vMulColor));
		ImGui::ColorEdit3("Additive Color", reinterpret_cast<_float*>(&m_vAddColor));
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		const char* szPatterns[] = 
		{ 
			"Explosion", "Drop", "Rise", "Fraction", "Hold", 
			"Rise_Water", "Charging", "Rise_Water_Env", 
			"Ash_Flow", "Cross_Spread", "God_Ray",
			"Expl_SlowDown", "Expl_ScaleGradient",
		};

		const char* szMeshInstPatterns[] =
		{
			"Fraction","Expl_SlowDown","Around", "Rise", "Hold", 
		};

		switch (m_iImguiSelectedType)
		{
		case CEffectObject::PARTICLE:
		case CEffectObject::SPRITE:
			ImGui::InputInt("All Atlas Num", &m_iNumAtlas);
			ImGui::InputInt2("Atlas Nums", reinterpret_cast<_int*>(&m_vNumAtlas));
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			if (m_vSpeed.x > m_vSpeed.y)
				m_vSpeed.y = m_vSpeed.x;
			ImGui::Combo("Pattern", &m_iSelectedPattern, szPatterns, IM_ARRAYSIZE(szPatterns));
			ImGui::Checkbox("Loop", &m_bIsLoop);
			ImGui::SliderInt("Instance Num", &m_iNumInstance, 1, 2000);
			ImGui::SliderFloat3("Range", reinterpret_cast<_float*>(&m_vRange), 0.f, 100.f);
			ImGui::SliderFloat3("Center", reinterpret_cast<_float*>(&m_vCenter), -100.f, 100.f);
			ImGui::SliderFloat2("Size", reinterpret_cast<_float*>(&m_vSize), 0.001f, 200.f);
			ImGui::SliderFloat("Final Size", &m_fFinalSize, 0.001f, 100.f);
			ImGui::SliderFloat2("Speed", reinterpret_cast<_float*>(&m_vSpeed), 0.1f, 1000.f);
			ImGui::SliderFloat3("Pivot", reinterpret_cast<_float*>(&m_vPivot), -100.f, 100.f);
			ImGui::SliderFloat2("LifeTime", reinterpret_cast<_float*>(&m_vLifeTime), 0.001f, 10.f);
			ImGui::Separator();
			break;

		case CEffectObject::MESH:
		case CEffectObject::MESH_TRAIL:
			/* 이펙트 공통 설정 */
			ImGui::SliderFloat3("Center", reinterpret_cast<_float*>(&m_vCenter), -100.f, 100.f);
			ImGui::SliderFloat("LifeTime", &m_vLifeTime.x, 0.01f, 10.f);

			/* 초기 트랜스폼 설정 */
			ImGui::SliderFloat3("Transform", reinterpret_cast<_float*>(&m_vMeshTransform), -100.f, 100.f);
			ImGui::SliderFloat3("Scale", reinterpret_cast<_float*>(&m_vMeshScale), -100.f, 100.f);
			

			/* 트랜스폼 애니메이션 : 이동, 회전, 스케일링 */
			// m_iSelectedPattern
			ImGui::Text("Transform Animation Type");
			ImGui::Checkbox("Translation", &m_bTranslationAnim); ImGui::SameLine();
			ImGui::Checkbox("Rotation", &m_bRotationAnim); ImGui::SameLine();
			ImGui::Checkbox("Scaling", &m_bScalingAnim);
			ImGui::Checkbox("Loop", &m_bIsLoop);

			if (true == m_bTranslationAnim)
				ImGui::SliderFloat2("Translation Speed", &m_vSpeed.x, 0.1f, 1000.f);

			if (true == m_bRotationAnim)
				ImGui::SliderFloat2("Rotation Speed", &m_vSpeed.y, 0.1f, 1000.f);

			if (true == m_bScalingAnim)
				ImGui::SliderFloat("Final Size", &m_fFinalSize, 0.1f, 100.f);

			/* UV 애니메이션 */

			ImGui::Checkbox("UV Animation", &m_bUVAnim);
			if (true == m_bUVAnim)
			{
				// m_vUVStart : 이펙트의 Base 텍스쳐 UV 좌표의 샘플링 시작 위치 결정
				ImGui::SliderFloat2("UV Start", reinterpret_cast<_float*>(&m_vUVStart), 0.f, 1.f);
				// m_vUVSpeed : UV 좌표의 샘플링 위치 변화 속도 결정
				ImGui::SliderFloat2("UV Speed", reinterpret_cast<_float*>(&m_vUVSpeed), -10.f, 10.f);
				// m_vUVTileCount : UV 좌표의 샘플링 배율 결정
				ImGui::SliderFloat2("UV TileCount", reinterpret_cast<_float*>(&m_vUVTileCount), 0.1f, 10.f);
			}

			if(CEffectObject::MESH_TRAIL)
				ImGui::SliderFloat("Max Trail Length", &m_fMaxTrailLength, 0.f, 1.f);

			ImGui::Separator();
			break;

		case CEffectObject::MESH_INSTANCE:
			ImGui::Checkbox("Loop", &m_bIsLoop);
			// ImGui::Checkbox("Apply Billboard", &m_bBillboard);

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			/* 오브젝트 트랜스폼 */
			ImGui::SliderFloat3("Object Transform", reinterpret_cast<_float*>(&m_vMeshTransform), -100.f, 100.f);

			/* 파티클 제어 */
			ImGui::Combo("Pattern", &m_iSelectedPattern, szMeshInstPatterns, IM_ARRAYSIZE(szMeshInstPatterns));
			ImGui::SliderInt("Instance Num", &m_iNumInstance, 1, 2000);

			ImGui::SliderFloat3("Range", reinterpret_cast<_float*>(&m_vRange), 0.f, 100.f);
			ImGui::SliderFloat3("Center", reinterpret_cast<_float*>(&m_vCenter), -100.f, 100.f);
			ImGui::SliderFloat3("Pivot", reinterpret_cast<_float*>(&m_vPivot), -100.f, 100.f);
			ImGui::SliderFloat2("Speed", reinterpret_cast<_float*>(&m_vSpeed), 0.1f, 100.f);
			ImGui::SliderFloat2("LifeTime", reinterpret_cast<_float*>(&m_vLifeTime), 0.1f, 10.f);
			if (m_vSpeed.x > m_vSpeed.y)
				m_vSpeed.y = m_vSpeed.x;

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			
			ImGui::SliderFloat3("Particle Scaling Size", reinterpret_cast<_float*>(&m_vMeshScale), 0.f, 100.f);
			ImGui::SliderFloat("Final Scale", &m_fFinalSize, 0.001f, 100.f);

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::RadioButton("Right Axis", &m_iRotationAxis, 0); ImGui::SameLine();
			ImGui::RadioButton("Up Axis", &m_iRotationAxis, 1); ImGui::SameLine();
			ImGui::RadioButton("Look Axis", &m_iRotationAxis, 2);

			ImGui::SliderFloat("Particle Rotation Speed", &m_fParticleRotation, 0.f, 180.f);
			ImGui::SliderFloat("Final Rotation Speed", &m_fParticleFinalRotation, 0.f, 180.f);

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			/* UV 애니메이션 */

			ImGui::Checkbox("UV Animation", &m_bUVAnim);
			if (true == m_bUVAnim)
			{
				// m_vUVStart : 이펙트의 Base 텍스쳐 UV 좌표의 샘플링 시작 위치 결정
				ImGui::SliderFloat2("UV Start", reinterpret_cast<_float*>(&m_vUVStart), 0.f, 1.f);
				// m_vUVSpeed : UV 좌표의 샘플링 위치 변화 속도 결정
				ImGui::SliderFloat2("UV Speed", reinterpret_cast<_float*>(&m_vUVSpeed), -10.f, 10.f);
				// m_vUVTileCount : UV 좌표의 샘플링 배율 결정
				ImGui::SliderFloat2("UV TileCount", reinterpret_cast<_float*>(&m_vUVTileCount), 0.1f, 10.f);
			}
			break;
		default:
			break;
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		if (true == m_bOptionEnable[TEX_DISSOLVE])
			ImGui::SliderFloat2("DissolveStart", reinterpret_cast<_float*>(&m_vDissolveStart), 0.f, 1.f);

		if (true == m_bOptionEnable[TEX_NOISE]) {}
		if (true == m_bOptionEnable[TEX_EMISSIVE]) {}
		if (true == m_bOptionEnable[TEX_DISTORTION]) 
		{

		}
		
		if (ImGui::Button("Adjust To Current Effect"))
			Adjust_Effect();
	}
	ImGui::End();
}

HRESULT CLevel_EffectTool::Render()
{
	//#ifdef _DEBUG
	//    SetWindowText(g_hWnd, L"게임 플레이 레벨이빈다.");
	//#endif

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Layer_Environment(const _wstring& strLayerTag)
{
	CSky::DESC SkyDesc{};
	SkyDesc.iTextureIndex = 3;
	if (FAILED(m_pGameInstance->
		Add_GameObject_To_Layer(LEVELID::LEVEL_VILLAGE, L"Prototype_GameObject_Sky", LEVELID::LEVEL_VILLAGE, strLayerTag, &SkyDesc))) // 얘도 사실 Static 에 있어도 되는거 아니야?
		return E_FAIL;

	if (FAILED(m_pGameInstance->
		Add_GameObject_To_Layer(LEVEL_VILLAGE, TEXT("Prototype_GameObject_Tutorial_Terrain"), LEVEL_VILLAGE, strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Layer_Camera(const _wstring& strLayerTag)
{
	CCamera_Free::DESC CameraFreeDesc{};
	{
		CameraFreeDesc.CameraDesc.GameObjectDesc.TransformDesc.fSpeedPerSec = 10.f;
		CameraFreeDesc.CameraDesc.GameObjectDesc.TransformDesc.fRotationPerSec = XMConvertToRadians(90.f);

		CameraFreeDesc.CameraDesc.vEye = _float3(0.f, 10.f, -7.f);
		CameraFreeDesc.CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
		CameraFreeDesc.CameraDesc.fFovY = XMConvertToRadians(60.f);
		CameraFreeDesc.CameraDesc.fNear = 0.1f;
		CameraFreeDesc.CameraDesc.fFar = 1000.f;

		// CameraFreeDesc.fMouseSensitivity = 0.005f;
		CameraFreeDesc.fMouseSensitivity = 0.1f;
	}

	if (FAILED(m_pGameInstance->
		Add_GameObject_To_Layer(LEVEL_VILLAGE, TEXT("Prototype_GameObject_Camera_Free"), LEVEL_VILLAGE, strLayerTag, &CameraFreeDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Layer_Effect(const _wstring& strLayerTag)
{
	return S_OK;
}

void CLevel_EffectTool::Save_Effect()
{
	TCHAR szCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDir);

	OPENFILENAME FileName;
	_tchar szFile[MAX_PATH] = {};

	ZeroMemory(&FileName, sizeof FileName);
	FileName.lStructSize = sizeof FileName;
	FileName.hwndOwner = NULL;
	FileName.lpstrFilter = TEXT("데이터 파일 (*.dat)\0*.dat\0");
	FileName.lpstrFile = szFile;
	FileName.nMaxFile = MAX_PATH;
	FileName.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&FileName))
	{
		_ulong dwByte = {};
		HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// 타입 저장
		WriteFile(hFile, &m_iImguiSelectedType, sizeof(_int), &dwByte, nullptr);
		WriteFile(hFile, &m_iTextureIndex, sizeof(_uint), &dwByte, nullptr);

		_tchar szName[MAX_PATH]{};
		if (CEffectObject::PARTICLE == m_iImguiSelectedType)
		{
			// 베이스 텍스쳐 정보(프로토타입 이름) 저장 : m_ParticleTexPrototypeName
			const _tchar* szBaseTexProtoName = m_ParticleTexPrototypeName[m_iTextureIndex].c_str();
			lstrcpy(szName, szBaseTexProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			Save_Buffer_Data(hFile, dwByte);
			static_cast<CSpriteEffect*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))->Save_Effect(hFile, dwByte);
		}
		else if (CEffectObject::SPRITE == m_iImguiSelectedType)
		{
			const _tchar* szBaseTexProtoName = m_SpriteTexPrototypeName[m_iTextureIndex].c_str();
			lstrcpy(szName, szBaseTexProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			Save_Buffer_Data(hFile, dwByte);
			static_cast<CSpriteEffect*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))->Save_Effect(hFile, dwByte);
		}
		else if (CEffectObject::MESH == m_iImguiSelectedType)
		{
			WriteFile(hFile, &m_iSelectedMeshType, sizeof(_uint), &dwByte, nullptr);
			WriteFile(hFile, &m_vMeshTransform, sizeof(_float3), &dwByte, nullptr);
			WriteFile(hFile, &m_vMeshScale, sizeof(_float3), &dwByte, nullptr);
			const _tchar* szMeshProtoName = m_ModelPrototypeName[m_iSelectedMeshType].c_str();
			lstrcpy(szName, szMeshProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			const _tchar* szBaseTexProtoName = m_BaseTexPrototypeName[m_iTextureIndex].c_str();
			lstrcpy(szName, szBaseTexProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			static_cast<CMeshEffect*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))->Save_Effect(hFile, dwByte);
		}
		else if (CEffectObject::MESH_TRAIL == m_iImguiSelectedType)
		{
			WriteFile(hFile, &m_iSelectedMeshType, sizeof(_uint), &dwByte, nullptr);
			WriteFile(hFile, &m_vMeshTransform, sizeof(_float3), &dwByte, nullptr);
			WriteFile(hFile, &m_vMeshScale, sizeof(_float3), &dwByte, nullptr);
			const _tchar* szMeshProtoName = m_TrailPrototypeName[m_iSelectedMeshType].c_str();
			lstrcpy(szName, szMeshProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			const _tchar* szBaseTexProtoName = m_BaseTexPrototypeName[m_iTextureIndex].c_str();
			lstrcpy(szName, szBaseTexProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			static_cast<CMeshEffect*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))->Save_Effect(hFile, dwByte);
		}
		else if (CEffectObject::MESH_INSTANCE == m_iImguiSelectedType)
		{
			WriteFile(hFile, &m_iSelectedMeshType, sizeof(_uint), &dwByte, nullptr);
			WriteFile(hFile, &m_vMeshTransform, sizeof(_float3), &dwByte, nullptr);
			const _tchar* szMeshProtoName = m_ModelPrototypeName[m_iSelectedMeshType].c_str();
			lstrcpy(szName, szMeshProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			const _tchar* szBaseTexProtoName = m_BaseTexPrototypeName[m_iTextureIndex].c_str();
			lstrcpy(szName, szBaseTexProtoName);
			WriteFile(hFile, &szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			static_cast<CMeshInstEffect*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))->Save_Effect(hFile, dwByte);
			Save_InstancingBuffer_Data(hFile, dwByte);
		}

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}
	SetCurrentDirectory(szCurrentDir);
}

void CLevel_EffectTool::Load_Effect()
{
	if (nullptr != m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))
		m_pGameInstance->Delete_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect"));

	TCHAR szCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDir);

	OPENFILENAME FileName;
	_tchar szFile[MAX_PATH] = {};

	ZeroMemory(&FileName, sizeof FileName);
	FileName.lStructSize = sizeof FileName;
	FileName.hwndOwner = NULL;
	FileName.lpstrFilter = TEXT("데이터 파일 (*.dat)\0*.dat\0");
	FileName.lpstrFile = szFile;
	FileName.nMaxFile = MAX_PATH;
	FileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;


	if (GetOpenFileName(&FileName))
	{
		_ulong dwByte = {};
		HANDLE hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		// 타입 저장
		ReadFile(hFile, &m_iImguiSelectedType, sizeof(_int), &dwByte, nullptr);
		ReadFile(hFile, &m_iTextureIndex, sizeof(_uint), &dwByte, nullptr);
		
		_tchar szName[MAX_PATH]{};
		if (CEffectObject::PARTICLE == m_iImguiSelectedType || CEffectObject::SPRITE == m_iImguiSelectedType)
		{
			// 베이스 텍스쳐 정보(프로토타입 이름) 로드
			ReadFile(hFile, szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			CSpriteEffect::DESC Desc{};
			Desc.eLevelID = LEVEL_VILLAGE;
			Desc.strTextureTag = szName;
			ReadFile(hFile, &m_iNumInstance, sizeof(_int), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.iNumInstance = m_iNumInstance;
			ReadFile(hFile, &m_vRange, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vRange = m_vRange;
			ReadFile(hFile, &m_vCenter, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vCenter = m_vCenter;
			ReadFile(hFile, &m_vSize, sizeof(_float2), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vSize = m_vSize;
			ReadFile(hFile, &m_vSpeed, sizeof(_float2), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vSpeed = m_vSpeed;
			ReadFile(hFile, &m_vPivot, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vPivot = m_vPivot;
			ReadFile(hFile, &m_vLifeTime, sizeof(_float2), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.vLifeTime = m_vLifeTime;
			ReadFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
			Desc.BufferDesc.ParticleDesc.isLoop = m_bIsLoop;

			ReadFile(hFile, &m_iSelectedPattern, sizeof(_int), &dwByte, nullptr);
			Desc.iPattern = m_iSelectedPattern;
			Desc.BufferDesc.ParticleDesc.iPattern = m_iSelectedPattern;
			_int3 vNumAtlas{};
			ReadFile(hFile, &vNumAtlas, sizeof(_int3), &dwByte, nullptr);
			m_vNumAtlas.x = vNumAtlas.x; m_vNumAtlas.y = vNumAtlas.y; m_iNumAtlas = vNumAtlas.z;
			Desc.vNumAtlas = _int2(m_vNumAtlas.x, m_vNumAtlas.y);
			Desc.iNumAtlas = m_iNumAtlas;
			ReadFile(hFile, &Desc.vUVInterval, sizeof(_float2), &dwByte, nullptr);

			ReadFile(hFile, &m_bBlendEnable, sizeof(_bool), &dwByte, nullptr);
			Desc.EffectDesc.bBlendEnable = m_bBlendEnable;
			ReadFile(hFile, &m_fActiveDelay, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fActiveDelay = m_fActiveDelay;
			Desc.EffectDesc.iEffectType = m_iImguiSelectedType;
			ReadFile(hFile, &m_fFinalSize, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fFinalSize = m_fFinalSize;
			ReadFile(hFile, &m_vMulColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vMulColor = m_vMulColor;
			ReadFile(hFile, &m_vAddColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vAddColor = m_vAddColor;
			ReadFile(hFile, &m_iShadingOption, sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iShadingOption = m_iShadingOption;

			if (CEffectObject::MASK & m_iShadingOption)
				m_bOptionEnable[TEX_MASK] = true;
			if (CEffectObject::NOISE & m_iShadingOption)
				m_bOptionEnable[TEX_NOISE] = true;
			if (CEffectObject::DISSOLVE & m_iShadingOption)
				m_bOptionEnable[TEX_DISSOLVE] = true;
			if (CEffectObject::EMISSIVE & m_iShadingOption)
				m_bOptionEnable[TEX_EMISSIVE] = true;
			if (CEffectObject::DISTORTION & m_iShadingOption)
				m_bOptionEnable[TEX_DISTORTION] = true;

			ReadFile(hFile, &m_iOptionTextureIndex[TEX_MASK], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_NOISE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISSOLVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_EMISSIVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISTORTION], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
			ReadFile(hFile, &m_vDissolveStart, sizeof(_float2), &dwByte, nullptr);
			Desc.EffectDesc.vDissolveStart = m_vDissolveStart;
			ReadFile(hFile, &Desc.EffectDesc.fDistortionDissolveStart, sizeof(_float), &dwByte, nullptr);

			SetCurrentDirectory(szCurrentDir);
			m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyEffect", LEVEL_VILLAGE, L"Layer_Effect", &Desc);
		}
		else if (CEffectObject::MESH == m_iImguiSelectedType || CEffectObject::MESH_TRAIL == m_iImguiSelectedType)
		{
			ReadFile(hFile, &m_iSelectedMeshType, sizeof(_uint), &dwByte, nullptr);
			CMeshEffect::DESC Desc{};
			Desc.eLevelID = LEVEL_VILLAGE;
			ReadFile(hFile, &m_vMeshTransform, sizeof(_float3), &dwByte, nullptr);
			Desc.vTranslation = m_vMeshTransform;
			ReadFile(hFile, &m_vMeshScale, sizeof(_float3), &dwByte, nullptr);
			Desc.vScale = m_vMeshScale;
			ReadFile(hFile, szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			Desc.strModelTag = szName;
			ReadFile(hFile, szName, sizeof(_tchar) * MAX_PATH, &dwByte, nullptr);
			Desc.strTextureTag = szName;

			ReadFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
			Desc.bIsLoop = m_bIsLoop;
			ReadFile(hFile, &Desc.iMeshAnimOption, sizeof(_uint), &dwByte, nullptr);
			ReadFile(hFile, &m_vLifeTime.x, sizeof(_float), &dwByte, nullptr);
			Desc.fLifeTime = m_vLifeTime.x;
			ReadFile(hFile, &m_vCenter, sizeof(_float3), &dwByte, nullptr);
			Desc.vCenter = m_vCenter;

			ReadFile(hFile, &m_bUVAnim, sizeof(_bool), &dwByte, nullptr);
			Desc.bUVAnim = m_bUVAnim;
			ReadFile(hFile, &m_vUVStart, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVStart = m_vUVStart;
			ReadFile(hFile, &m_vUVSpeed, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVSpeed = m_vUVSpeed;
			ReadFile(hFile, &m_vUVTileCount, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVTileCount = m_vUVTileCount;
			ReadFile(hFile, &m_fMaxTrailLength, sizeof(_float), &dwByte, nullptr);
			Desc.fMaxTrailLength = m_fMaxTrailLength;

			ReadFile(hFile, &m_bBlendEnable, sizeof(_bool), &dwByte, nullptr);
			Desc.EffectDesc.bBlendEnable = m_bBlendEnable;
			ReadFile(hFile, &m_fActiveDelay, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fActiveDelay = m_fActiveDelay;
			Desc.EffectDesc.iEffectType = m_iImguiSelectedType;
			ReadFile(hFile, &m_fFinalSize, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fFinalSize = m_fFinalSize;
			ReadFile(hFile, &m_vMulColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vMulColor = m_vMulColor;
			ReadFile(hFile, &m_vAddColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vAddColor = m_vAddColor;
			ReadFile(hFile, &m_iShadingOption, sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iShadingOption = m_iShadingOption;

			if (CEffectObject::MASK & m_iShadingOption)
				m_bOptionEnable[TEX_MASK] = true;
			if (CEffectObject::NOISE & m_iShadingOption)
				m_bOptionEnable[TEX_NOISE] = true;
			if (CEffectObject::DISSOLVE & m_iShadingOption)
				m_bOptionEnable[TEX_DISSOLVE] = true;
			if (CEffectObject::EMISSIVE & m_iShadingOption)
				m_bOptionEnable[TEX_EMISSIVE] = true;
			if (CEffectObject::DISTORTION & m_iShadingOption)
				m_bOptionEnable[TEX_DISTORTION] = true;

			ReadFile(hFile, &m_iOptionTextureIndex[TEX_MASK], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_NOISE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISSOLVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_EMISSIVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISTORTION], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
			ReadFile(hFile, &m_vDissolveStart, sizeof(_float2), &dwByte, nullptr);
			Desc.EffectDesc.vDissolveStart = m_vDissolveStart;
			ReadFile(hFile, &Desc.EffectDesc.fDistortionDissolveStart, sizeof(_float), &dwByte, nullptr);
			
			SetCurrentDirectory(szCurrentDir);
			m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyMesh", LEVEL_VILLAGE, L"Layer_Effect", &Desc);
		}
		else if (CEffectObject::MESH_INSTANCE == m_iImguiSelectedType)
		{
			ReadFile(hFile, &m_iSelectedMeshType, sizeof(_uint), &dwByte, nullptr);
			CMeshInstEffect::DESC Desc{};
			Desc.BufferDesc.eLevelID = LEVEL_VILLAGE;
			ReadFile(hFile, &m_vMeshTransform, sizeof(_float3), &dwByte, nullptr);
			Desc.vOriginLocation = m_vMeshTransform;
			ReadFile(hFile, szName, sizeof(_tchar)* MAX_PATH, &dwByte, nullptr);
			Desc.BufferDesc.strModelTag = szName;
			ReadFile(hFile, szName, sizeof(_tchar)* MAX_PATH, &dwByte, nullptr);
			Desc.strTextureTag = szName;

			ReadFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
			Desc.EffectDesc.bIsLoop = m_bIsLoop;
			ReadFile(hFile, &m_vLifeTime.y, sizeof(_float), &dwByte, nullptr);
			Desc.fLifeTime = m_vLifeTime.y;
			ReadFile(hFile, &m_bUVAnim, sizeof(_bool), &dwByte, nullptr);
			Desc.bUVAnim = m_bUVAnim;
			ReadFile(hFile, &m_vUVStart, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVStart = m_vUVStart;
			ReadFile(hFile, &m_vUVSpeed, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVSpeed = m_vUVSpeed;
			ReadFile(hFile, &m_vUVTileCount, sizeof(_float2), &dwByte, nullptr);
			Desc.vUVTileCount = m_vUVTileCount;

			ReadFile(hFile, &m_bBlendEnable, sizeof(_bool), &dwByte, nullptr);
			Desc.EffectDesc.bBlendEnable = m_bBlendEnable;
			ReadFile(hFile, &m_fActiveDelay, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fActiveDelay = m_fActiveDelay;
			Desc.EffectDesc.iEffectType = m_iImguiSelectedType;
			ReadFile(hFile, &m_fFinalSize, sizeof(_float), &dwByte, nullptr);
			Desc.EffectDesc.fFinalSize = m_fFinalSize;
			ReadFile(hFile, &m_vMulColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vMulColor = m_vMulColor;
			ReadFile(hFile, &m_vAddColor, sizeof(_float3), &dwByte, nullptr);
			Desc.EffectDesc.vAddColor = m_vAddColor;
			ReadFile(hFile, &m_iShadingOption, sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iShadingOption = m_iShadingOption;

			if (CEffectObject::MASK & m_iShadingOption)
				m_bOptionEnable[TEX_MASK] = true;
			if (CEffectObject::NOISE & m_iShadingOption)
				m_bOptionEnable[TEX_NOISE] = true;
			if (CEffectObject::DISSOLVE & m_iShadingOption)
				m_bOptionEnable[TEX_DISSOLVE] = true;
			if (CEffectObject::EMISSIVE & m_iShadingOption)
				m_bOptionEnable[TEX_EMISSIVE] = true;
			if (CEffectObject::DISTORTION & m_iShadingOption)
				m_bOptionEnable[TEX_DISTORTION] = true;

			ReadFile(hFile, &m_iOptionTextureIndex[TEX_MASK], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_NOISE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISSOLVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_EMISSIVE], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
			ReadFile(hFile, &m_iOptionTextureIndex[TEX_DISTORTION], sizeof(_uint), &dwByte, nullptr);
			Desc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
			ReadFile(hFile, &m_vDissolveStart, sizeof(_float2), &dwByte, nullptr);
			Desc.EffectDesc.vDissolveStart = m_vDissolveStart;
			ReadFile(hFile, &Desc.EffectDesc.fDistortionDissolveStart, sizeof(_float), &dwByte, nullptr);


			ReadFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
			Desc.BufferDesc.bIsLoop = m_bIsLoop;
			ReadFile(hFile, &m_iSelectedPattern, sizeof(_int), &dwByte, nullptr);
			Desc.BufferDesc.iPattern = m_iSelectedPattern;
			ReadFile(hFile, &m_iNumInstance, sizeof(_int), &dwByte, nullptr);
			Desc.BufferDesc.iNumInstance = m_iNumInstance;
			ReadFile(hFile, &m_vRange, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.vRange = m_vRange;
			ReadFile(hFile, &m_vCenter, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.vCenter = m_vCenter;
			ReadFile(hFile, &m_vPivot, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.vPivot = m_vPivot;
			ReadFile(hFile, &m_vSpeed, sizeof(_float2), &dwByte, nullptr);
			Desc.BufferDesc.vSpeed = m_vSpeed;
			ReadFile(hFile, &m_vLifeTime, sizeof(_float2), &dwByte, nullptr);
			Desc.BufferDesc.vLifeTime = m_vLifeTime;

			ReadFile(hFile, &m_vMeshScale, sizeof(_float3), &dwByte, nullptr);
			Desc.BufferDesc.vScalingSize = m_vMeshScale;
			ReadFile(hFile, &m_fFinalSize, sizeof(_float), &dwByte, nullptr);
			Desc.BufferDesc.fFinalSize = m_fFinalSize;

			ReadFile(hFile, &m_iRotationAxis, sizeof(_int), &dwByte, nullptr);
			Desc.BufferDesc.iRotationAxis = m_iRotationAxis;
			ReadFile(hFile, &m_fParticleRotation, sizeof(_float), &dwByte, nullptr);
			Desc.BufferDesc.fRotationSpeed = m_fParticleRotation;
			ReadFile(hFile, &m_fParticleFinalRotation, sizeof(_float), &dwByte, nullptr);
			Desc.BufferDesc.fRotationFinalSpeed = m_fParticleFinalRotation;

			SetCurrentDirectory(szCurrentDir);
			m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyMeshInst", LEVEL_VILLAGE, L"Layer_Effect", &Desc);
		}

		CloseHandle(hFile);
	}
}

void CLevel_EffectTool::Save_Buffer_Data(HANDLE hFile, _ulong& dwByte)
{
	WriteFile(hFile, &m_iNumInstance, sizeof(_int), &dwByte, nullptr);
	WriteFile(hFile, &m_vRange, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vCenter, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vSize, sizeof(_float2), &dwByte, nullptr);
	WriteFile(hFile, &m_vSpeed, sizeof(_float2), &dwByte, nullptr);
	WriteFile(hFile, &m_vPivot, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vLifeTime, sizeof(_float2), &dwByte, nullptr);
	WriteFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
}

void CLevel_EffectTool::Save_InstancingBuffer_Data(HANDLE hFile, _ulong& dwByte)
{
	WriteFile(hFile, &m_bIsLoop, sizeof(_bool), &dwByte, nullptr);
	WriteFile(hFile, &m_iSelectedPattern, sizeof(_int), &dwByte, nullptr);
	WriteFile(hFile, &m_iNumInstance, sizeof(_int), &dwByte, nullptr);
	WriteFile(hFile, &m_vRange, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vCenter, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vPivot, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_vSpeed, sizeof(_float2), &dwByte, nullptr);
	WriteFile(hFile, &m_vLifeTime, sizeof(_float2), &dwByte, nullptr);
	WriteFile(hFile, &m_vMeshScale, sizeof(_float3), &dwByte, nullptr);
	WriteFile(hFile, &m_fFinalSize, sizeof(_float), &dwByte, nullptr);
	WriteFile(hFile, &m_iRotationAxis, sizeof(_int), &dwByte, nullptr);
	WriteFile(hFile, &m_fParticleRotation, sizeof(_float), &dwByte, nullptr);
	WriteFile(hFile, &m_fParticleFinalRotation, sizeof(_float), &dwByte, nullptr);
}

HRESULT CLevel_EffectTool::Ready_Images()
{
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Snow"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_DefaultParticle"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line002"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line004"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_BubbleOne"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Plastic"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_PlasticDots"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line002d"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash00"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash01"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash02"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash03"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash04"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash05"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_HitFlash00"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_HitFlash01"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_HitFlash02"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_HitFlash03"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GodRay"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_BloodCloud"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_FXT_BubblesMany001"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_TearDrop"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_MusicOn"));
	m_ParticleTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_MusicQuater"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Snow/Snow.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/Default_Particle.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/E_line_002c.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/E_line_004_clamp.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/FXT_bubblesMany_001.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/FXT_line.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/Plastic.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/FXT_dots.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/E_line_002d.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash00.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash01.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash02.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash03.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash04.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash05.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/HitFlash00.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/HitFlash01.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/HitFlash02.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/HitFlash03.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/GodRay.dds"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/Blood_cloud.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/FXT_bubblesMany_001.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/Teardrop.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/MusicOn.png"));
	m_ParticleFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/MusicQuater.png"));
	for (size_t i = 0; i < m_ParticleFilePath.size(); i++)
		m_pParticleTextures.push_back(CTexture::Create(m_pDevice, m_pContext, m_ParticleFilePath[i].c_str(), 1));

	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Lightning"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_RadialGlow"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_DustSmoke"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_EnergySpark1"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_EnergySpark03"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Spark"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Bokeh02"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Flash01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Flash02"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Impact04"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Lightning04"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Poof01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_RadialGlow01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_RadialGlow03"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_RadialGlow04"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Shatter10"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Snap01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Sparks01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Sparks02"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Splat01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Star01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_HitSpark"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Smoke114"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Smoke002"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Smoke103"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Glow005"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark00"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark02"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark03"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark04"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark05"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark06"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark07"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark08"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark09"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark10"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark11"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark12"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark13"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_FXT_trail01"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_WaterSplash025"));
	m_SpriteTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_FXT_BubblesMany001"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Lightning.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Radial_Glow.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/DustSmoke.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/EnergySpark1.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/EnergySpark03.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Spark.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Bokeh_02.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Flash_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Flash_02.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Impact_04.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Lightning_04.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Poof_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_03.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_04.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Shatter_10.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Snap_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Sparks_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Sparks_02.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Splat_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Star_01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/HitSpark.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/E_smoke_114.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/E_smoke_002_a.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/E_smoke_103.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Fx_glow_005.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark00.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark02.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark03.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark04.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark05.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark06.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark07.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark08.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark09.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark10.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark11.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark12.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark13.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/FXT_trail01.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/E_water_025.png"));
	m_SpriteFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/FXT_bubblesMany_001.png"));
	for (size_t i = 0; i < m_SpriteFilePath.size(); i++)
		m_pSpriteTextures.push_back(CTexture::Create(m_pDevice, m_pContext, m_SpriteFilePath[i].c_str(), 1));

	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Vein032"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Vein093"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Vein138"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail0158"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Evelyn002"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Noise017gt"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail01"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail02"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail03"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail10"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Wind18"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail06"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Water"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Water001"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Wind"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Wind26"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Wind29"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Pink"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail31"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line3"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line04"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line07"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Boss10"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Bite"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_LineSpot"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line001"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line008"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line012"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Balde001"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_LineC"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line003"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SmallStarTrail2"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Smoke001"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SmokeSlash"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Sueyoi004"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Trail008"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_ScaryFace"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_DNA"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_MeshGlow137"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark00"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark01"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark02"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark03"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark04"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark05"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark06"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark07"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark08"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark09"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark10"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark11"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark12"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_GlowSpark13"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_MeshGlow001"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SparkFlash03"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Line002"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Blade16"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Eff_MetalScrap"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_MeshNoise01"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_Shatter10"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_ButterflyFish"));
	m_BaseTexPrototypeName.push_back(TEXT("Prototype_Component_Texture_SurgeonFish"));

	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/E_vein_032.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/E_vein_093.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/E_vein_138.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail_0158.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_trail_evelyn_002.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/noise_017_gt.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Trail_01.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Trail_02.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Trail_03.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Trail_10.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Wind_18_add.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail06.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_water.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_water_001.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_wind.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Wind_26_alpha.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Wind_29_alpha.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Pink.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Trail31.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Fx_line_3.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Fx_line_04_alpha.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Fx_line_07.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_BigBoss_sp_10.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_bite.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line_Spot.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line_001_alpha.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line_004.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line_012_c.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Line_balde_001.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line_c.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_line003.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_SmallStarTrail2.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Smoke_001.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_smokeSlash.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_sueyoi_skill_004.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail_008.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/ScaryFace.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/DNA.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/MeshGlow_137.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark00.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark01.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark02.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark03.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark04.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark05.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark06.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark07.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark08.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark09.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark10.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark11.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark12.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/GlowSpark13.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/E_glow_001.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/SparkFlash03.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Particle/E_line_002c.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Blade_16_alpha.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/MetalScrap_basecolor.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/Tex_Noise_01.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/Sprite/Tex_Shatter_10.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/ButterflyFish.png"));
	m_BaseTextureFilePath.push_back(TEXT("../Bin/Resources/Textures/Effects/MeshBase/SurgeonFish.png"));

	for (size_t i = 0; i < m_BaseTextureFilePath.size(); i++)
		m_pBaseTextures.push_back(CTexture::Create(m_pDevice, m_pContext, m_BaseTextureFilePath[i].c_str(), 1));


	_tchar szFilePath[MAX_PATH]{};
	for (_int i = 0; i < 35; i++)
	{
		wsprintf(szFilePath, TEXT("../Bin/Resources/Textures/Effects/Noise/Noise%02d.png"), i);
		m_OptionTextureFilePath[TEX_NOISE].push_back(szFilePath);
	}
	for (size_t i = 0; i < m_OptionTextureFilePath[TEX_NOISE].size(); i++)
		m_pOptionTextures[TEX_NOISE].push_back(CTexture::Create(m_pDevice, m_pContext, m_OptionTextureFilePath[TEX_NOISE][i].c_str(), 1));

	for (_int i = 0; i < 31; i++)
	{
		wsprintf(szFilePath, TEXT("../Bin/Resources/Textures/Effects/Mask/Mask%02d.png"), i);
		m_OptionTextureFilePath[TEX_MASK].push_back(szFilePath);
	}
	for (size_t i = 0; i < m_OptionTextureFilePath[TEX_MASK].size(); i++)
		m_pOptionTextures[TEX_MASK].push_back(CTexture::Create(m_pDevice, m_pContext, m_OptionTextureFilePath[TEX_MASK][i].c_str(), 1));

	for (_int i = 0; i < 133; i++)
	{
		wsprintf(szFilePath, TEXT("../Bin/Resources/Textures/Effects/Emissive/Emissive%03d.png"), i);
		m_OptionTextureFilePath[TEX_EMISSIVE].push_back(szFilePath);
	}
	for (size_t i = 0; i < m_OptionTextureFilePath[TEX_EMISSIVE].size(); i++)
		m_pOptionTextures[TEX_EMISSIVE].push_back(CTexture::Create(m_pDevice, m_pContext, m_OptionTextureFilePath[TEX_EMISSIVE][i].c_str(), 1));


	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Meshes()
{
	// 기타
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Blade");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DNA");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_PlaneCircle");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_PlaneRect");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Ring00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Ring01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Roll00");

	// 크리스탈 조각
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CrystalPart00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CrystalPart01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CrystalPart02");

	// 돔
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DomePlane00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DomePlane01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DomePlane02");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DomePlane03");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome02");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome03");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome04");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Dome05");

	// 소라껍데기
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_EffMoonShell");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Barnacle");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_MoonBrick");

	// 회오리
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Screw00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Screw01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Screw02");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Screw03");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Screw04");

	// 스킬 관련
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Hand");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_ScaryFace");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_TopodaPunch");

	// 촉수
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Tentacle00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Tentacle01");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Tentacle02");

	// 구
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Sphere00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Sphere01");

	// 문쉘 주변 이펙트
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_ShellWind00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_ShellWind01");

	// 임팩트
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_WindImpact");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_DoomScrew");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Impact");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_WindImpact_Ground");

	// 파편
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_BrokenPiece00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_BrokenGlass00");

	// 임팩트
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_WindCannon00");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_WindCannon01");

	// 환경 오브젝트
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_ButterflyFish");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_SurgeonFish");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CylinderSteam");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CylinderDrop");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_MagnetTube");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_RustyMetal");

	// 크리스탈 파편
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CrystalShard0");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_CrystalShard1");

	// 스킬 : 성게 날리기(추가)
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Urchin");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_Tornado");
	m_ModelPrototypeName.push_back(L"Prototype_Component_Model_WaterSplash");


	// 트레일
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_TrailHammer");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail00");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail01");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail02");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail03");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail04");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail05");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail06");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail07");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail08");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail09");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail10");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail11");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail12");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Trail13");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_TrailGolf");

	// 잉커톤용 트레일
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Daoguang05");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Daoguang005");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_BladeDoubleSide001");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_BladeSingleSide001");
	m_TrailPrototypeName.push_back(L"Prototype_Component_Model_Blade001");

	return S_OK;
}

void CLevel_EffectTool::Adjust_Effect()
{
	if (nullptr != m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect")))
		m_pGameInstance->Delete_GameObject(LEVEL_VILLAGE, TEXT("Layer_Effect"));

	CEffectObject* pEffect = { nullptr };

	m_iShadingOption = 0;
	if (true == m_bOptionEnable[TEX_MASK])
		m_iShadingOption |= CEffectObject::MASK;
	if (true == m_bOptionEnable[TEX_NOISE])
		m_iShadingOption |= CEffectObject::NOISE;
	if (true == m_bOptionEnable[TEX_DISSOLVE])
		m_iShadingOption |= CEffectObject::DISSOLVE;
	if (true == m_bOptionEnable[TEX_EMISSIVE])
		m_iShadingOption |= CEffectObject::EMISSIVE;
	if (true == m_bOptionEnable[TEX_DISTORTION])
		m_iShadingOption |= CEffectObject::DISTORTION;

	CSpriteEffect::DESC SpriteDesc{};

	CMeshEffect::DESC MeshDesc{};

	CMeshInstEffect::DESC MeshInstDesc{};
	_uint iMeshAnimOption{};
	switch (m_iImguiSelectedType)
	{
	case CEffectObject::PARTICLE:
	case CEffectObject::SPRITE:
		SpriteDesc.eLevelID = LEVEL_VILLAGE;
		if (CEffectObject::PARTICLE == m_iImguiSelectedType)
		{
			SpriteDesc.EffectDesc.iEffectType = CEffectObject::PARTICLE;
			if (m_iTextureIndex >= m_ParticleTexPrototypeName.size())
				m_iTextureIndex = 0;
			SpriteDesc.strTextureTag = m_ParticleTexPrototypeName[m_iTextureIndex];
		}
		else
		{
			SpriteDesc.EffectDesc.iEffectType = CEffectObject::SPRITE;
			if (m_iTextureIndex >= m_SpriteTexPrototypeName.size())
				m_iTextureIndex = 0;
			SpriteDesc.strTextureTag = m_SpriteTexPrototypeName[m_iTextureIndex];
		}

		SpriteDesc.EffectDesc.bBlendEnable = m_bBlendEnable;
		SpriteDesc.EffectDesc.fActiveDelay = m_fActiveDelay;
		SpriteDesc.EffectDesc.fFinalSize = m_fFinalSize;
		SpriteDesc.EffectDesc.vMulColor = m_vMulColor;
		SpriteDesc.EffectDesc.vAddColor = m_vAddColor;
		SpriteDesc.iPattern = (_uint)m_iSelectedPattern;
		SpriteDesc.iNumAtlas = m_iNumAtlas;
		SpriteDesc.vNumAtlas = m_vNumAtlas;

		SpriteDesc.EffectDesc.iShadingOption = m_iShadingOption;
		SpriteDesc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
		SpriteDesc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
		SpriteDesc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
		SpriteDesc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
		SpriteDesc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
		SpriteDesc.EffectDesc.vDissolveStart = m_vDissolveStart;
		SpriteDesc.EffectDesc.bIsLoop = m_bIsLoop;

		SpriteDesc.BufferDesc.ParticleDesc.iNumInstance = m_iNumInstance;
		SpriteDesc.BufferDesc.ParticleDesc.isLoop = m_bIsLoop;
		SpriteDesc.BufferDesc.ParticleDesc.vRange = m_vRange;
		SpriteDesc.BufferDesc.ParticleDesc.vCenter = m_vCenter;
		SpriteDesc.BufferDesc.ParticleDesc.vSize = m_vSize;
		SpriteDesc.BufferDesc.ParticleDesc.vSpeed = m_vSpeed;
		SpriteDesc.BufferDesc.ParticleDesc.vPivot = m_vPivot;
		SpriteDesc.BufferDesc.ParticleDesc.vLifeTime = m_vLifeTime;
		SpriteDesc.BufferDesc.ParticleDesc.iPattern = (_uint)m_iSelectedPattern;


		if (FAILED(m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyEffect", LEVEL_VILLAGE, L"Layer_Effect", &SpriteDesc)))
			return;

		pEffect = static_cast<CEffectObject*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, L"Layer_Effect"));
		break;
	case CEffectObject::MESH:
	case CEffectObject::MESH_TRAIL:
		MeshDesc.eLevelID = LEVEL_VILLAGE;
		MeshDesc.vTranslation = m_vMeshTransform;
		MeshDesc.vScale = m_vMeshScale;
		if (CEffectObject::MESH == m_iImguiSelectedType)
		{
			MeshDesc.EffectDesc.iEffectType = CEffectObject::MESH;
			if (m_iSelectedMeshType >= m_ModelPrototypeName.size())
				m_iSelectedMeshType = 0;
			MeshDesc.strModelTag = m_ModelPrototypeName[m_iSelectedMeshType];
		}
		else
		{
			MeshDesc.EffectDesc.iEffectType = CEffectObject::MESH_TRAIL;
			if (m_iSelectedMeshType >= m_TrailPrototypeName.size())
				m_iSelectedMeshType = 0;
			MeshDesc.strModelTag = m_TrailPrototypeName[m_iSelectedMeshType];
		}
		if (m_iTextureIndex >= m_BaseTexPrototypeName.size())
			m_iTextureIndex = 0;
		MeshDesc.strTextureTag = m_BaseTexPrototypeName[m_iTextureIndex];

		MeshDesc.EffectDesc.GameObjectDesc.fSpeedPerSec = m_vSpeed.x;
		MeshDesc.EffectDesc.GameObjectDesc.fRotationPerSec = m_vSpeed.y;
		// Desc.MeshEffectDesc.EffectDesc.GameObjectDesc.pParentWorldMatrix

		MeshDesc.EffectDesc.bBlendEnable = m_bBlendEnable;
		MeshDesc.EffectDesc.fActiveDelay = m_fActiveDelay;
		MeshDesc.EffectDesc.fFinalSize = m_fFinalSize;
		MeshDesc.EffectDesc.vMulColor = m_vMulColor;
		MeshDesc.EffectDesc.vAddColor = m_vAddColor;

		MeshDesc.bIsLoop = m_bIsLoop;
		MeshDesc.vCenter = m_vCenter;
		MeshDesc.fLifeTime = m_vLifeTime.x;

		if (true == m_bTranslationAnim)
			iMeshAnimOption |= CEffectObject::ANIM_TRANS;
		if (true == m_bRotationAnim)
			iMeshAnimOption |= CEffectObject::ANIM_ROTATE;
		if (true == m_bScalingAnim)
			iMeshAnimOption |= CEffectObject::ANIM_SCALE;
		MeshDesc.iMeshAnimOption = iMeshAnimOption;

		MeshDesc.bUVAnim = m_bUVAnim;
		MeshDesc.vUVStart = m_vUVStart;
		MeshDesc.vUVSpeed = m_vUVSpeed;
		MeshDesc.vUVTileCount = m_vUVTileCount;
		MeshDesc.fMaxTrailLength = m_fMaxTrailLength;

		MeshDesc.EffectDesc.iShadingOption = m_iShadingOption;
		MeshDesc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
		MeshDesc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
		MeshDesc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
		MeshDesc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
		MeshDesc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
		MeshDesc.EffectDesc.vDissolveStart = m_vDissolveStart;

		if (FAILED(m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyMesh", LEVEL_VILLAGE, L"Layer_Effect", &MeshDesc)))
			return;

		pEffect = static_cast<CEffectObject*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, L"Layer_Effect"));
		break;

	case CEffectObject::MESH_INSTANCE:
		if (m_iTextureIndex >= m_BaseTexPrototypeName.size())
			m_iTextureIndex = 0;
		MeshInstDesc.strTextureTag = m_BaseTexPrototypeName[m_iTextureIndex];

		MeshInstDesc.fLifeTime = m_vLifeTime.y;
		MeshInstDesc.vOriginLocation = m_vMeshTransform;
		MeshInstDesc.bUVAnim = m_bUVAnim;
		MeshInstDesc.vUVSpeed = m_vUVSpeed;
		MeshInstDesc.vUVStart = m_vUVStart;
		MeshInstDesc.vUVTileCount = m_vUVTileCount;
		
		MeshInstDesc.BufferDesc.eLevelID = LEVEL_VILLAGE;
		if (m_iSelectedMeshType >= m_ModelPrototypeName.size())
			m_iSelectedMeshType = 0;
		MeshInstDesc.BufferDesc.strModelTag = m_ModelPrototypeName[m_iSelectedMeshType];

		MeshInstDesc.BufferDesc.iNumInstance = m_iNumInstance;
		MeshInstDesc.BufferDesc.iPattern = m_iSelectedPattern;

		MeshInstDesc.BufferDesc.bIsLoop = m_bIsLoop;
		MeshInstDesc.BufferDesc.iRotationAxis = m_iRotationAxis;
		MeshInstDesc.BufferDesc.fRotationSpeed = m_fParticleRotation;
		MeshInstDesc.BufferDesc.fRotationFinalSpeed = m_fParticleFinalRotation;

		MeshInstDesc.BufferDesc.vScalingSize = m_vMeshScale;
		MeshInstDesc.BufferDesc.fFinalSize = m_fFinalSize;

		MeshInstDesc.BufferDesc.vCenter = m_vCenter;
		MeshInstDesc.BufferDesc.vLifeTime = m_vLifeTime;
		MeshInstDesc.BufferDesc.vPivot = m_vPivot;
		MeshInstDesc.BufferDesc.vRange = m_vRange;
		MeshInstDesc.BufferDesc.vSpeed = m_vSpeed;
		
		
		MeshInstDesc.EffectDesc.iEffectType = CEffectObject::MESH_INSTANCE;

		MeshInstDesc.EffectDesc.bBlendEnable = m_bBlendEnable;
		MeshInstDesc.EffectDesc.bIsLoop = m_bIsLoop;
		MeshInstDesc.EffectDesc.fActiveDelay = m_fActiveDelay;

		MeshInstDesc.EffectDesc.vMulColor = m_vMulColor;
		MeshInstDesc.EffectDesc.vAddColor = m_vAddColor;

		MeshInstDesc.EffectDesc.iShadingOption = m_iShadingOption;
		MeshInstDesc.EffectDesc.iMaskImageNum = m_iOptionTextureIndex[TEX_MASK];
		MeshInstDesc.EffectDesc.iNoiseImageNum = m_iOptionTextureIndex[TEX_NOISE];
		MeshInstDesc.EffectDesc.iDissolveImageNum = m_iOptionTextureIndex[TEX_DISSOLVE];
		MeshInstDesc.EffectDesc.iEmissiveImageNum = m_iOptionTextureIndex[TEX_EMISSIVE];
		MeshInstDesc.EffectDesc.iDistortionImageNum = m_iOptionTextureIndex[TEX_DISTORTION];
		MeshInstDesc.EffectDesc.vDissolveStart = m_vDissolveStart;
		

		MeshInstDesc.EffectDesc.GameObjectDesc;
		MeshInstDesc.EffectDesc.fFinalSize = m_fFinalSize; // 얜 안쓰는디

		if (FAILED(m_pGameInstance->Add_GameObject_To_Layer(LEVEL_VILLAGE, L"Prototype_GameObject_DummyMeshInst", LEVEL_VILLAGE, L"Layer_Effect", &MeshInstDesc)))
			return;

		pEffect = static_cast<CEffectObject*>(m_pGameInstance->Find_GameObject(LEVEL_VILLAGE, L"Layer_Effect"));
		break;
	}
	

	if (nullptr == pEffect)
		return;
}

//void CLevel_EffectTool::Add_To_Container()
//{
//	m_pEffectContainer->Add_PartObject()
//	// Desc.EffectObjectDesc.EffectDesc.GameObjectDesc.pParentWorldMatrix
//}

HRESULT CLevel_EffectTool::Ready_Lights()
{
	if (FAILED(m_pGameInstance->SetUp_ShadowLight(XMVectorSet(-10.f, 10.f, -10.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMConvertToRadians(60.f), g_iWindowSizeX / static_cast<_float>(g_iWindowSizeY), 0.1f, 1000.f)))
		return E_FAIL;

	LIGHT_DESC LightDesc{};
	ZeroMemory(&LightDesc, sizeof(LIGHT_DESC));

	LightDesc.eType = LIGHT_TYPE::DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.0f, 1.0f, 1.0f, 1.f);
	LightDesc.vAmbient = _float4(0.5f, 0.5f, 0.5f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(L"Light_Global", LightDesc)))
		return E_FAIL;

	return S_OK;
}

CLevel_EffectTool* CLevel_EffectTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_EffectTool* pInstance = new CLevel_EffectTool{ pDevice, pContext };

	return pInstance;
}

void CLevel_EffectTool::Free()
{
	CLevel::Free();

	for (auto& pTexture : m_pParticleTextures)
		Safe_Release(pTexture);

	for (auto& pTexture : m_pSpriteTextures)
		Safe_Release(pTexture);

	for (auto& pTexture : m_pBaseTextures)
		Safe_Release(pTexture);

	for (size_t i = 0; i < TEX_END; i++)
	{
		for (auto& pTexture : m_pOptionTextures[i])
			Safe_Release(pTexture);
	}
}
