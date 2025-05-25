#pragma once
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CEventObject abstract : public CGameObject
{
public:
	struct EVENT_DESC : CGameObject::GAMEOBJECT_DESC
	{
		_float	fCallDelay{}; // �̺�Ʈ ȣ�� �� �߻������� ������
		_float	fEffectiveRange{}; // �̺�Ʈ ȣ�� �߻� �������κ��� ����ü�� ������ �� �ִ� ����

		_uint	iCount{ 1 };	// �� ����ü ����
		_float	fCountDelay{}; // ����ü �� ����

		_uint	iFrequency{ 1 }; // �� ȸ�� ������ ����߸� ������
		_float	fFrequencyDelay{}; // ȸ�� ����

		// �� ����ü ���� = iCount * iFrequency
	};

protected:
	CEventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEventObject(const CEventObject& Prototype);
	virtual ~CEventObject() = default;

public:
	_float			Get_CallDelay() { return m_fCallDelay; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void	Priority_Update(_float fTimeDelta);
	virtual void	Update(_float fTimeDelta);
	virtual void	Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void			Execute_Event(_uint iEventType, _fvector vPoint);
	virtual	_bool	Event(_float fTimeDelta, _uint iEventType) = 0;		// �̺�Ʈ ���� �� true ��ȯ

private:
	void			Shutdown_Event();

protected:
	_uint			m_iEventType = {};
	_bool			m_bEventCall = { false };
	_float3			m_vDestination = {};
	_bool			m_bActivatedFirstFrame = {};

	_float			m_fCallDelay = {};
	_float			m_fCallTimeAcc = {};
	
	_bool			m_bAttackEnable = {};
	_float			m_fEffectiveRange = {};

	_uint			m_iCount = {};
	_uint			m_iCountAcc = {};
	_float			m_fCountDelay = {};
	_float			m_fCountTimeAcc = {};

	_uint			m_iFrequency = {};
	_uint			m_iFrequencyAcc = {};
	_float			m_fFrequencyDelay = {};
	_float			m_fFrequencyTimeAcc = {};


public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END