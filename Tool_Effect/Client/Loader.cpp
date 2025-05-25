#include "pch.h"
#include "Loader.h"

#include "GameInstance.h"

/* for.Level Static*/
#include "Camera_Free.h"

/* for.Level Logo*/
#include "BackGround.h"

/* for.Level GamePlay*/
#include "Tutorial_Terrain.h"
#include "Sky.h"
#include "Dummy_Effect.h"
#include "Dummy_Mesh.h"
#include "Dummy_MeshInst.h"


CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

//_uint APIENTRY Thread_Main(void* pArg)
_uint WINAPI Thread_Main(void* pArg)
{
	CLoader* pLoader = static_cast<CLoader*>(pArg);

	if (FAILED(pLoader->Start_Loading()))
	{
		MSG_BOX("Failed to Loading");
	}

	return 0;
}

HRESULT CLoader::Initialize(LEVELID eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_CriticalSection);
	// m_eNextLevelID가 필요로하는 자원을 로드해주기위한 스레드를 만들어낸다. 
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, Thread_Main, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Start_Loading()
{
	HRESULT hr{};

	// chat gpt 가 CoInitializeEx 코드는 임계영역 전에 작성하는게 좋다고 했음
	hr = CoInitializeEx(nullptr, 0);
	if (FAILED(hr)) // 내가 작성함
		return hr;

	EnterCriticalSection(&m_CriticalSection);

	switch (m_eNextLevelID)
	{
	case LEVELID::LEVEL_VILLAGE:
		hr = Loading_For_Level_GamePlay();
		break;
	}

	LeaveCriticalSection(&m_CriticalSection);

	if (FAILED(hr))
		return E_FAIL;

	return S_OK;
}

_float CLoader::Get_CurrnetProgress() const
{
	_float fRatio = m_iCurrentLine / static_cast<_float>(m_iNumTotalLine);

	fRatio = min(1.f, fRatio);
	fRatio = max(0.f, fRatio);

	return fRatio;
}

#ifdef _DEBUG
void CLoader::SetUp_WindowText()
{
	SetWindowText(g_hWnd, m_szLoadingText);
}
#endif

HRESULT CLoader::Loading_For_Level_GamePlay()
{
#pragma region DEFAULT TEXTURE
	/* for Static: Texture =================================================================== */
	lstrcpy(m_szLoadingText, TEXT("Load Resources for Static: Texture"));
	/* For.Prototype_Component_Texture_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, L"Prototype_Component_Texture_Sky",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/SkyBox/Sky_%d.dds", 6))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pContext, _T("../Bin/Resources/Textures/Terrain/SandBaseColor%d.png"), 3))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Normal */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_Component_Texture_Normal"),
		CTexture::Create(m_pDevice, m_pContext, _T("../Bin/Resources/Textures/Terrain/SandNoiseNormalMap.png")))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Normal_Sand */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_Component_Texture_Normal_Sand"),
		CTexture::Create(m_pDevice, m_pContext, _T("../Bin/Resources/Textures/Terrain/Sand_normal.png")))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Normal_Wavy */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_Component_Texture_Normal_Wavy"),
		CTexture::Create(m_pDevice, m_pContext, _T("../Bin/Resources/Textures/Terrain/SandWavey.png")))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Terrain_Mask */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_Terrain_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Field00_Mask2.png")))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_MS_Sand */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Texture_MS_Sand"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/SandMetalicSmoothness.png")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_EffMask */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_EffMask",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Mask/Mask%02d.png", 31))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_EffNoise */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_EffNoise",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Noise/Noise%02d.png", 35))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_EffEmissive */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_EffEmissive",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Emissive/Emissive%03d.png", 133))))
		return E_FAIL;
#pragma endregion

#pragma region PARTICLE TEX
	/* For.Prototype_Component_Texture_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Snow",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Snow/Snow.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_DefaultParticle */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_DefaultParticle",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/Default_Particle.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line002 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line002",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/E_line_002c.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line004 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line004",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/E_line_004_clamp.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_BubbleOne */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_BubbleOne",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/FXT_bubblesMany_001.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/FXT_line.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Plastic */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Plastic",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/Plastic.png", 1))))
		return E_FAIL;	
	
	/* For.Prototype_Component_Texture_PlasticDots */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_PlasticDots",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/FXT_dots.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line002d */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line002d",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/E_line_002d.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash00 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash00",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash00.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash04.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SparkFlash05 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SparkFlash05",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/SparkFlash05.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_HitFlash00 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_HitFlash00",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/HitFlash00.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_HitFlash01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_HitFlash01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/HitFlash01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_HitFlash02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_HitFlash02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/HitFlash02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_HitFlash03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_HitFlash03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/HitFlash03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GodRay */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GodRay",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/GodRay.dds", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_TearDrop */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_TearDrop",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/Teardrop.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_MusicOn */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_MusicOn",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/MusicOn.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_MusicQuater */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_MusicQuater",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/MusicQuater.png", 1))))
		return E_FAIL;

#pragma endregion

#pragma region SPRITE TEX
	/* For.Prototype_Component_Texture_Lightning */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Lightning",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Lightning.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_RadialGlow */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_RadialGlow",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Radial_Glow.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_DustSmoke */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_DustSmoke",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/DustSmoke.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_EnergySpark1 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_EnergySpark1",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/EnergySpark1.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_EnergySpark03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_EnergySpark03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/EnergySpark03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Spark */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Spark",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Spark.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bokeh02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Bokeh02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Bokeh_02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Flash01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Flash01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Flash_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Flash02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Flash02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Flash_02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Impact04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Impact04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Impact_04.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Lightning04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Lightning04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Lightning_04.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Poof01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Poof01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Poof_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_RadialGlow01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_RadialGlow01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_RadialGlow03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_RadialGlow03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_RadialGlow04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_RadialGlow04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_RadialGlow_04.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Shatter10 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Shatter10",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Shatter_10.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Snap01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Snap01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Snap_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sparks01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Sparks01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Sparks_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sparks02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Sparks02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Sparks_02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Splat01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Splat01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Splat_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Star01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Star01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Tex_Star_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_HitSpark */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_HitSpark",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/HitSpark.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Smoke114 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Smoke114",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/E_smoke_114.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Smoke002 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Smoke002",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/E_smoke_002_a.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Smoke103 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Smoke103",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/E_smoke_103.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Glow005 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Glow005",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/Fx_glow_005.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark00 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark00",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark00.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark04.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark05 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark05",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark05.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark06 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark06",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark06.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark07 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark07",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark07.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark08 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark08",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark08.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark09 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark09",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark09.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark10 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark10",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark10.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark11 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark11",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark11.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark12 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark12",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark12.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_GlowSpark13 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_GlowSpark13",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/GlowSpark13.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FXT_trail01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_FXT_trail01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/FXT_trail01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_WaterSplash025 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_WaterSplash025",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/E_water_025.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FXT_BubblesMany001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_FXT_BubblesMany001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Sprite/FXT_bubblesMany_001.png", 1))))
		return E_FAIL;
#pragma endregion

#pragma region MESHBASE 
	/* For.Prototype_Component_Texture_Vein032 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Vein032",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/E_vein_032.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Vein093 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Vein093",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/E_vein_093.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Vein138 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Vein138",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/E_vein_138.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail0158 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail0158",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail_0158.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Evelyn002 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Evelyn002",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_trail_evelyn_002.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Noise017gt */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Noise017gt",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/noise_017_gt.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Trail_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail02 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail02",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Trail_02.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail03 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail03",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Trail_03.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail10 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail10",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Trail_10.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Wind18 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Wind18",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Wind_18_add.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail06 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail06",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail06.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Water */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Water",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_water.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Water001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Water001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_water_001.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Wind */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Wind",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_wind.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Wind26 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Wind26",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Wind_26_alpha.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Wind29 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Wind29",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Wind_29_alpha.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Pink */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Pink",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Pink.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail31 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail31",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Trail31.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line3 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line3",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Fx_line_3.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line04 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line04",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Fx_line_04_alpha.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line07 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line07",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Fx_line_07.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Boss10 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Boss10",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_BigBoss_sp_10.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bite */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Bite",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_bite.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_LineSpot */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_LineSpot",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line_Spot.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line_001_alpha.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line008 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line008",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line_004.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line012 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line012",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line_012_c.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Balde001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Balde001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Line_balde_001.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_LineC */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_LineC",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line_c.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Line003 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Line003",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_line003.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SmallStarTrail2 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SmallStarTrail2",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_SmallStarTrail2.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Smoke001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Smoke001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Smoke_001.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SmokeSlash */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SmokeSlash",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_smokeSlash.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sueyoi004 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Sueyoi004",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_sueyoi_skill_004.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Trail008 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Trail008",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/FXT_Trail_008.png", 1))))
		return E_FAIL;	
	
	/* For.Prototype_Component_Texture_ScaryFace */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_ScaryFace",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/ScaryFace.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_DNA */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_DNA",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/DNA.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_MeshGlow137 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_MeshGlow137",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/MeshGlow_137.png", 1))))
		return E_FAIL;	
	
	/* For.Prototype_Component_Texture_MeshGlow001 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_MeshGlow001",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/E_glow_001.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Blade16 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Blade16",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Blade_16_alpha.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_BloodCloud */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_BloodCloud",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/Particle/Blood_cloud.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Eff_MetalScrap */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_Eff_MetalScrap",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/MetalScrap_basecolor.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_MeshNoise01 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_MeshNoise01",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/Tex_Noise_01.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_ButterflyFish */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_ButterflyFish",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/ButterflyFish.png", 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_SurgeonFish */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_SurgeonFish",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/MeshBase/SurgeonFish.png", 1))))
		return E_FAIL;

#pragma endregion

#pragma region COLOR TEX

	/* For.Prototype_Component_Texture_ColorSampler */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_Texture_ColorSampler",
		CTexture::Create(m_pDevice, m_pContext, L"../Bin/Resources/Textures/Effects/ColorSampler.png", 1))))
		return E_FAIL;
	
#pragma endregion

#pragma region NAVIGATION
	/* for Static: Navigation =================================================================== */
	lstrcpy(m_szLoadingText, TEXT("Load Resources for Static: Navigation"));
	/* For.Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, TEXT("Prototype_Component_Navigation"),
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../Bin/DataFiles/Navigation.dat")))))
		return E_FAIL;
#pragma endregion

#pragma region  SHADER
	/* for Static: Shader =================================================================== */
	lstrcpy(m_szLoadingText, TEXT("Load Resources for Static: Shader"));
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh_Effect */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxMesh_Effect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Effect.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh_Particle */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxMesh_Particle"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Particle.hlsl"), VTXMESH_PARTICLE::Elements, VTXMESH_PARTICLE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxAnimMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxCube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPosTex_Particle */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxPosTex_Particle"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex_Particle.hlsl"), 
			VTXPOSTEX_PARTICLE::Elements, VTXPOSTEX_PARTICLE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPoint_Particle */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxPoint_Particle"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPoint_Particle.hlsl"), 
			VTXPOINT_PARTICLE::Elements, VTXPOINT_PARTICLE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPoint_GodRay */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Shader_VtxPoint_GodRay"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPoint_GodRay.hlsl"),
			VTXPOINT_PARTICLE::Elements, VTXPOINT_PARTICLE::iNumElements))))
		return E_FAIL;

#pragma endregion

#pragma region VIBUFFER
	/* for Static: VIBuffer =================================================================== */
	lstrcpy(m_szLoadingText, TEXT("Load Resources for Static: VIBuffer"));
	/* For.Prototype_Component_VIBuffer_Cube */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_VIBuffer_Cube"),
		CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_Sphere */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_VIBuffer_Sphere"),
		CVIBuffer_Sphere::Create(m_pDevice, m_pContext))))
		return E_FAIL;
#pragma endregion

#pragma region MODEL
	lstrcpy(m_szLoadingText, TEXT("모델 원형을 생성한다."));
#pragma region TUTORIAL_VIBUFFER

	lstrcpyW(m_szLoadingText, TEXT("Load Resources for Tutorial : VIBuffer"));
	/* For.Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_VIBuffer_Terrain_Tutorial"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/MakeTutorialMap.bmp"), false))))
		return E_FAIL;

#pragma endregion

	_matrix PreTransformMatrix = XMMatrixIdentity();
	{
		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Blade"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Blade00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DNA"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/DNA.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_PlaneCircle"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Plane_Circle.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_PlaneRect"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Plane_Rect.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;
		
		PreTransformMatrix = XMMatrixScaling(0.075f, 0.075f, 0.075f) * XMMatrixRotationX(XMConvertToRadians(90.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Ring00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Ring00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Ring01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Ring01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Roll00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Roll00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Sphere00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Sphere00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Sphere01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Sphere01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Crystal

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CrystalPart00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Crystal/Crystal_Part0.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CrystalPart01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Crystal/Crystal_Part1.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CrystalPart02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Crystal/Crystal_Part2.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Dome

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DomePlane00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome_Plane00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DomePlane01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome_Plane01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DomePlane02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome_Plane02.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DomePlane03"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome_Plane03.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationX(XMConvertToRadians(90.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome02.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome03"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome03.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome04"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome04.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Dome05"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome05.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// MoonShell

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_EffMoonShell"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/MoonShell/MoonShell.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Barnacle"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/MoonShell/Barnacle.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_MoonBrick"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/MoonShell/MoonBrick.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Screw

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Screw00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Screw/Screw00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Screw01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Screw/Screw01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Screw02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Screw/Screw02.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Screw03"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Screw/Screw03.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Screw04"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Screw/Screw04.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Skill

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Hand"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Skill/Hand.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_ScaryFace"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Skill/ScaryFace.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_TopodaPunch"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Skill/Topoda_Punch.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Tentacle

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Tentacle00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Tentacle/Tentacle00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Tentacle01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Tentacle/Tentacle01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Tentacle02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Tentacle/Tentacle02.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// MoonShell Effect

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_ShellWind00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/MoonShell/ShellWind00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_ShellWind01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/MoonShell/ShellWind01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Wind Impact

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_WindImpact"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/WindImpact.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Doom Screw

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_DoomScrew"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Dome/Dome_Screw.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Impacts

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Impact"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Impact.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_WindImpact_Ground"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/WindImpact_Ground.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Magnet

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_MagnetTube"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Magnet_Tube.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_RustyMetal"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Rusty_Metal.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Crystal Shards

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CrystalShard0"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Crystal/CrystalShard.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CrystalShard1"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Crystal/CrystalShard1.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/*------------------------------*/// Trail
		PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(XMConvertToRadians(90.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_TrailHammer"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail_Hammer.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(270.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail01.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail02"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail02.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail03"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail03.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail04"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail04.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail05"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail05.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail06"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail06.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail07"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail07.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail08"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail08.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail09"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail09.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail10"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail10.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail11"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail11.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail12"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail12.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Trail13"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail13.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationX(XMConvertToRadians(90.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_TrailGolf"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Trail_Golf.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;


		PreTransformMatrix = XMMatrixScaling(0.001f, 0.001f, 0.001f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_BrokenGlass00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Broken/Broken_Glass_00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_BrokenPiece00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Broken/Broken_Piece_00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_WindCannon00"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/FXMesh_wind_cannon02_001.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_WindCannon01"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/FXMesh_wind_cannon02_003.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Daoguang05"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Inkerton/daoguang_05.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Daoguang005"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Inkerton/Fx_mesh_daoguang005.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_BladeDoubleSide001"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Inkerton/FXMesh_blade_doubleside_001.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_BladeSingleSide001"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Inkerton/FXMesh_blade_singleside_001.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationZ(XMConvertToRadians(180.f));
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Blade001"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Trail/Inkerton/FXT_mesh_blade_001.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		/* 환경 오브젝트 */
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_ButterflyFish"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/EnvironmentObject/ButterflyFish.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_SurgeonFish"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/EnvironmentObject/SurgeonFish.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;		

		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CylinderSteam"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Cylinder_Steam.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;


		/* 스킬 */
		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_CylinderDrop"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Cylinder_Drop.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Urchin"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Skill/Urchin.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_Tornado"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/Tornado00.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_Component_Model_WaterSplash"),
			CModel::Create(m_pDevice, m_pContext, "../Bin/Resources/Models/Effects/WaterSplash.fbx", CModel::TYPE::PRE_TRANSFORM, PreTransformMatrix))))
			return E_FAIL;
	}

	/* For.Prototype_Component_VIBuffer_Particle_Point */
	{
		/* For.Particle_Palyer */
		CVIBuffer_InstancingParticle::DESC Desc {};

		Desc.iNumInstance = 300;
		Desc.vCenter = _float3(0.f, 2.f, 0.f);
		Desc.vRange = _float3(0.3f, 0.3f, 0.3f);
		Desc.vSize = _float2(0.02f, 0.05f);
		Desc.vSpeed = _float2(5.f, 8.f);
		Desc.vLifeTime = _float2(0.2f, 0.3f);
		Desc.isLoop = true;

	
		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_VIBuffer_Particle_Point",
			CVIBuffer_Particle_Point::Create(m_pDevice, m_pContext, Desc))))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_VILLAGE, L"Prototype_Component_VIBuffer_MeshInstancing_SRT",
		CVIBuffer_Instancing_Mesh_SRT::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma endregion

#pragma region GAME_OBJECT
	lstrcpy(m_szLoadingText, _T("객체 원형을 생성"));

	/*For. Prototype_GameObject_Sky*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_GameObject_Sky"),
		CSky::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/*For. Prototype_GameObject_Camera_Free*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/*For. Prototype_GameObject_MyTerrain*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_GameObject_Tutorial_Terrain"), CTutorial_Terrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/*For. Prototype_GameObject_DummyEffect*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_GameObject_DummyEffect"), CDummy_Effect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/*For. Prototype_GameObject_DummyMesh*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_GameObject_DummyMesh"), CDummy_Mesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;	
	
	/*For. Prototype_GameObject_DummyMeshInst*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVELID::LEVEL_VILLAGE, _T("Prototype_GameObject_DummyMeshInst"), CDummy_MeshInst::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma endregion

	/* 로딩이 완료되었습ㄴ미다 */
	lstrcpyW(m_szLoadingText, TEXT("로딩끝."));
	m_bIsFinished = true;

	return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVELID eNextLevelID)
{
	CLoader* pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Create : CLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	Engine::CBase::Free();

	WaitForSingleObject(m_hThread, INFINITE);
	//DeleteObject(m_hThread); // 이거 결국 사용 안하는듯, 리턴값도 false 임
	CloseHandle(m_hThread);

	DeleteCriticalSection(&m_CriticalSection);
	
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
