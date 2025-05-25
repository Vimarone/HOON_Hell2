#include "Gem_Manager.h"
#include "Gem.h"
#include "EventObject.h"

CGem_Manager::CGem_Manager()
{
}

HRESULT CGem_Manager::Initialize()
{
	return S_OK;
}

void CGem_Manager::Update(_float fTimeDelta)
{
	for (auto& Gem : m_Gems)
		Gem.second->Update(fTimeDelta);
}

_int CGem_Manager::Get_KeyByIndex(_uint iIndex)
{
	_uint iCurrIndex{};
	for (auto& Gem : m_Gems)
	{
		if (iCurrIndex++ == iIndex)
			return Gem.first;
	}
	return -1;
}

vector<_uint>& CGem_Manager::Get_SeqByKey(_uint iKey)
{
	return m_Gems[iKey]->Get_Seq();
}

HRESULT CGem_Manager::Add_Gem(_uint iGemKey, vector<_uint>& GemSequence, _float fCoolTime, class CEventObject* pEventObject)
{
	// 문자열 비교하여 없으면 추가, 있으면 E_FAIL 반환
	if (nullptr != Find_Gem(iGemKey))
		return E_FAIL;

	CGem* pGem = CGem::Create(GemSequence, fCoolTime);
	if (nullptr == pGem)
		return E_FAIL;

	m_Gems.emplace(iGemKey, pGem);
	m_EventObjects.emplace(iGemKey, pEventObject);
	pEventObject->AddRef();

	return S_OK;
}

HRESULT CGem_Manager::Get_CoolInfo(_uint iGemKey, _float& fOutCoolTime, _float& fOutCoolRate)
{
	CGem* pGem = Find_Gem(iGemKey);
	if (nullptr == pGem)
		return E_FAIL;

	fOutCoolTime = pGem->Get_ActivatedTimeAcc();
	fOutCoolRate = pGem->Get_CoolRate();

	return S_OK;
}

_bool CGem_Manager::Get_SequeningByKey(_uint iKey)
{
	return  m_Gems[iKey]->Get_Sequencing();
}

void CGem_Manager::Check_Start()
{
	if (false == m_bCheck)
	{
		m_bCheck = true;
		m_iCurrentIndex = 0;

		// 쿨타임 진행 중이 아닌 모든 젬 입력 대기
		for (auto& Gem : m_Gems)
		{
			CGem* pGem = Gem.second;
			if(false == pGem->Get_Cooling())
				Gem.second->Start_Sequencing();
		}
	}
}

void CGem_Manager::Check_End()
{
	// 피치 못할 사정으로 체크가 종료된 경우
	if (true == m_bCheck)
	{
		m_bCheck = false;
		m_iCurrentIndex = 0;

		for (auto& Gem : m_Gems)
			Gem.second->End_Sequencing();
	}
}

_int CGem_Manager::Check_Gems(_uint iKey)
{
	if (true == m_bCheck)
	{
		_int	iGemIndex = {};
		for (auto& Gem : m_Gems)
		{
			CGem* pGem = Gem.second;
			// 입력 대기 중인 젬일 경우
			if (true == pGem->Get_Sequencing())
			{
				// 현재 입력 체크 후 미일치 시 해당 젬 시퀀싱 해제
				if (false == pGem->Compare_Sequence(m_iCurrentIndex, iKey))
					pGem->End_Sequencing();
				// 현재 입력과 시퀀스가 일치하면
				else
				{
					// 현재 인덱스와 해당 젬 시퀀스 크기 비교
					if (m_iCurrentIndex == (pGem->Get_SequenceEndIndex()))
					{
						// 같으면(시퀀스가 모두 진행 됐으면) 젬 활성화 후 순회 해제
						pGem->Activate();
						Check_End();
						return iGemIndex;
					}
				}
			}
			iGemIndex++;
		}
		m_iCurrentIndex++;
	}
	return -1;
}

CGem* CGem_Manager::Find_Gem(_uint iGemKey)
{
	auto	iter = m_Gems.find(iGemKey);
	if (iter == m_Gems.end())
		return nullptr;

	return iter->second;
}

CEventObject* CGem_Manager::Find_EventObject(_uint iGemKey)
{
	auto	iter = m_EventObjects.find(iGemKey);
	if (iter == m_EventObjects.end())
		return nullptr;

	return iter->second;
}

CGem_Manager* CGem_Manager::Create()
{
	CGem_Manager* pInstance = new CGem_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CGem_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGem_Manager::Free()
{
	__super::Free();
	for (auto& Gem : m_Gems)
		Safe_Release(Gem.second);

	m_Gems.clear();

	for (auto& EventObject : m_EventObjects)
		Safe_Release(EventObject.second);

	m_EventObjects.clear();
}
