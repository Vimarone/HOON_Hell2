#pragma once


#include "Client_Defines.h"
#include "Base.h"

BEGIN(Client)

class CImgui_Manager final : public CBase
{
public:
	static HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static void Update();
	static void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static void Release();
};

END