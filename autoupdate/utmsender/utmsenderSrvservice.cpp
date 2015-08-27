#include "stdafx.h"
#include "utmsenderSrvservice.h"
#include "Service.h"
#include "cacti/util/IniFile.h"
#include "cacti/logging/LazyLog.h"

#include "sessionobject.h"
#include "twglobal.h" 
#include "rule.h"
#include "TLuaExec.h"
#include "../../timeschedule/timestrategy.h"

cacti::LazyLog g_serviceLog("module");

// ȫ�ֺ���
// ��ӡ��־
void mprint(const char* fmt, ...)
{
	static char	msg[1024];
	va_list args;
	va_start(args,fmt);
	_vsnprintf_s(msg, 1022, fmt, args);
	va_end(args);

	// ʱ��
	time_t t = time(NULL);
	struct tm tm;
	localtime_s(&tm, &t);

	static char line[1048];
	sprintf_s(line, "[%02d:%02d:%02d] %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, msg);

	// ��ʱʹ��printf��ӡ
	//printf("%s", line);

	g_serviceLog.print(line);
}


MTL_BEGIN_MESSAGE_MAP(USService)
MTL_ON_MESSAGE(_EvtStartSession, onCreateSession)
MTL_ON_MESSAGE(_EvtTimeOut, onTimeOut)
MTL_ON_MESSAGE(_EvtKillSession, onKillSession)
MTL_ON_MESSAGE(_EvtStartWork,onStartWork)
//MTL_ON_MESSAGE(_EvtRecognizeResult, onRecognizeResult)		// ���Ը���Ϣ
MTL_END_MESSAGE_MAP

USService::USService(int appport, MessageDispatcher * dispatcher )
:ServiceSkeleton(appport, dispatcher)
{
	g_serviceLog.open();

	//m_pStrategy = NULL;
 }

USService::~USService()
{
	uninit();
	Service::printConsole("AlarmServer Service exit\n");
}

bool USService::init()
{
	Service::printConsole("AlarmServer Service init OK \n");
	m_testUnit.setOwner(this);

	// ����lua����������������
	if (0 != TLuaExec::LoadAndExecLua("./system/systemop.lua"))
	{
		g_serviceLog.print("[Error] Load lua fail\n");
	}

	// ���غ궨��
	LoadAllMacroes();

	// ����peer��Ϣ
	//LoadPeers();

	// ����ʱ�����
// 	unsigned int day = GetPrivateProfileIntEx("repeat", "day", 1, SYSTEM_CFG_FILE);
// 	if (day <=0 || day >= 31)
// 	{
// 		day = 1;
// 	}
	
	// ����Ҫ��ʱִ��
// 	char szTime[1024];
// 	GetPrivateProfileString("repeat", "time", "00:00:00", szTime, 1024, SYSTEM_CFG_FILE);
// 	unsigned int hour = 0;
// 	unsigned int minute = 0;
// 	unsigned int second = 0;
// 	// ������ʱ���֡���
// 	sscanf_s(szTime, "%d:%d:%d", &hour, &minute, &second);
// 	hour =  (hour > 23) ? 0 : hour;
// 	minute = (minute > 59) ? 0 : minute;
// 	second = (second > 59) ? 0 : second;
// 	
// 	m_pStrategy = new MonthStrategy(day, hour, minute, second);
// 	m_timeSchedule.Register(this, m_pStrategy);
// 	m_timeSchedule.Start();

	// �����󣬿����Զ����¶�ʱ��
	UserTransferMessagePtr utm(new UserTransferMessage);
	utm->setMessageId(_EvtStartWork);
	
	unsigned int nWaitTimeAfterInit = GetPrivateProfileIntEx("system", "waittimeafterinit", 5, SYSTEM_CFG_FILE);
	printf("nWaitTimeAfterInit=%d\n", nWaitTimeAfterInit);
	setTimer(nWaitTimeAfterInit * 1000, utm);
	return true;
}

void USService::uninit()
{
// 	if (m_pStrategy)
// 	{
// 		delete m_pStrategy;
// 		m_pStrategy = NULL;
// 	}

	m_sessionManager.ClearAll();
	return;
}


void USService::onDefaultMessage(const ServiceIdentifier &sender,UserTransferMessage& utm)
{
	// �жϸ���Ϣ�Ƿ�Ϊ��ǰ״̬�£�ĳ��session���ڵȴ�����Ϣ������ǣ��򴥷���session����������
	if (IsSessionMsg(sender, utm))
	{
		return;
	}	

	string sID = GetSIDByIID(utm.getMessageId());

	switch(utm.getMessageId())
	{
	case _EvtModuleActive:
		onModuleActive(sender,utm);
		break;
	case _EvtModuleDeactive:
		onModuleDeactive(sender,utm);
		break;
	default:
		if (sID == "")
		{
			g_serviceLog.print("[INFO] Recv Message (0x%08X) but no handler\n", utm.getMessageId());
		}
		else
		{
			g_serviceLog.print("[INFO] Recv Message (%s) but no handler\n", sID.c_str());
		}
		break;
	}

}

void USService::onModuleActive(const ServiceIdentifier &sender, UserTransferMessage &utm)
{
	printf("Module active: %s\n", sender.toString2().c_str());
	m_dataSource.addServiceIDByApport(sender.m_appport, sender);	
}

void USService::onModuleDeactive(const ServiceIdentifier &sender, UserTransferMessage &utm)
{
	printf("Module deactive: %s\n", sender.toString2().c_str());
	m_dataSource.removeServiceIDByApport(sender.m_appport, sender);
}

void USService::ResponseUTM(const ServiceIdentifier& sender, UserTransferMessage& utm, int status)
{
	// TODO
}

void USService::onCreateSession(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	int sessionID = m_sessionManager.GenerateSessionID();
	TLuaSession *pSessionObject = new TLuaSession(this, sessionID);
	if (!pSessionObject)
	{
		ResponseUTM(sender, utm, 1);
		return;
	}

	// ��ӵ�������
	m_sessionManager.AddSessionObject(pSessionObject);

	// ȡ����
	int flowID = utm[_TagFlowId];

	// �������̣�Ŀǰֻʹ�õ�2�����̣�����ֱ�Ӷ���
	char sLuaFile[1024];
	strcpy_s(sLuaFile, "./lua/autoupdate.lua");
// 	if (flowID == 1)
// 	{
// 		strcpy_s(sLuaFile, "./lua/autoupdate.lua");
// 	}
// 	else if (flowID == 2)
// 	{
// 		strcpy_s(sLuaFile, "./lua/heartbeat.lua");
// 	}
// 	else
// 		return;

// 	char sKey[100];
// 	sprintf_s(sKey, 100, "lua.%d", flowID);
// 	GetPrivateProfileString("lua", sKey, "./lua/autoupdate.lua", sLuaFile, sizeof(sLuaFile), SYSTEM_CFG_FILE);
// 	printf("lua source: %s\n", sLuaFile);
	if (pSessionObject->LoadScript(sLuaFile)) // TODO: ��ʱд��
	{
		ResponseUTM(sender, utm, 2);
		m_sessionManager.DeleteSessionObject(pSessionObject->m_SessionID);
		return;
	}

	// ִ������
	if (pSessionObject->ExecScript())			// ִ�нű�ʧ��
	{
		ResponseUTM(sender, utm, 2);
		m_sessionManager.DeleteSessionObject(pSessionObject->m_SessionID);
		return;
	}
}

/************************************************************************/
/* 
����: �����ִ�����ϢID�õ�������ϢID
����: 
	sID ---- �ִ�����ϢID
����: 
	������ϢID���粻���ڣ����� IVALID_MESSAGEID
*/
/************************************************************************/
int USService::GetIIDBySID(const string& sID)
{
	map<string, int>::iterator it = m_strID2IID.find(sID);
	if (it != m_strID2IID.end())
	{
		return it->second;
	}

	return INVALID_MESSID;
}

/************************************************************************/
/* 
����: ����������ϢID�õ��ִ�����ϢID
����: 
	iID ---- ������ϢID
����: 
	������ϢID���粻���ڣ����ؿմ�
*/
/************************************************************************/
string USService::GetSIDByIID(int iID)
{
	map<int, string>::iterator it = m_iID2SID.find(iID);
	if (it != m_iID2SID.end())
	{
		return it->second;
	}

	return "";
}

/************************************************************************/
/* 
����: ��ͷ�ļ��м��ز��������к�
����: 
����: 
*/
/************************************************************************/
// #define ADDIDMAP(sID, iID) m_strID2IID[#sID] = iID; \
// 	m_iID2SID[iID] = #sID;
void USService::LoadAllMacroes()
{
	// �������ִ������͵�ӳ��

	// ����ÿ��ͷ�ļ�
	LoadOneFile(".\\common\\evtmsg.h");
	//LoadOneFile(".\\common\\reqmsg.h");
	//LoadOneFile(".\\common\\resmsg.h");
	
	// tag.h�ĺ�ֵȫ���ڵ��� 0x80000000��ʹ�����ⷽ������
	//LoadBig8File(".\\common\\tag.h");

	// ���ڵ��� 0x80000000������ѽ��
	//LoadOneFile(".\\common\\tag.h");
}

/************************************************************************/
/* 
����: ����һ��ͷ�ļ�
����: 
����: 
*/
/************************************************************************/
void USService::LoadOneFile(const string& filename)
{
	FILE *pf = fopen(filename.c_str(), "r");
	if (!pf)
	{
		g_serviceLog.print("[Error] Load macro file failed, open failed: %s\n", filename.c_str());
		return;
	}

	int intValue;
	char sKeyName[100];
	char line[1024];
	// �����ļ�ÿһ��
	while (fgets(line, 1024, pf))
	{	
		int ret = 1;
		if (0 != TLuaExec::CallLuaFunction(0, "ParseLine", "s>isi", line, &ret, sKeyName, &intValue))
		{
			g_serviceLog.print("[Error] Call lua function failed\n");
			continue;
		}

		// �жϽ����н��
		if (0 == ret)
		{
			m_strID2IID[sKeyName] = intValue;
			m_iID2SID[intValue] = sKeyName;

			//g_serviceLog.print("%-30s = 0x%08X\n", sKeyName, intValue);
		}
	}

	fclose(pf);
}

/************************************************************************/
/* 
����: ר�Ž���ֵ���ڵ���0x80000000�ĺ�
����: 
����: 
*/
/************************************************************************/
void USService::LoadBig8File(const string& filename)
{
	FILE *pf = fopen(filename.c_str(), "r");
	if (!pf)
	{
		g_serviceLog.print("[Error] Load macro file failed, open failed: %s\n", filename.c_str());
		return;
	}

	char intValue[20];
	char sKeyName[100];
	char line[1024];
	// �����ļ�ÿһ��
	while (fgets(line, 1024, pf))
	{
		int ret = 1;
		if (0 != TLuaExec::CallLuaFunction(0, "ParseLineBig8", "s>iss", line, &ret, sKeyName, intValue))
		{
			g_serviceLog.print("[Error] Call lua function failed\n");
			continue;
		}

		// �жϳ��ȣ��Ϊ10���ֽ� "0xffffffff"
		if (strlen(intValue) > 10)
		{
			continue;
		}

		// ��ʮ��������ʽ���ִ�ת��Ϊ����
		//char *str;
		//unsigned int tIntValue = (int)strtol(intValue, &str, 16);
		int tIntValue = 0;
		sscanf(intValue, "%x", &tIntValue);   

		// �жϽ����н��
		if (0 == ret)
		{
			m_strID2IID[sKeyName] = tIntValue;
			m_iID2SID[tIntValue] = sKeyName;
		}
	}

	fclose(pf);
}

// /************************************************************************/
// /* 
// ����: ����peers��Ϣ
// ����: 
// ����: 
// */
// /************************************************************************/
// void USService::LoadPeers()
// {
// // 	int appid = 0;
// // 	int appport = 0;
// // 	int id = 0;
// // 	char key[100];
// // 	char url[1024];
// // 	int luaRet = -1;
// // 
// // 	for (id = 1; ; id++;)
// // 	{
// // 		sprintf_s(key, "peers%d", id);
// // 		GetPrivateProfileString("peers", key, url, 1024, SYSTEM_CFG_FILE);
// // 		if (0 == strcmp(url, ""))
// // 		{
// // 			break;
// // 		}	
// // 	}
// }

/************************************************************************/
/* 
����: ��ʱ����������
����: 
����: 
*/
/************************************************************************/
void USService::onTimeOut(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	// TODO: ���ⶨʱ���ж�

	// ����session
	u32 sessionID = utm[_TagSessionID];
	TLuaSession *pSessionObject = m_sessionManager.GetSessionObject(sessionID);
	if (!pSessionObject)
	{
		mprint("[Error]: SessionObject not exist, SessionID=0x%08x", sessionID);
		return;
	}

	// ״̬��ת
	// ����������ϢID�õ��ִ�����ϢID
	int iID = utm.getMessageId();
	string sID = GetSIDByIID(iID);
	if (sID == "")
	{
		mprint("[Error] OnTimer: No str message id according to msgID(0x%08x)", iID);
		return;
	}

	// ��ת����ƥ��
	Rule matchedRule;	// ƥ�����
	int ret = pSessionObject->GetMatchedRule(Rule(iID, sID, utm.getReturn(), ""), matchedRule);
	if (ret != 0)
	{
		mprint("[Info] OnTimer: No matched rule");
		return;
	}

	// ֹͣ��ʱ��
	pSessionObject->StopTimer();

	// ��ת����һ״̬
	pSessionObject->CallLuaFunc(matchedRule.m_state.GetFuncName().c_str());
}

/************************************************************************/
/* 
����: �ر�session
����: 
����: 
*/
/************************************************************************/
void USService::onKillSession(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	u32 sessionID = utm[_TagSessionID];
	m_sessionManager.DeleteSessionObject(sessionID);
}

/************************************************************************/
/* 
����: ����ʶ������Ϣ
����: 
����: 
*/
/************************************************************************/
void USService::onRecognizeResult(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	// ��������� onTimeOut ���ƣ�����Ҫ��utm�����в�����ȷ���ݸ�lua

	// ��utm�����в�����ȷ���ݸ�lua

	// ����: 
	// �ڴ���ÿ����Ϣʱ����c�н�������key-value����

	// utm���ݴ���
	// ��ջL�ﴴ����ʱtable
// 	lua_newtable(m_L);  
// 	// ��utm���������ݷ�����ʱtable������
// 	for (int i=1; i<=MAX_FIELD_COUNT; i++)
// 	{
// 		lua_pushinteger(m_L, i);
// 		lua_pushstring(m_L, m_Field[i-1]);
// 		lua_rawset(m_L, -3);
// 	} 
// 	lua_setglobal(m_L, LC_FIELD);


}

/************************************************************************/
/*
����: �ж�utm�Ƿ�Ϊĳsession���ڵȴ�����Ϣ
����: 
����: 
	true ---- ��
	false --- ����
*/
/************************************************************************/
bool USService::IsSessionMsg(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	// ����session
	u32 sessionID = utm.getRes().m_appref; 
	TLuaSession *pSessionObject = m_sessionManager.GetSessionObject(sessionID);
	if (!pSessionObject)
	{
		return false;
	}

	// ע��: ��ʹ���ҵ�session��Ҳ����֤����utm�Ϳ϶�����Ҫ�͸���session�ģ���Ҫ�����ж�

	// �жϸ�utm�Ƿ�Ϊ��session���ڵȴ�����Ϣ��ͨ������ƥ���ж�

	// ״̬��ת
	// ����������ϢID�õ��ִ�����ϢID
	int iID = utm.getMessageId();
	string sID = GetSIDByIID(iID);
	if (sID == "")
	{
		mprint("[Error] IsSessionMsg: No str message id according to msgID(0x%08x)", iID);
		return false;
	}

	// ��ת����ƥ��
	Rule matchedRule;	// ƥ�����
	int ret = pSessionObject->GetMatchedRule(Rule(iID, sID, utm.getReturn(), ""), matchedRule);
	if (ret != 0)
	// �Ҳ���ƥ�����˵����utm���Ǹ�session���ڵȴ�����Ϣ
	{
		return false;
	}

	// ֹͣ��ʱ��
	pSessionObject->StopTimer();

	// ��sessi on�������һ�ν��յ������utm
	pSessionObject->SetLastValidUtm(utm);

	// ��ת����һ״̬
	pSessionObject->CallLuaFunc(matchedRule.m_state.GetFuncName().c_str());

	return 1;
}

/************************************************************************/
/*
����: ��ȡappport��Ӧ�����service id
����: 
����: 
	0 ---- �ɹ�
	���� - ʧ��
*/
/************************************************************************/
int USService::GetSIDByApport(int appport, ServiceIdentifier& sid)
{
	return m_dataSource.getServiceIDByApport(appport, sid);
}

/************************************************************************/
/*
����: ����ڴ����
����: 
����: 
*/
/************************************************************************/
void USService::SnapShot()
{
	m_sessionManager.Snapshot(NULL);
}

/************************************************************************/
/*
����: ʱ�����ڵ���ʱ�Ļص�
����: 
����: 
*/
/************************************************************************/
void USService::Notify()
{
	printf("on time to execute\n");

	// ��������
	UserTransferMessagePtr utm(new UserTransferMessage);
	utm->setMessageId(_EvtStartSession);
	//utm->setReq(ServiceIdentifier(4, AppPort::_apAlarmProxy, 0));
	//utm->setRes(m_owner->myIdentifier());
	//utm->setReturn(99);

	//(*utm)[_TagFlowId] = (u32)999;
	//(*utm)[_TagSessionID] = "kkkk";

	postSelfMessage(utm);
}

/************************************************************************/
/*
����: ��ʼ�����ű�
����: 
����: 
*/
/************************************************************************/
void USService::onStartWork(const ServiceIdentifier& sender, UserTransferMessage& utm)
{
	UserTransferMessagePtr startSessionUtm(new UserTransferMessage);
	(*startSessionUtm)[_TagFlowId] = (u32)1;
	// ����������Ҫ����ز���
	startSessionUtm->setMessageId(_EvtStartSession);
	startSessionUtm->setReq(ServiceIdentifier(4, AppPort::_apAlarmProxy, 0));
	startSessionUtm->setRes(myIdentifier());
		
	postSelfMessage(startSessionUtm);
}