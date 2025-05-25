#include "VIBuffer_Particle_Instancing.h"
#include "GameInstance.h"

CVIBuffer_Particle_Instancing::CVIBuffer_Particle_Instancing(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Particle_Instancing::CVIBuffer_Particle_Instancing(const CVIBuffer_Particle_Instancing & Prototype)
	: CVIBuffer{ Prototype }
{	
}

HRESULT CVIBuffer_Particle_Instancing::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer_Particle_Instancing::Initialize(void * pArg)
{
	PARTICLE_INSTANCING_DESC* pDesc = static_cast<PARTICLE_INSTANCING_DESC*>(pArg);

	m_iNumInstance = pDesc->iNumInstance;
	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOINT);
	m_iInstanceVertexStride = sizeof(VTX_PARTICLE_INSTANCE);
	m_iNumIndexPerInstance = 1;
	m_iNumIndices = m_iNumIndexPerInstance * m_iNumInstance;
	m_iIndexStride = 2;
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

#pragma region INSTANCE_BUFFER
	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);
	m_InstanceBufferDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.StructureByteStride = m_iInstanceVertexStride;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;

	m_pInstanceVertices = new VTX_PARTICLE_INSTANCE[m_iNumInstance];
	ZeroMemory(m_pInstanceVertices, sizeof(VTX_PARTICLE_INSTANCE) * m_iNumInstance);

	m_pSpeeds = new _float[m_iNumInstance];
	ZeroMemory(m_pSpeeds, sizeof(_float) * m_iNumInstance);

	m_isLoop = pDesc->isLoop;

	m_vPivot = pDesc->vPivot;

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_float4			vTranslation = {};

		vTranslation.x = m_pGameInstance->Compute_Random(pDesc->vCenter.x - pDesc->vRange.x * 0.5f, pDesc->vCenter.x + pDesc->vRange.x * 0.5f);
		vTranslation.y = m_pGameInstance->Compute_Random(pDesc->vCenter.y - pDesc->vRange.y * 0.5f, pDesc->vCenter.y + pDesc->vRange.y * 0.5f);
		vTranslation.z = m_pGameInstance->Compute_Random(pDesc->vCenter.z - pDesc->vRange.z * 0.5f, pDesc->vCenter.z + pDesc->vRange.z * 0.5f);
		vTranslation.w = 1;

		m_pSpeeds[i] = m_pGameInstance->Compute_Random(pDesc->vSpeed.x, pDesc->vSpeed.y);

		m_pInstanceVertices[i].vRight = _float4(1.f, 0.f, 0.f, 0.f);
		m_pInstanceVertices[i].vUp = _float4(0.f, 1.f, 0.f, 0.f);
		m_pInstanceVertices[i].vLook = _float4(0.f, 0.f, 1.f, 0.f);
		m_pInstanceVertices[i].vTranslation = vTranslation;
		m_pInstanceVertices[i].vLifeTime.x = m_pGameInstance->Compute_Random(pDesc->vLifeTime.x, pDesc->vLifeTime.y);

		_float			fScale = m_pGameInstance->Compute_Random(pDesc->vSize.x, pDesc->vSize.y);

		XMStoreFloat4(&m_pInstanceVertices[i].vRight, XMLoadFloat4(&m_pInstanceVertices[i].vRight) * fScale);
		XMStoreFloat4(&m_pInstanceVertices[i].vUp, XMLoadFloat4(&m_pInstanceVertices[i].vUp) * fScale);
		XMStoreFloat4(&m_pInstanceVertices[i].vLook, XMLoadFloat4(&m_pInstanceVertices[i].vLook) * fScale);
	}

	m_InstanceInitialData.pSysMem = m_pInstanceVertices;

#pragma endregion

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.StructureByteStride = m_iVertexStride;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	VTXPOINT* pVertices = new VTXPOINT[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOINT) * m_iNumVertices);

	pVertices[0].vPosition = _float3(0.f, 0.0f, 0.f);
	pVertices[0].fPSize = 1.f;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion


#pragma region INDEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.StructureByteStride = m_iIndexStride;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion

	return m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceInitialData, &m_pVBInstance);	
}

HRESULT CVIBuffer_Particle_Instancing::Bind_InputAssembler()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	ID3D11Buffer*		pVertexBuffer[] = { m_pVB, m_pVBInstance, };
	_uint				iVertexStrides[] = { m_iVertexStride, m_iInstanceVertexStride, };
	_uint				iOffsets[] = { 0, 0 };

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffer, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}

HRESULT CVIBuffer_Particle_Instancing::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

_bool CVIBuffer_Particle_Instancing::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE*	pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;
		pVertices[i].vLifeTime.y += fTimeDelta;
		if (pVertices[i].vLifeTime.x <= pVertices[i].vLifeTime.y)
		{
			if (true == m_isLoop && false == m_isLoopEnd)
			{
				pVertices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
				pVertices[i].vLifeTime.y = 0.f;
			}
			else
				iCount++;
		}
		else
		{
			if (true == m_isLoopEnd)
				iCount++;
		}
	}
	m_pContext->Unmap(m_pVBInstance, 0);
	if (iCount == m_iNumInstance)
		return true;
	return false;
}

_bool CVIBuffer_Particle_Instancing::Rise(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE*	pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vTranslation.y += m_pSpeeds[i] * fTimeDelta;
		pVertices[i].vLifeTime.y += fTimeDelta;
		if (pVertices[i].vLifeTime.x <= pVertices[i].vLifeTime.y)
		{
			if (true == m_isLoop && false == m_isLoopEnd)
			{
				pVertices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
				pVertices[i].vLifeTime.y = 0.f;
			}
			else
				iCount++;
		}
		else
		{
			if (true == m_isLoopEnd)
				iCount++;
		}
	}
	m_pContext->Unmap(m_pVBInstance, 0);
	if (iCount == m_iNumInstance)
		return true;
	return false;
}

_bool CVIBuffer_Particle_Instancing::Explosion(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE*	pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector		vMoveDir = XMVectorSetW(XMVector3Normalize(XMLoadFloat4(&pVertices[i].vTranslation) - XMLoadFloat3(&m_vPivot)), 0.f);

		XMStoreFloat4(&pVertices[i].vTranslation, XMLoadFloat4(&pVertices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);

		pVertices[i].vLifeTime.y += fTimeDelta;
		if (pVertices[i].vLifeTime.x <= pVertices[i].vLifeTime.y)
		{
			if (true == m_isLoop && false == m_isLoopEnd)
			{
				pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
				pVertices[i].vLifeTime.y = 0.f;
			}
			else
				iCount++;
		}
	}
	m_pContext->Unmap(m_pVBInstance, 0);
	if (iCount == m_iNumInstance)
		return true;
	return false;
}

_bool CVIBuffer_Particle_Instancing::Fraction(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE* pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector		vMoveDir = XMVectorSetW(XMVector3Normalize(XMLoadFloat4(&pVertices[i].vTranslation) - XMLoadFloat3(&m_vPivot)), 0.f);

		XMStoreFloat4(&pVertices[i].vTranslation, XMLoadFloat4(&pVertices[i].vTranslation)
					+ vMoveDir * m_pSpeeds[i] * fTimeDelta
					- XMLoadFloat4(&m_pInstanceVertices[i].vUp) * GRAVITY * 0.1f * pVertices[i].vLifeTime.y);

		pVertices[i].vLifeTime.y += fTimeDelta;
		if (pVertices[i].vLifeTime.x <= pVertices[i].vLifeTime.y)
		{
			if (true == m_isLoop && false == m_isLoopEnd)
			{
				pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
				pVertices[i].vLifeTime.y = 0.f;
			}
			else
				iCount++;
		}
		else
		{
			if (true == m_isLoopEnd)
				iCount++;
		}
	}
	m_pContext->Unmap(m_pVBInstance, 0);
	if (iCount == m_iNumInstance)
		return true;
	return false;
}

_bool CVIBuffer_Particle_Instancing::Hold(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE* pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vLifeTime.y += fTimeDelta;
		if (pVertices[i].vLifeTime.x <= pVertices[i].vLifeTime.y)
		{
			if (true == m_isLoop && false == m_isLoopEnd)
				pVertices[i].vLifeTime.y = 0.f;
			else
				iCount++;
		}
		else
		{
			if (true == m_isLoopEnd)
				iCount++;
		}		
	}
	m_pContext->Unmap(m_pVBInstance, 0);
	if (iCount == m_iNumInstance)
		return true;
	return false;
}

void CVIBuffer_Particle_Instancing::Reset_Life()
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTX_PARTICLE_INSTANCE* pVertices = static_cast<VTX_PARTICLE_INSTANCE*>(SubResource.pData);
	_uint iCount{};
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vLifeTime.y = 0;
		pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
	}
	if (true == m_isLoop)
		m_isLoopEnd = false;
	
	m_pContext->Unmap(m_pVBInstance, 0);
}

#ifdef _DEBUG
void CVIBuffer_Particle_Instancing::Set_Desc(void* pArg)
{
	Safe_Release(m_pVB);
	Safe_Release(m_pIB);
	Safe_Delete_Array(m_pSpeeds);
	Safe_Delete_Array(m_pInstanceVertices);
	Safe_Release(m_pVBInstance);

	Initialize(pArg);
}
#endif

CVIBuffer_Particle_Instancing* CVIBuffer_Particle_Instancing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Particle_Instancing* pInstance = new CVIBuffer_Particle_Instancing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CVIBuffer_Particle_Instancing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Particle_Instancing::Clone(void* pArg)
{
	CVIBuffer_Particle_Instancing* pInstance = new CVIBuffer_Particle_Instancing(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Created : CVIBuffer_Particle_Instancing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Particle_Instancing::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pSpeeds);
	Safe_Delete_Array(m_pInstanceVertices);

	Safe_Release(m_pVBInstance);
}
