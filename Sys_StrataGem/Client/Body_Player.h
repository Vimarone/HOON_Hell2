#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CBody_Player final : public CPartObject
{	
public:
	enum PLAYER_ANIM
	{

	};

public:
	typedef struct body_player_desc : public CPartObject::PARTOBJECT_DESC
	{
		const _uint*		pParentDir = { nullptr };
		_uint*				pParentState = { nullptr };
		_float*				pParentIdleTime = { nullptr };
	}BODY_PLAYER_DESC;

private:
	CBody_Player(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBody_Player(const CBody_Player& Prototype);
	virtual ~CBody_Player() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void	Set_RifleAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop);
	void	Set_LauncherAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop);
	void	Set_ThrowAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop);
	void	Set_KnockbackAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop, _int& iUpperAnimIndex, _bool& bIsUpperLoop);
	void	Set_StrataAnimation(_uint& iAnimIndex, _bool& bIsLoop, _int& iArmAnimIndex, _bool& bIsArmLoop);
	void	Set_MotionEnd();

private:
	CShader*					m_pShaderCom = { nullptr };	
	CModel*						m_pModelCom = { nullptr };

private:
	class CCamera_Follow*		m_pPlayerCamera = { nullptr };

private:
	_uint*						m_pParentState = { nullptr };
	const _uint*				m_pParentDir = { nullptr };
	_float*						m_pParentIdleTime = { nullptr };

	_bool						m_bIsArmStop = {};
	_bool						m_bDeadAnimEnd = {};

public:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CBody_Player* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END