#include "Gem.h"

CGem::CGem()
{
}

HRESULT CGem::Initialize(vector<_uint>& GemSequence, _float fCoolTime)
{
	m_GemSequence = GemSequence;
	m_fCoolTime = fCoolTime;
    return S_OK;
}

void CGem::Update(_float fTimeDelta)
{
	// Ȱ��ȭ �� ���� �����Ӹ� true ��ȯ
	if (true == m_bActivate)
	{
		m_bActivate = false;
		m_fActivatedTimeAcc = m_fCoolTime;
	}

	// ��Ÿ�� ����
	if(true == m_bCooling)
	{
		m_fActivatedTimeAcc -= fTimeDelta;

		// ��Ÿ�� �� ������ �� ����
		if (m_fActivatedTimeAcc <= 0.f)
		{
			m_bCooling = false;
			m_fActivatedTimeAcc = 0.f;
		}
	}
}

_bool CGem::Compare_Sequence(_uint iIndex, _uint iInput)
{
	if (false == m_bSequencing)
		return false;

	if (m_GemSequence[iIndex] == iInput) 
		return true;

	m_bSequencing = false;
	return false;
}

CGem* CGem::Create(vector<_uint>& GemSequence, _float fCoolTime)
{
	CGem* pInstance = new CGem();

	if (FAILED(pInstance->Initialize(GemSequence, fCoolTime)))
	{
		MSG_BOX("Failed to Created : CGem");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGem::Free()
{
    __super::Free();
}
