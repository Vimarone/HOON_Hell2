#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Bind_InputAssembler();
	virtual HRESULT Render();

public:
	vector<_float3>&					Get_VtxPos() { return m_VertexPos; }
	_uint								Get_VtxNum() { return m_iNumVertices; }
	_uint								Get_IdxNum() { return m_iNumIndices; }

protected:
	ID3D11Buffer*					m_pVB = { nullptr };
	ID3D11Buffer*					m_pIB = { nullptr };

	D3D11_BUFFER_DESC				m_BufferDesc = {};
	D3D11_SUBRESOURCE_DATA			m_InitialData = {};

protected:
	DXGI_FORMAT						m_eIndexFormat = {};
	_uint							m_iNumVertexBuffers = {};
	_uint							m_iNumVertices = {};
	_uint							m_iVertexStride = {};
	_uint							m_iNumIndices = {};
	_uint							m_iIndexStride = {};
	_float3*						m_pVertexPositions = { nullptr };
	D3D11_PRIMITIVE_TOPOLOGY		m_ePrimitiveTopology = {};

protected:
	vector<_float3>					m_VertexPos;

protected:
	HRESULT Create_Buffer(ID3D11Buffer** ppOut);


public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free();
};

END