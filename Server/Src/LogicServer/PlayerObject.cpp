﻿#include "stdafx.h"
#include "PlayerObject.h"
#include "PacketHeader.h"
#include "PlayerManager.h"
#include "RoleModule.h"
#include "GameSvrMgr.h"
#include "CopyModule.h"
#include "BagModule.h"
#include "EquipModule.h"
#include "PetModule.h"
#include "PartnerModule.h"
#include "TaskModule.h"
#include "ActivityModule.h"
#include "MountModule.h"
#include "CounterModule.h"
#include "StoreModule.h"
#include "GemModule.h"
#include "../ServerData/ServerDefine.h"
#include "../ServerData/RoleData.h"
#include "../ConfigData/ConfigData.h"
#include "../ServerData/CopyData.h"
#include "../GameServer/GameService.h"
#include "../Message/Msg_ID.pb.h"
#include "../Message/Msg_RetCode.pb.h"

CPlayerObject::CPlayerObject()
{

}

CPlayerObject::~CPlayerObject()
{

}

BOOL CPlayerObject::Init(UINT64 u64ID)
{
	m_u64ID             = u64ID;
	m_dwProxyConnID     = 0;
	m_dwClientConnID    = 0;
	m_dwCopyGuid        = 0;      //当前的副本ID
	m_dwCopyID			= 0;        //当前的副本类型
	m_dwCopySvrID       = 0;        //副本服务器的ID
	m_dwToCopyGuid      = 0;        //正在前往的副本ID
	m_dwToCopyID        = 0;         //正在前往的副本ID
	m_dwToCopySvrID     = 0;
	m_IsOnline			= FALSE;

	ERROR_RETURN_FALSE(CreateAllModule());

	return TRUE;
}

BOOL CPlayerObject::Uninit()
{
	DestroyAllModule();
	m_u64ID             = 0;		//角色ID
	m_dwProxyConnID     = 0;		//网关服的连接ID
	m_dwClientConnID    = 0;		//客户端的连接ID
	m_dwCopyGuid        = 0;        //当前的副本ID
	m_dwCopyID          = 0;        //当前的副本类型
	m_dwCopySvrID       = 0;        //副本服务器的ID
	m_dwToCopyGuid      = 0;        //正在前往的副本ID
	m_dwToCopyID        = 0;        //正在前往的副本类型
	m_IsOnline			= FALSE;
	return TRUE;
}


BOOL CPlayerObject::OnCreate(UINT64 u64RoleID)
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->OnCreate(u64RoleID);
	}
	return TRUE;
}

BOOL CPlayerObject::OnDestroy()
{
	DestroyAllModule();

	return TRUE;
}

BOOL CPlayerObject::OnLogin()
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->OnLogin();
	}

	m_IsOnline = TRUE;

	return TRUE;
}

BOOL CPlayerObject::OnLogout()
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->OnLogout();
	}

	m_IsOnline = FALSE;

	return TRUE;
}

BOOL CPlayerObject::OnNewDay()
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->OnNewDay();
	}
	return TRUE;
}

BOOL CPlayerObject::ReadFromDBLoginData(DBRoleLoginAck& Ack)
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->ReadFromDBLoginData(Ack);
	}

	return TRUE;
}

BOOL CPlayerObject::DispatchPacket(NetPacket* pNetPacket)
{
	for (int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);

		if (pBase->DispatchPacket(pNetPacket))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CPlayerObject::CreateAllModule()
{
	m_MoudleList.assign(MT_END, NULL);
	m_MoudleList[MT_ROLE]			= new CRoleModule(this);
	m_MoudleList[MT_COPY]			= new CCopyModule(this);
	m_MoudleList[MT_BAG]			= new CBagModule(this);
	m_MoudleList[MT_EQUIP]			= new CEquipModule(this);
	m_MoudleList[MT_GEM]			= new CGemModule(this);
	m_MoudleList[MT_PET]			= new CPetModule(this);
	m_MoudleList[MT_PARTNER]		= new CPartnerModule(this);
	m_MoudleList[MT_TASK]			= new CTaskModule(this);
	m_MoudleList[MT_MOUNT]			= new CMountModule(this);
	m_MoudleList[MT_ACTIVITY]		= new CActivityModule(this);
	m_MoudleList[MT_COUNTER]		= new CCounterModule(this);
	m_MoudleList[MT_STORE]			= new CStoreModule(this);

	return TRUE;
}

BOOL CPlayerObject::DestroyAllModule()
{
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);

		pBase->OnDestroy();
		delete pBase;
	}

	m_MoudleList.clear();
	return TRUE;
}

BOOL CPlayerObject::SendMsgProtoBuf(UINT32 dwMsgID, const google::protobuf::Message& pdata)
{
	ERROR_RETURN_FALSE(m_dwProxyConnID != 0);

	return ServiceBase::GetInstancePtr()->SendMsgProtoBuf(m_dwProxyConnID, dwMsgID, GetObjectID(), m_dwClientConnID, pdata);
}

BOOL CPlayerObject::SendMsgRawData(UINT32 dwMsgID, const char* pdata, UINT32 dwLen)
{
	ERROR_RETURN_FALSE(m_dwProxyConnID != 0);

	return ServiceBase::GetInstancePtr()->SendMsgRawData(m_dwProxyConnID, dwMsgID, GetObjectID(), m_dwClientConnID, pdata, dwLen);
}

BOOL CPlayerObject::SendMsgToScene(UINT32 dwMsgID, const google::protobuf::Message& pdata)
{
	return ServiceBase::GetInstancePtr()->SendMsgProtoBuf(CGameSvrMgr::GetInstancePtr()->GetConnIDBySvrID(m_dwCopySvrID), dwMsgID, m_u64ID, m_dwCopyGuid, pdata);
}

BOOL CPlayerObject::OnAllModuleOK()
{
	ERROR_RETURN_FALSE(m_u64ID != 0);
	CalcFightDataInfo();
	SendRoleLoginAck();
	CGameSvrMgr::GetInstancePtr()->SendPlayerToMainCity(m_u64ID, GetCityCopyID());
	m_dwCopyID = 0;
	m_dwCopyGuid = 0;
	return TRUE;
}

UINT32 CPlayerObject::CheckCopyConditoin(UINT32 dwCopyID)
{
	return TRUE;
	StCopyInfo* pCopyInfo = CConfigData::GetInstancePtr()->GetCopyInfo(dwCopyID);
	ERROR_RETURN_CODE(m_u64ID != 0, MRC_INVALID_COPYID);

	CRoleModule* pRoleModule = (CRoleModule*)GetModuleByType(MT_ROLE);
	ERROR_RETURN_CODE(pRoleModule != NULL, MRC_UNKNOW_ERROR);

	if(!pRoleModule->CheckActionEnough(pCopyInfo->dwCostActID, pCopyInfo->dwCostActNum))
	{
		return MRC_NOT_ENOUGH_ACTOIN;
	}

	CCopyModule* pCopyModule = (CCopyModule*)GetModuleByType(MT_COPY);
	ERROR_RETURN_CODE(pCopyModule != NULL, MRC_UNKNOW_ERROR);

	CopyDataObject* pObject = pCopyModule->GetCopyData(dwCopyID);
	ERROR_RETURN_CODE(pObject != NULL, MRC_UNKNOW_ERROR);

	if(pObject->m_dwBattleCnt >= pCopyInfo->dwBattleTimes)
	{
		return MRC_NOT_ENOUGH_TIMES;
	}

	return MRC_SUCCESSED;
}

BOOL CPlayerObject::SendIntoSceneNotify(UINT32 dwCopyGuid, UINT32 dwCopyID, UINT32 dwSvrID)
{
	ERROR_RETURN_FALSE(dwCopyID != 0);
	ERROR_RETURN_FALSE(dwCopyGuid != 0);
	ERROR_RETURN_FALSE(dwSvrID != 0);

	ERROR_RETURN_FALSE(m_dwCopyGuid != dwCopyGuid);
	ERROR_RETURN_FALSE(m_dwCopyID != dwCopyID);

	NotifyIntoScene Nty;
	Nty.set_copyid(dwCopyID);
	Nty.set_copyguid(dwCopyGuid);
	Nty.set_serverid(dwSvrID);
	Nty.set_roleid(m_u64ID);
	ERROR_RETURN_FALSE(m_u64ID != 0);
	SendMsgProtoBuf(MSG_NOTIFY_INTO_SCENE, Nty);
	return TRUE;
}

BOOL CPlayerObject::SendLeaveScene(UINT32 dwCopyGuid, UINT32 dwSvrID)
{
	LeaveSceneReq LeaveReq;
	LeaveReq.set_roleid(m_u64ID);
	ServiceBase::GetInstancePtr()->SendMsgProtoBuf(CGameSvrMgr::GetInstancePtr()->GetConnIDBySvrID(dwSvrID), MSG_LEAVE_SCENE_REQ, m_u64ID, dwCopyGuid, LeaveReq);
	return TRUE;
}

BOOL CPlayerObject::SetConnectID(UINT32 dwProxyID, UINT32 dwClientID)
{
	m_dwProxyConnID = dwProxyID;
	m_dwClientConnID = dwClientID;

	return TRUE;
}


CModuleBase* CPlayerObject::GetModuleByType(UINT32 dwModuleType)
{
	if(dwModuleType >= (UINT32)m_MoudleList.size())
	{
		LOG_ERROR;
		return NULL;
	}

	return m_MoudleList.at(dwModuleType);
}

UINT64 CPlayerObject::GetObjectID()
{
	return m_u64ID;
}

UINT32 CPlayerObject::GetCityCopyID()
{
	CRoleModule* pModule = (CRoleModule*)GetModuleByType(MT_ROLE);
	ERROR_RETURN_FALSE(pModule != NULL);
	ERROR_RETURN_FALSE(pModule->m_pRoleDataObject != NULL);
	return pModule->m_pRoleDataObject->m_CityCopyID;
}

BOOL CPlayerObject::SendRoleLoginAck()
{
	CRoleModule* pModule = (CRoleModule*)GetModuleByType(MT_ROLE);
	ERROR_RETURN_FALSE(pModule != NULL);
	RoleLoginAck Ack;
	Ack.set_retcode(MRC_SUCCESSED);
	for(int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);
		pBase->SaveToClientLoginData(Ack);
	}

	SendMsgProtoBuf(MSG_ROLE_LOGIN_ACK, Ack);
	return TRUE;
}

BOOL CPlayerObject::SendObjectChangeNtf(UINT32 dwChangeType, UINT64 uIntValue1, UINT64 uIntValue2, std::string strValue)
{
	ObjectChangeNotify Ntf;
	Ntf.set_roleid(GetObjectID());
	Ntf.set_changetype(dwChangeType);
	Ntf.set_intvalue1(uIntValue1);
	Ntf.set_intvalue2(uIntValue2);
	if (!strValue.empty() && strValue.size() > 0)
	{
		Ntf.set_strvalue(strValue);
	}

	SendMsgToScene(MSG_SCENEOBJ_CHAGE_NTF, Ntf);

	return TRUE;
}

BOOL CPlayerObject::ToTransferData( TransferDataReq& Req )
{
	CRoleModule* pModule = (CRoleModule*)GetModuleByType(MT_ROLE);

	TransRoleData* pRoleData = Req.mutable_roledata();
	ERROR_RETURN_FALSE(pRoleData != NULL);


	pRoleData->set_roleid(m_u64ID);
	pRoleData->set_carrerid(pModule->m_pRoleDataObject->m_CarrerID);
	pRoleData->set_actorid(pModule->GetActorID());
	pRoleData->set_level(pModule->m_pRoleDataObject->m_Level);
	pRoleData->set_name(pModule->m_pRoleDataObject->m_szName);

	for(int i = 0; i < PROPERTY_NUM; i++)
	{
		pRoleData->add_propertys(m_Propertys[i]);
	}

	CEquipModule* pEquipModule = (CEquipModule*)GetModuleByType(MT_EQUIP);
	ERROR_RETURN_FALSE(pEquipModule != NULL);

	for (int i = 0; i < EQUIP_MAX_NUM; i++)
	{
		EquipDataObject* pEquip = pEquipModule->m_vtDressEquip[i];
		if (pEquip == NULL)
		{
			pRoleData->add_equips(0);
		}
		else
		{
			pRoleData->add_equips(pEquip->m_EquipID);
		}
	}

	//CPetModule* pPetModule = (CPetModule*)GetModuleByType(MT_PET);


	return TRUE;
}

BOOL CPlayerObject::NotifyTaskEvent(UINT32 dwEventID, UINT32 dwParam1, UINT32 dwParam2)
{
	CTaskModule* pTaskModule = (CTaskModule*)GetModuleByType(MT_TASK);
	pTaskModule->OnTaskEvent((ETaskEvent)dwEventID, dwParam1, dwParam2);
	return TRUE;
}

BOOL CPlayerObject::IsOnline()
{
	if (m_IsOnline && m_dwProxyConnID == 0)
	{
		ASSERT_FAIELD;
		return FALSE;
	}

	return m_IsOnline;
}

BOOL CPlayerObject::NotifyChange()
{
	for (int i = MT_ROLE; i < MT_END; i++)
	{
		CModuleBase* pBase = m_MoudleList.at(i);
		ERROR_RETURN_FALSE(pBase != NULL);

		pBase->NotifyChange();
	}
	return TRUE;
}

BOOL CPlayerObject::CalcFightDataInfo()
{
	INT32 PropertyValue[PROPERTY_NUM] = {0};
	INT32 PropertyPercent[PROPERTY_NUM] = {0};
	INT32 FightValue = 0;

	for(int nIndex = MT_ROLE; nIndex < MT_END; nIndex++)
	{
		CRoleModule* pTempModule = (CRoleModule*)GetModuleByType(nIndex);
		pTempModule->CalcFightValue(PropertyValue, PropertyPercent, FightValue);
	}

	CRoleModule* pModule = (CRoleModule*)GetModuleByType(MT_ROLE);
	ERROR_RETURN_FALSE(pModule != NULL);
	StLevelInfo* pLevelInfo = CConfigData::GetInstancePtr()->GetCarrerLevelInfo(pModule->m_pRoleDataObject->m_CarrerID, pModule->m_pRoleDataObject->m_Level);
	ERROR_RETURN_FALSE(pLevelInfo != NULL);
	memcpy(PropertyValue, pLevelInfo->Propertys, sizeof(INT32)*PROPERTY_NUM);

	for(int i = 0; i < PROPERTY_NUM; i++)
	{
		m_Propertys[i] = PropertyValue[i] + PropertyValue[i] * PropertyPercent[i] / 10000;
	}

	return TRUE;
}

BOOL CPlayerObject::ClearCopyState()
{
	m_dwCopyGuid = 0;        //当前的副本ID
	m_dwCopyID = 0;        //当前的副本类型
	m_dwCopySvrID = 0;        //副本服务器的ID
	m_dwToCopyGuid = 0;        //正在前往的副本ID
	m_dwToCopyID = 0;        //正在前往的副本类型
	m_dwToCopySvrID = 0;

	return TRUE;
}






