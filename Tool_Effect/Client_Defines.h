#pragma once

namespace Client
{
	const unsigned int g_iWindowSizeX = 1280;
	const unsigned int g_iWindowSizeY = 720;

	enum LEVELID 
	{ 
		LEVEL_STATIC,
		LEVEL_LOADING,
		LEVEL_LOGO, 
		LEVEL_VILLAGE, 
		LEVEL_END 
	};

	enum class CELL_OPTION
	{
		DEFAULT,
		DUMMY,

		MAX
	};

	enum class POSTEX_PASS
	{
		DEFAULT,
		SOFTPARTICLE,
		INTERSECTIONHIGHLIGHT,
		SKY,

	};

	enum class SHADER_PASS_UI
	{
		DEFAULT,
		COLOR,
		COLOR_BLEND,
		CLOCK,
		FILTER_BLEND,
	};

	enum class SHADER_PASS_MESH
	{
		DEFAULT,
		BASE_COLOR,
		NON_NORMAL_MAPPING,
		FOWARD,
		UV_WRAPPING,
		UV_WRAPPING_EXCEPT_NORMAL,

	};

	enum class SHADER_PASS_ANIMMESH
	{
		DEFAULT,
		NON_NORMAL_MAPPING,
		FOWARD,
		SHADOW,
		BASE_COLOR,
	};

	enum class SHADER_PASS_PARTICLE
	{
		DEFAULT,
		SPRITE_UV,
		BLENDACC,
		BLENDACC_UV,
		REVEAL,
		REVEAL_UV,
	};

	enum class SHADER_PASS_EFFECTMESH
	{
		DEFAULT,
		TRAIL,
		DISTORTION,
		DISTORTION_TRAIL,
		BLENDACC,
		BLENDACC_TRAIL,
		REVEAL,
		REVEAL_TRAIL,

	};

	enum class SHADER_PASS_EFFECTMESHINST
	{
		DEFAULT,
		BLENDACC,
		REVEAL,
		DISTORTION
	};

	enum class COLLISION_GROUP
	{
		DEFAULT,
		SCAVENGER,
		MONSTER,
		SWING_HIT,
		ITEM,
		MONSTER_ATTACK,
		SHIP,
		SHIP_INTERIOR,
		MAP_INTERACTION,
		SCAN,

	};

	enum class SOUND_CHANNEL
	{
		BGM,
		AMBIENCE,
		PLAYER,
		UI,
	};
}
using namespace Client;

extern HINSTANCE g_hInst;
extern HWND g_hWnd;

#include <process.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
