#include "stdafx.h"
#include "sessionobject.h"
#include "utmsenderSrvservice.h"
#include "twglobal.h"
#include "CDownloadCTL.h"
#include "cacti\util\FileSystem.h"
using namespace cacti;

extern "C"
{ 
#include "lua.h"  
#include "lualib.h"
#include "lauxlib.h"
}

// lua��cͨ�õ�ȫ�ֱ�����
#define LC_MSG_STATUS "_Status"
#define LC_RESULT "_Result"
#define lC_SESSION_ID "_SessionID"

extern USService *g_usservice;
extern void mprint(const char* fmt, ...);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]= __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////
/* ע�����Ա������lua */
// ע��DataAccess��ResponseMessage��3��int���������һ��int�������
typedef int (TLuaSession::*Fun_iii_i)(int, int, int);

//����ָ��������ݻ�浽UpValue�ĵ�һ��ֵ�У��˴���upvalue��ȡ��
unsigned char* getFirstUpValue(lua_State *L)
{	
	unsigned char* buffer = (unsigned char*)lua_touserdata(L, lua_upvalueindex(1));	
	return buffer;	
}

/*
ʵ�ֶ�class����memeber function�ĵ���  
����cla Ҫ���õ����ʵ��	
TLuaSession::*func ��ĺ���ָ��	  
*/
int callMemFunc(int sessionID, Fun_iii_i func, lua_State *L, int index)
{		
	int v1 = 0;
	int v2 = 0;
	int v3 = 0;

#ifdef _DEBUG
	// ���L�������Ϣ
	//printf("stack top = 0x%08x\n", L->stack);
#endif

	// ��luaջ��ȡ��ʵ��
	if (lua_isnumber(L, 1))
	{ 
		v1 = lua_tointeger(L, 1);
	}
	if (lua_isnumber(L, 2))
	{ 
		v2 = lua_tointeger(L, 2);
	}
	if (lua_isnumber(L, 3))
	{ 
		v3 = lua_tointeger(L, 3);
	}

	// ��module�л�ȡsession����
	TLuaSession* pSessionObject = NULL;
	pSessionObject = g_usservice->m_sessionManager.GetSessionObject(sessionID);
	if (pSessionObject)
	{
		return (pSessionObject->*func)(v1, v2, v3);
	}

	return 1;	
}

/*
ʵ��callMemFunc��lua������ʽ��װ  
*/
int directCallMemFunc(lua_State *L)
{	
	//�õ�����ָ��	
	unsigned char* buffer = getFirstUpValue(L);

	//ת������Ӧ�ĺ�������	
	int ret = callMemFunc(*(int*)(buffer), *(Fun_iii_i*)(buffer + sizeof(int)), L, 1);	
	lua_pushnumber(L, ret);   // ����Ա�����ķ��ؽ����ջ���Է��ظ�lua
	return 1;
}

/*
��directCallMemFuncע���lua  
*/
void lua_pushdirectmemclosure(lua_State *L, int sessionID, Fun_iii_i func, unsigned int nupvalues)
{	
	//����userdata����cla��funcָ���ֵ������ȥ	
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(int) + sizeof(func));	
	memcpy(buffer, &sessionID, sizeof(int));
	memcpy(buffer + sizeof(int), &func, sizeof(func));	
	lua_pushcclosure(L, directCallMemFunc, nupvalues + 1);
}

#define lua_directregistry_memfunc_iii_i(L, name, sessionID, func) \
	lua_pushstring(L, name); \
	lua_pushdirectmemclosure(L, sessionID, func, 0); \
	lua_settable(L, LUA_GLOBALSINDEX);

// ע�� SetNextState  �������: int int string  �������: int
typedef int (TLuaSession::*Fun_sis_i)(string, int, string);

/*
ʵ�ֶ�class����memeber function�ĵ���  
����cla Ҫ���õ����ʵ��	
TLuaSession::*func ��ĺ���ָ��	  
*/
int callMemFunc_sis_i(int sessionID, Fun_sis_i func, lua_State *L, int index)
{		
	string v1 = "";
	int v2 = 0;
	string v3 = "";

	// ��luaջ��ȡ��ʵ��
	if (lua_isstring(L, 1))
	{ 
		v1 = lua_tostring(L, 1);
	}
	if (lua_isnumber(L, 2))
	{ 
		v2 = lua_tointeger(L, 2);
	}
	if (lua_isstring(L, 3))
	{ 
		v3 = lua_tostring(L, 3);
	}

	// ��module�л�ȡsession����
	TLuaSession* pSessionObject = NULL;
	pSessionObject = g_usservice->m_sessionManager.GetSessionObject(sessionID);
	if (pSessionObject)
	{
		return (pSessionObject->*func)(v1, v2, v3);
	}

	return 1;	
}

/*
ʵ��callMemFunc��lua������ʽ��װ  
*/
int directCallMemFunc_sis_i(lua_State *L)
{	
	//�õ�����ָ��	
	unsigned char* buffer = getFirstUpValue(L);

	//ת������Ӧ�ĺ�������	
	int ret = callMemFunc_sis_i(*(int*)buffer, *(Fun_sis_i*)(buffer + sizeof(int)), L, 1);
	lua_pushinteger(L, ret);
	return 1;
}

/*
��directCallMemFuncע���lua  
*/
void lua_pushdirectmemclosure_sis_i(lua_State *L, int sessionID, Fun_sis_i func, unsigned int nupvalues)
{	
	//����userdata����cla��funcָ���ֵ������ȥ	
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(int) + sizeof(func));	
	memcpy(buffer, &sessionID, sizeof(int));
	memcpy(buffer + sizeof(int), &func, sizeof(func));

	lua_pushcclosure(L, directCallMemFunc_sis_i, nupvalues + 1);	
}

#define lua_directregistry_memfunc_sis_i(L, name, sessionID, func) \
	lua_pushstring(L, name); \
	lua_pushdirectmemclosure_sis_i(L, sessionID, func, 0); \
	lua_settable(L, LUA_GLOBALSINDEX);

// ע�� SendHttpReq  �������: string int int int  �������: int
typedef int (TLuaSession::*Fun_siii_i)(string, int, int, int);

/*
ʵ�ֶ�class����memeber function�ĵ���  
����cla Ҫ���õ����ʵ��	
TLuaSession::*func ��ĺ���ָ��	  
*/
int callMemFunc_siii_i(int sessionID, Fun_siii_i func, lua_State *L, int index)
{		
	string v1 = "";
	int v2 = 0;
	int v3 = 0;
	int v4 = 0;

	// ��luaջ��ȡ��ʵ��
	if (lua_isstring(L, 1))
	{ 
		v1 = lua_tostring(L, 1);
	}
	if (lua_isnumber(L, 2))
	{ 
		v2 = lua_tointeger(L, 2);
	}
	if (lua_isnumber(L, 3))
	{ 
		v3 = lua_tointeger(L, 3);
	}
	if (lua_isnumber(L, 4))
	{ 
		v4 = lua_tointeger(L, 4);
	}

	// ��module�л�ȡsession����
	TLuaSession* pSessionObject = NULL;
	pSessionObject = g_usservice->m_sessionManager.GetSessionObject(sessionID);
	if (pSessionObject)
	{
		return (pSessionObject->*func)(v1, v2, v3, v4);
	}

	return 1;	
}

/*
ʵ��callMemFunc��lua������ʽ��װ  
*/
int directCallMemFunc_siii_i(lua_State *L)
{	
	//�õ�����ָ��	
	unsigned char* buffer = getFirstUpValue(L);

	//ת������Ӧ�ĺ�������	
	int ret = callMemFunc_siii_i(*(int*)buffer, *(Fun_siii_i*)(buffer + sizeof(int)), L, 1);	
	lua_pushnumber(L, ret);   // ����Ա�����ķ��ؽ����ջ���Է��ظ�lua
	return 1;
}

/*
��directCallMemFuncע���lua  
*/
void lua_pushdirectmemclosure_siii_i(lua_State *L, int sessionID, Fun_siii_i func, unsigned int nupvalues)
{	
	//����userdata����cla��funcָ���ֵ������ȥ	
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(int) + sizeof(func));	
	memcpy(buffer, &sessionID, sizeof(int));
	memcpy(buffer + sizeof(int), &func, sizeof(func));

	lua_pushcclosure(L, directCallMemFunc_siii_i, nupvalues + 1);	
}

#define lua_directregistry_memfunc_siii_i(L, name, sessionID, func) \
	lua_pushstring(L, name); \
	lua_pushdirectmemclosure_siii_i(L, sessionID, func, 0); \
	lua_settable(L, LUA_GLOBALSINDEX);
///////////////////////////////////////////////////////////////////////////

// ע�� SendHttpReq  �������: string int int int  �������: int
typedef int (TLuaSession::*Fun_sss_i)(string, string, string);

/*
ʵ�ֶ�class����memeber function�ĵ���  
����cla Ҫ���õ����ʵ��	
TLuaSession::*func ��ĺ���ָ��	  
*/
int callMemFunc_sss_i(int sessionID, Fun_sss_i func, lua_State *L, int index)
{		
	string v1 = "";
	string v2 = "";
	string v3 = "";

	// ��luaջ��ȡ��ʵ��
	if (lua_isstring(L, 1))
	{ 
		v1 = lua_tostring(L, 1);
	}
	if (lua_isstring(L, 2))
	{ 
		v2 = lua_tostring(L, 2);
	}
	if (lua_isstring(L, 3))
	{ 
		v3 = lua_tostring(L, 3);
	}

	// ��module�л�ȡsession����
	TLuaSession* pSessionObject = NULL;
	pSessionObject = g_usservice->m_sessionManager.GetSessionObject(sessionID);
	if (pSessionObject)
	{
		return (pSessionObject->*func)(v1, v2, v3);
	}

	return 1;	
}

/*
ʵ��callMemFunc��lua������ʽ��װ  
*/
int directCallMemFunc_sss_i(lua_State *L)
{	
	//�õ�����ָ��	
	unsigned char* buffer = getFirstUpValue(L);

	//ת������Ӧ�ĺ�������	
	int ret = callMemFunc_sss_i(*(int*)buffer, *(Fun_sss_i*)(buffer + sizeof(int)), L, 1);	
	lua_pushnumber(L, ret);   // ����Ա�����ķ��ؽ����ջ���Է��ظ�lua
	return 1;
}

/*
��directCallMemFuncע���lua  
*/
void lua_pushdirectmemclosure_sss_i(lua_State *L, int sessionID, Fun_sss_i func, unsigned int nupvalues)
{	
	//����userdata����cla��funcָ���ֵ������ȥ	
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(int) + sizeof(func));	
	memcpy(buffer, &sessionID, sizeof(int));
	memcpy(buffer + sizeof(int), &func, sizeof(func));

	lua_pushcclosure(L, directCallMemFunc_sss_i, nupvalues + 1);	
}

#define lua_directregistry_memfunc_sss_i(L, name, sessionID, func) \
	lua_pushstring(L, name); \
	lua_pushdirectmemclosure_sss_i(L, sessionID, func, 0); \
	lua_settable(L, LUA_GLOBALSINDEX);
///////////////////////////////////////////////////////////////////////////

// ����lua����
int call_lua_funcion(const UserTransferMessage& utm, TLuaSession* pSession, lua_State* L, const char* func)
{
// 	if (!pSession)
// 	{
// 		return 1;
// 	}
// 
// 	if (!L || !func || 0==strlen(func))
// 	{
// 		return 1;
// 	}
// 
// 	int narg;
// 	int nret;
// 	int nindex;
// 
// 	lua_getglobal(L, func);
// 	if(0 == lua_isfunction(L, -1))
// 	{
// 		lua_pop(L, 1);
// 		// lua����������
// 		mprint("[0x%08X][Error](%s) is not a lua function", pSession->m_SessionID, func);
// 		return 1;
// 	}
// 
// 	va_list ap;
// 	va_start(ap, fmt);
// 	for(narg=0; *fmt; ++narg){
// 		luaL_checkstack(L, 1, "too many arguments");
// 
// 		switch(*fmt)
// 		{
// 		case 'd':
// 			lua_pushnumber(L, va_arg(ap, double));
// 			break;
// 		case 'i':
// 			lua_pushinteger(L, va_arg(ap, int));
// 			break;
// 		case 's':
// 			lua_pushstring(L, va_arg(ap, char*));
// 		case '>':
// 			goto end_in;
// 		default:
// 			break;
// 		}
// 	}
// end_in:
// 
// 	nret = strlen(fmt);	// ���ز�������
// 	if(lua_pcall(L, narg, nret, 0) != 0)
// 	{
// 		// ����lua����ʧ��
// 		mprint("[0x%08X][Error]Call lua function (%s) failed, msg=%s", pSession->m_SessionID, func, lua_tostring(L, -1));
// 		return 1;
// 	}
// 
// 	nindex = -nret;
// 
// 	// ȡ��lua��������ֵ
// 	while(*fmt)
// 	{
// 		switch(*fmt)
// 		{
// 		case 'd':
// 			if (!lua_isnumber(L, nindex))
// 			{
// 				luaL_error(L, "wrong result type");
// 			}
// 			*va_arg(ap, double*) = lua_tonumber(L, nindex);
// 			break;
// 		case 'i':
// 			if (!lua_isnumber(L, nindex))
// 			{
// 				luaL_error(L, "wrong result type");
// 			}
// 			*va_arg(ap, int*) = lua_tointeger(L, nindex);
// 			break;
// 		case 's':
// 			if (!lua_isstring(L, nindex))
// 			{
// 				luaL_error(L, "wrong result type");
// 			}
// 			*va_arg(ap, const char**) = lua_tostring(L, nindex);
// 			break;
// 		default:
// 			luaL_error(L, "invalid option (%c)", *(fmt-1));
// 			break;
// 		}
// 		++nindex;
// 	}
// 
// 	lua_pop(L, nret);
// 
// 	va_end(ap);

	return 0;
}

TLuaSession::TLuaSession(USService* owner, int SessionID) : m_pOwner(owner)
{	
	m_L = luaL_newstate();
	m_SessionID = SessionID;
}

TLuaSession::~TLuaSession()
{
	// 	if(m_TimerOutID)
	// 		ClearTimer(m_TimerOutID);
	// 		
	if (m_L)
	{
		lua_close(m_L);
	}
}

TLuaSession& TLuaSession::operator=(TLuaSession& obj)
{
	return *this;
}



void TLuaSession::ClearTimer(int TimerID)
{
	//m_pOwner->ClearTimer(TimerID);
}

int TLuaSession::SetTimer(int owner, int interval, int attr)
{
	//return m_pOwner->SetTimer(owner, interval, attr);
	return 0;
}

// int TLuaSession::PostMessage(RTCS_CTpdu pdu)
// {
// 	m_DataAccessFlowID = GetTickCount();
// 	pdu.tranID = m_DataAccessFlowID;
// 
// 	return m_pOwner->PostMessage(pdu);
// }


///////////////////////////////////////////////////////////////////////////////
// ԭ�Ӷ���
///////////////////////////////////////////////////////////////////////////////
// Name: PrintLog
void TLuaSession::PrintLog(int nLevel, char *fmt, ...)
{
	// 	char szOut[LEN_LONG+1]= "";
	// 	va_list args;
	// 	va_start(args, fmt);
	// 	
	// 	vsprintf(szOut, fmt, args);
	// 
	// 	if(nLevel == 0)
	// 		PrintConsole(m_pOwner->m_ModuleName, szOut);
	// 
	// 	if(m_pOwner->logfile)
	// 		m_pOwner->logfile->Trace(nLevel, szOut);
	// 
	// 	va_end(args);
}

/* 
����: ���ؽű�
����:
file: �ű��ļ�·�����涨ʹ�����·��
����:
0: �ɹ�
1: ʧ��
*/
int	TLuaSession::LoadScript(const char* file)
{
	//mprint("[0x%08X]BEGIN:", m_SessionID);

	if (!file || 0==strlen(file))
	{
		mprint("Load script failed, script file path is null\n");
		return 1;
	}

	// ����lua�ļ�
	int ret = luaL_loadfile(m_L, file);
	if (ret) 
	{                                  
		mprint("Load script failed, bad file %s, errno=%d, msg=%s\n", file, ret, lua_tostring(m_L, -1));
		return 1;
	}

	//��������Lua��
	luaL_openlibs(m_L);

	// ��ʼ�� field
	InitGlobalParams();

	// ���� package.path
	SetLuaPath(m_L, "./lua/");
	//SetLuaPath(m_L, "d:/Program Files (x86)/Lua/5.1/lua/");

	// ע������c����
	RegisterAllFunction2Lua();

	// ͬ��������luaջ��
	SyncFieldC2Lua();

	return 0;
}

/* 
����: ��ʼ������ȫ�ֱ���
����:
����:
*/
int	TLuaSession::InitGlobalParams()
{
	// 	// ��ջL�ﴴ��field table
	// 	lua_newtable(m_L);   
	// 	// ��ʼ������field��ֵ	
	// 	for (int i=1; i<=1000; i++)
	// 	{
	// 		lua_pushinteger(m_L, i);
	// 		lua_pushstring(m_L, m_Field[i-1]);
	// 		lua_rawset(m_L, -3);
	// 	} 
	// 	lua_setglobal(m_L, LC_FIELD);
	// 	
	// 	// ��ջL�ﴴ��field num
	// 	lua_pushinteger(m_L, m_nFieldNum);
	// 	lua_setglobal(m_L, LC_NUM_FIELD);
	// 	
	// ��ջL�ﴴ�� utm status
	lua_pushinteger(m_L, m_lastValidPDUStatus);
	lua_setglobal(m_L, LC_MSG_STATUS);
 
	// ��ջL�ﴴ�� result
	lua_pushstring(m_L, "");
	lua_setglobal(m_L, LC_RESULT);

	// ��ջL�ﴴ�� session id
	lua_pushinteger(m_L, m_SessionID);
	lua_setglobal(m_L, lC_SESSION_ID);

	return 0;
}

/* 
����: ִ�нű�
����:
����:
0: �ɹ�
1: ʧ��
*/
int	TLuaSession::ExecScript()
{
	int ret = lua_pcall(m_L, 0, LUA_MULTRET, 0);
	if (ret)
	{
		mprint("[0x%08X]Execute script failed, errno=%d, msg=%s\n", m_SessionID, ret, lua_tostring(m_L, -1));
		return 1;
	}

	return 0;
}

/* 
����: ��ѯƥ��Ĺ���
����:
rule: ��ѯ����
retRule: ��ѯ���Ĺ���
����:
0: ��ȡ��ƥ�����
1: δ��ȡ��ƥ�����
*/
int TLuaSession::GetMatchedRule(const Rule& rule, Rule& retRule)
{	
	int ret =  m_ruleManager.GetMatchedRule(rule, retRule);

	if (ret == 0)
	{
		//mprint("[0x%08X]%s:", m_SessionID, retRule.m_state.GetFuncName().c_str());		
	}
	else
	{
		mprint("[0x%08X][Error]Recv unexpected event (%s, %d)", m_SessionID, rule.m_sEvent.c_str(), rule.m_status);
	}
	return ret;
}

/* 
����: ���ýű�����
˵��: �Ե�ǰģʽ��c�е���lua�ĺ���ʱ������Ҫ����field֮��Ĳ���
����:
func: ������
����:
0: �ɹ�
1: ʧ��
*/
int	TLuaSession::CallLuaFunc(const char* func)
{		
	if (!func || 0==strlen(func))
	{
		mprint("[0x%08X]Call function failed, function name is null\n", m_SessionID);
		return 1;
	}

	// ��ն�ʱ��
	StopTimer();

	// �����ת����
	ClearRule();

	// ͬ��field��lua
	//m_lastValidPDUStatus = pdu.status;
	int traceFlag = 1;  // �����־��־  
	// 	if (pdu.messageID == EvtRTCS_IClientBase_Timer)		// Ϊʱ����Ϣʱ���򲻴�ӡfield��ֵ
	// 	{
	// 	  traceFlag = 0;
	// 	}
	SyncFieldC2Lua(traceFlag);

	// ����lua����
	lua_getglobal(m_L, func);
	if(0 == lua_isfunction(m_L, -1))
	{
		lua_pop(m_L, 1);
		// lua����������
		mprint("[0x%08X][Error](%s) is not a lua function", m_SessionID, func);
		return 1;
	}

	luaL_checkstack(m_L, 1, "too many arguments");	

	if(lua_pcall(m_L, 0, 0, 0) != 0)	// All arguments and the function value are popped from the stack when the function is called. 
	{
		// ����lua����ʧ��
		mprint("[0x%08X][Error]Call lua function (%s) failed, msg=%s", m_SessionID, func, lua_tostring(m_L, -1));

		// �ű��쳣���������Ự
		mprint("[Error]�ű��쳣���˳��Զ����³���");
		exit(0);
		return 1;
	}

	// ��Ϊ���õ�lua����û�з���ֵ�����Բ��ô�ջ��pop
	//lua_pop(m_L, nret);

	return 0;
}

/* 
����: ע�����к�����lua��
˵��: ��Ŀǰ������ÿע��һ�����͵ĳ�Ա����ʱ����Ҫ��Ҫ��дһ��ע�ắ������������ʹ��ģ�庯��
����:
����:
*/
void TLuaSession::RegisterAllFunction2Lua()
{
	// ע�� SendMsg
	lua_directregistry_memfunc_iii_i(m_L, "C_SendMsg", m_SessionID, &TLuaSession::_LuaDataAccess);	
	// 	// ע�� ResponseMessage
	// 	lua_directregistry_memfunc_iii_i(m_L, "C_ResponseMessage", m_SessionID, &TLuaSession::_LuaResponseMessage);
	// ע�� StartTimer
	lua_directregistry_memfunc_iii_i(m_L, "C_StartTimer", m_SessionID, &TLuaSession::_LuaStartTimer);
	// ע�� StopTimer
	lua_directregistry_memfunc_iii_i(m_L, "C_StopTimer", m_SessionID, &TLuaSession::_LuaStopTimer);
	// ע�� Exit
	lua_directregistry_memfunc_iii_i(m_L, "C_Exit", m_SessionID, &TLuaSession::_LuaExit);
	// ע�� DriverReturn
	lua_directregistry_memfunc_iii_i(m_L, "C_DriverReturn", m_SessionID, &TLuaSession::_LuaDriverReturn);
	// ע�� GetSessionID
	lua_directregistry_memfunc_iii_i(m_L, "C_GetSessionID", m_SessionID, &TLuaSession::_LuaGetSessionID);
	//
	lua_directregistry_memfunc_iii_i(m_L, "C_SetStatus", m_SessionID, &TLuaSession::_LuaSetStatus);
	//
	lua_directregistry_memfunc_iii_i(m_L, "C_PostSelfMsg", m_SessionID, &TLuaSession::_LuaPostSelfMsg);
	//
	lua_directregistry_memfunc_iii_i(m_L, "C_PostMsg", m_SessionID, &TLuaSession::_LuaPostMsg);
	//
	lua_directregistry_memfunc_iii_i(m_L, "C_SetResAppRef", m_SessionID, &TLuaSession::_LuaSetResAppRef);
	//
	lua_directregistry_memfunc_iii_i(m_L, "C_GetInput", m_SessionID, &TLuaSession::_LuaGetInput);
	// �Զ�����ר��
	lua_directregistry_memfunc_iii_i(m_L, "C_IsHttpDownloadOK", m_SessionID, &TLuaSession::_LuaIsHttpDownloadOK);

	// ע�� SetNextState
	lua_directregistry_memfunc_sis_i(m_L, "C_SetNextState", m_SessionID, &TLuaSession::_LuaSetNextState);
	// ע�� PrintLog
	lua_directregistry_memfunc_sis_i(m_L, "C_PrintLog", m_SessionID, &TLuaSession::_LuaPrintLog);
	// ע�� PutInt
	lua_directregistry_memfunc_sis_i(m_L, "C_PutInt", m_SessionID, &TLuaSession::_LuaPutInt);

	// ע�� GetString
	lua_directregistry_memfunc_sss_i(m_L, "C_GetString", m_SessionID, &TLuaSession::_LuaGetString);
	// ע�� GetInt
	lua_directregistry_memfunc_sss_i(m_L, "C_GetInt", m_SessionID, &TLuaSession::_LuaGetInt);
	// ע�� GetNumber
	lua_directregistry_memfunc_sss_i(m_L, "C_GetNumber", m_SessionID, &TLuaSession::_LuaGetNumber);
	// ע�� CreateUtm
	lua_directregistry_memfunc_sss_i(m_L, "C_CreateUtm", m_SessionID, &TLuaSession::_LuaCreateUtm);
	// ע�� SetMsgID
	lua_directregistry_memfunc_sss_i(m_L, "C_SetMsgID", m_SessionID, &TLuaSession::_LuaSetMsgID);
	// ע�� PutString
	lua_directregistry_memfunc_sss_i(m_L, "C_PutString", m_SessionID, &TLuaSession::_LuaPutString);
	// ע�� PutStringArray
	lua_directregistry_memfunc_sss_i(m_L, "C_PutStringArray", m_SessionID, &TLuaSession::_LuaPutStringArray);
	// ע�� ReadConfig
	lua_directregistry_memfunc_sss_i(m_L, "C_ReadConfig", m_SessionID, &TLuaSession::_LuaReadConfig)
	//
	lua_directregistry_memfunc_sss_i(m_L, "C_HttpDownload", m_SessionID, &TLuaSession::_LuaHttpDownload);
	//
	lua_directregistry_memfunc_sss_i(m_L, "C_CopyFile", m_SessionID, &TLuaSession::_LuaCopyFile);
}

/* 
����: ��lua���õ�DataAccess����
����:
templateID: ģ���
dst: ����Դ
timeout: ��ʱʱ�䣬��λ����
����:
*/
int TLuaSession::_LuaDataAccess(int templateID, int dataSource, int timeout)
{		
	SyncFieldLua2C();
	mprint("[0x%08X] DataAccess(%d, %d, %d)", m_SessionID, templateID, dataSource, timeout);	// ��Ϊ�õ���field�����Ը�����Ҫ����SyncFieldLua2C֮��

	sprintf(m_sLastAction, "DataAccess()");

	// 	RTCS_CTpdu pdu;
	// 	char szFuncName[LEN_NORMAL+1]= "";
	// 	string _field[MAX_FIELD_NUM];
	// 	stringArray pField = _field;
	// 	int nTimerDelay = timeout;
	// 
	// 
	// 	strcpy(szFuncName, "TLuaSession::DataAccess()");
	// 
	// 	pdu.messageID = CmdRTCS_IDBP_DataAccess;
	// 	pdu.sender = m_pOwner->GetGOR();
	// 	pdu.senderObj = m_SessionID;
	// 
	// 	pdu.receiver =  m_pOwner->m_pDataSourceUINTMgr->GetDataSource(dataSource);
	// 	pdu.status = 0;
	// 	
	// 	if(pdu.receiver == INVALID_GOR)
	// 	{
	// 		mprint("[0x%08X][Error][%s]: DataAccess error, receiver is INVALID_GOR\n", m_SessionID, szFuncName);
	// 		PostAlarmMessage(ALARM_MDO_AppServer_ReqFailed, 0, 0, 0, "��������(DataAccess)ʧ�ܣ����ز�����!");
	// 
	// 		RTCS_CTpdu respPdu;		
	// 		respPdu.messageID   = RespRTCS_IDBP_DataAccess;
	// 		respPdu.sender      = m_pOwner->GetGOR();
	// 		respPdu.receiverObj = m_SessionID;
	// 		respPdu.senderObj = m_SessionID;
	// 		respPdu.receiver    = m_pOwner->GetGOR();
	// 		respPdu.status      = 1001;		
	// 		respPdu.PutString(RTCS_IDBP_SessionID, m_transSessionID);
	// 
	// 		PostMessage(respPdu);
	// 		
	// 		return RTCS_Error_FAIL;
	// 	}
	// 	
	// 	for(unsigned int i = 0; i < m_nFieldNum; i++)
	// 	{
	// 		_field[i]= m_Field[i];
	// 	}
	// 
	// 	pdu.PutUInt(RTCS_IDBP_DataAccessID, templateID);
	// 	pdu.PutUInt(RTCS_IDBP_FieldNum, m_nFieldNum);
	// 	pdu.PutStringArray(RTCS_IDBP_ParamData, pField, m_nFieldNum);
	// 
	// 	pdu.PutString(RTCS_IDBP_SPID, (string)m_SpID);
	// 	pdu.PutString(RTCS_IDBP_ServiceID, (string)m_ServiceID);
	// 	pdu.PutString(RTCS_IDBP_OperateID, (string)m_OperateID);
	// 	pdu.PutString(RTCS_IDBP_CallingNumber, (string)m_CallingNum);
	// 	pdu.PutString(RTCS_IDBP_CalledNumber, (string)m_CalledNum);
	// 
	// 	pdu.PutString(RTCS_IDBP_SessionID, m_transSessionID);
	// 	
	// 
	// 	
	// 	int nRet = PostMessage(pdu);
	// 	if(nRet != 0)
	// 	{
	// 		mprint("[0x%08X][Error]���̶���%s: PostMessage����(Receiver=0x%08X)������DataAccess������Դ�Ƿ���ȷ, ret=%d��\n", 
	// 			m_SessionID, m_sLastAction, pdu.receiver, nRet);
	// 		PostAlarmMessage(ALARM_MDO_AppServer_ReqFailed, nRet, 0, 0, "��������(%s)ʧ�ܣ�����ʧ��!", m_sLastAction);
	// 
	// 		pdu.messageID   = RespRTCS_IDBP_DataAccess;
	// 		pdu.sender      = m_pOwner->GetGOR();
	// 		pdu.receiverObj = m_SessionID;
	// 		pdu.senderObj = m_SessionID;
	// 		pdu.receiver    = m_pOwner->GetGOR();
	// 		pdu.status      = 1001;
	// 		
	// 		PostMessage(pdu);
	// 		
	// 		return RTCS_Error_FAIL;
	// 	}
	// 
	// 	if(m_TimerID)
	// 		ClearTimer(m_TimerID);
	// 	m_TimerID = SetTimer(m_SessionID, nTimerDelay);

	return 0;
}

/* 
����: ��lua���õ�ResponseMessage����
˵��: ԭ����_LuaDataAccessһ�£��Ա�ʹ��ͬ���Ĵ�����ע��ú�����lua����2��3������δʹ��
����:
status: �ظ�ֵ
����:
*/
int TLuaSession::_LuaResponseMessage(int status, int empty1, int empty2)
{	
	sprintf(m_sLastAction, "ResponseMessage()");

	SyncFieldLua2C();

	// 	int nResult = status;
	// 	RTCS_CTpdu pdu;
	// 	string FieldArray[MAX_FIELD_NUM]; //�ֶθ�ֵ
	// 	stringArray pFieldArray = FieldArray;
	// 	char szFuncName[LEN_NORMAL+1]= "";
	// 
	// 	strcpy(szFuncName, "TLuaSession::ResponseMessage()");
	// 
	// 	pdu.sender = m_AppModuleGOR;
	// 	if(m_nOrgPDUSender != INVALID_GOR)
	// 	{
	// 		pdu.receiver = m_nOrgPDUSender;
	// 		pdu.receiverObj = m_nOrgPDUSenderObj;
	// 	}
	// 	else
	// 	{
	// 		pdu.receiver = m_nOrgSender;
	// 		pdu.receiverObj = m_nOrgSenderObj;
	// 	}
	// 	pdu.tranID = m_nOrgTranID;
	// 	pdu.messageID = m_nOrgMessageID | 0x20000000;
	// 	pdu.status = nResult;
	// 
	// 	for(unsigned int i = 0; i < m_nFieldNum; i++)
	// 	{
	// 		FieldArray[i]= m_Field[i];
	// 	}
	// 
	// 	pdu.PutUInt(RTCS_IDBP_DataAccessID, m_nFlowID); //���̺�
	// 	pdu.PutUInt(RTCS_IDBP_FieldNum, m_nFieldNum);
	// 	pdu.PutStringArray(RTCS_IDBP_ParamData, pFieldArray, m_nFieldNum);
	// 	if(m_nHTTPStatus >= 0)
	// 		pdu.PutUInt(RTCS_ISPServer_HttpStatus, m_nHTTPStatus);
	// 
	// 	pdu.PutString(RTCS_IDBP_SessionID, m_transSessionID);
	// 
	// 	int ret = m_pOwner->PostMessage(pdu);
	// 	if(ret != 0)
	// 	{
	// 		PostAlarmMessage(ALARM_MDO_AppServer_RespFailed, ret, 0, 0, "�ظ�IVR���󣬷���ʧ��!");
	// 		mprint("[0x%08X][Error][%s]: Send message error, Receiver=0x%08X, ret=%d\n", 
	// 			m_SessionID, szFuncName, pdu.receiver, ret);
	// 		
	// 		return 0;
	// 	}
	// 	
	// 	mprint("[0x%08X] ResponseMessage(%d)", m_SessionID, status);

	return 0;
}

/* 
����: ���ö�ʱ��
˵��: ԭ����_LuaDataAccessһ�£��Ա�ʹ��ͬ���Ĵ�����ע��ú�����lua����2��3������δʹ��
����:
val: ʱ������λ����
����:
*/
int TLuaSession::_LuaStartTimer(int val, int empty1, int empty2)
{
	//mprint("[0x%08X] StartTimer(%d)", m_SessionID, val);
	mprint("StartTimer(%d)", val);
	StartTimer(val);
	return 0;
}

/* 
����: �رն�ʱ��
����:
����:
*/
int TLuaSession::_LuaStopTimer(int, int, int)
{
	mprint("[0x%08X] StopTimer()", m_SessionID);
	StopTimer();
	return 0;
}

/* 
����: ����session
˵��: ԭ����_LuaDataAccessһ�£��Ա�ʹ��ͬ���Ĵ�����ע��ú�����lua����2��3������δʹ��
����:
����:
*/
int TLuaSession::_LuaExit(int, int, int)
{	
	mprint("[0x%08X] END()", m_SessionID);	

	StopTimer();

	UserTransferMessagePtr utm(new UserTransferMessage);
	utm->setMessageId(_EvtKillSession);
	(*utm)[_TagSessionID] = (u32)m_SessionID;
	m_pOwner->postSelfMessage(utm);
	return 0;
}

/* 
����: ����EvtReturn�¼�
����:
receiverGor: ������gor
receiverObj: 
status: ״̬
����:
*/
int TLuaSession::_LuaDriverReturn(int receiverGor, int receiverObj, int status)
{
	char szFuncName[200+1]= "";	
	strcpy(szFuncName, "TLuaSession::DriverReturn()");

	SyncFieldLua2C();
	mprint("[0x%08X] DriverReturn(0x%08X, 0x%08X, %d)", m_SessionID, receiverGor, receiverObj, status);
	// 	if(receiverGor == INVALID_GOR)
	// 	{
	// 		mprint("[0x%08X][Error][%s]: DriverReturn error, receiver is INVALID_GOR\n", m_SessionID, szFuncName);
	// 		PostAlarmMessage(ALARM_MDO_AppServer_ReqFailed, 0, 0, 0, "��������(DriverReturn)ʧ��!");
	// 		return RTCS_Error_FAIL;
	// 	}
	// 
	// 	RTCS_CTpdu pdu;
	// 	string _field[MAX_FIELD_NUM];
	// 	stringArray pField = _field;
	// 
	// 	pdu.messageID = EvtRTCS_DRIVER_Return;
	// 	pdu.sender = m_pOwner->GetGOR();
	// 	pdu.senderObj = m_SessionID;	
	// 	pdu.receiver =  receiverGor;
	// 	pdu.receiverObj = receiverObj;
	// 	pdu.status = status;	
	// 	
	// 	for(unsigned int i = 0; i < m_nFieldNum; i++)
	// 	{
	// 		_field[i]= m_Field[i];
	// 	}
	// 	
	// 	pdu.PutUInt(RTCS_IDBP_FieldNum, m_nFieldNum);
	// 	pdu.PutStringArray(RTCS_IDBP_ParamData, pField, m_nFieldNum);	
	// 	int nRet = PostMessage(pdu);
	// 	if(nRet != 0)
	// 	{
	// 		mprint("[0x%08X][Error]���̶���%s: DriverReturn����(Receiver=0x%08X)�����������Ƿ���ȷ, ret=%d��\n", 
	// 			m_SessionID, m_sLastAction, pdu.receiver, nRet);
	// 		PostAlarmMessage(ALARM_MDO_AppServer_ReqFailed, nRet, 0, 0, "��������(%s)ʧ�ܣ�����ʧ��!", m_sLastAction);		
	// 		return RTCS_Error_FAIL;
	// 	}

	return 0;
}

/* 
����: ��ȡSession ID
����:
����:
0: �ɹ�, �������ŵ�_Result��
1: ʧ��
*/
int TLuaSession::_LuaGetSessionID(int , int , int)
{
	return m_SessionID;
}

/* 
����: ���÷���utm��״̬
����:
	status ---- ״̬
����:
*/
int TLuaSession::_LuaSetStatus(int status, int, int)
{
	mprint("[0x%08X] SetStatus(%d)", m_SessionID, status);
	m_postUtm.setReturn(status);
	return 0;
}

/* 
����: ���Լ�������Ϣ
����:
����:
*/
int TLuaSession::_LuaPostSelfMsg(int, int, int)
{
	mprint("[0x%08X] PostSelfMsg()", m_SessionID);
	if (m_postUtm.getMessageId() == INVALID_MESSID)
	{
		mprint("[0x%08X][Error]: CreateUtm action not done", m_SessionID);
		return 1;
	}

	m_pOwner->postSelfMessage(m_postUtm);

	return 0;
}

/* 
����: ��ָ��appport��Ӧ��ģ�鷢����Ϣ
����:
	appport --
����:
*/
int TLuaSession::_LuaPostMsg(int appport, int, int)
{
	mprint("[0x%08X] PostMsg(%d)", m_SessionID, appport);
	if (m_postUtm.getMessageId() == INVALID_MESSID)
	{
		mprint("[0x%08X][Error]: CreateUtm action not done", m_SessionID);
		return 1;
	}

	// ��ȡĿ��ģ��ķ���ID
	ServiceIdentifier res;
	if (0 != m_pOwner->GetSIDByApport(appport, res))
	{
		mprint("[0x%08X] Post msg failed, dest program not online", m_SessionID);
		return 1;
	}

	res.m_appref = (m_selfSpecResAppref == -1) ? m_lastValidUtm.getReq().m_appref : m_selfSpecResAppref;		// ��session id���ڴ˴�
	m_postUtm.setRes(res);

	m_pOwner->postMessage(res, m_postUtm);
	return 0;
}

/* 
����: ���÷�����Ϣ�е� res.m_appref ֵ�������ֵ�����ã�������ʹ�ø�ֵ������ʹ��m_lastValidUtm.getReq().m_appref
����:
����:
*/
int TLuaSession::_LuaSetResAppRef(int appref, int, int)
{
	mprint("[0x%08X] SetResAppref(%d)", m_SessionID, appref);
	m_selfSpecResAppref = appref;
	return 0;
}

/* 
����: ��ȡ�û����룬�������ȫ�ֱ��� _Result ��
����:
����:
*/
int TLuaSession::_LuaGetInput(int, int, int)
{
	char sTmp[1024];
	scanf_s("%s", sTmp);

	lua_pushstring(m_L, sTmp);
	lua_setglobal(m_L, LC_RESULT);
	return 0;
}

// ��ʱ�ṹ��
// typedef struct TDHttpParam{
// 	string url;
// 	string savePath;
// 	string saveFileName;
// 
// 	TDHttpParam(string v1, string v2, string v3) { url=v1; savePath=v2; saveFileName=v3;}
// } TDHttpParam;

// http�����̺߳���
unsigned int __stdcall HttpDownloadThreadFun(void *param)
{
	TLuaSession *pSession = (TLuaSession*)param;
	if (!pSession)
	{
		return 1;
	}

	DownloadCTL obj;
	obj.Init();
	obj.SetDownloadStatus(TRUE);		// release�汾�±������ø�ֵ������Ĭ��Ϊ0ֵ
	//printf("-- begin: %s %s\n", pSession->m_httpUrl.c_str(), pSession->m_saveFullPath.c_str());
	obj.WriteToLocalFile((char*)pSession->m_httpUrl.c_str(), (char*)pSession->m_saveFullPath.c_str());
// 	char *url = "http://demo.zksr.cn/upload/priceTag/bbb.wmv";
// 	char *savefilename = "ddd.wmv";
//	obj.WriteToLocalFile(url,savefilename);
//	printf("end\n");
	return 0;
}

/* 
����: ��ʼhttp����
����:
����:
*/
 int TLuaSession::_LuaHttpDownload(string url,string savePath, string saveFileName)
 {
	 m_httpUrl = url;
	 m_saveFullPath = savePath;

	 unsigned threadID;
	 m_handleDownHttp = (HANDLE)_beginthreadex(0, 0, HttpDownloadThreadFun, (void*)this, 0, &threadID);
	 //printf("handle: 0x%08x threadid: %d\n", m_handleDownHttp, threadID);
	 return 0;
 }

 /* 
 ����: �����ļ�
 ����:
 ����:
 */
 int TLuaSession::_LuaCopyFile(string src,string dst, string)
 {
	 mprint("CopyFile(%s, %s)", src.c_str(), dst.c_str());
	 return FileSystem::copyfile(src.c_str(), dst.c_str());
 }

 /* 
 ����: �ж�http�����Ƿ����
 ����:
 ����:
	1: �ѳɹ�
	0: δ�ɹ�
 */
int TLuaSession::_LuaIsHttpDownloadOK(int, int, int)
{
	DWORD ret = WaitForSingleObject(m_handleDownHttp, 1000);
	if (ret == WAIT_OBJECT_0)
	{
		CloseHandle(m_handleDownHttp);
	}

	return (ret == WAIT_OBJECT_0) ? 1 : 0;
}

// �����̺߳���
// unsigned int __stdcall HeartBeatThreadFun(void *param)
// {
// 	TLuaSession *pSession = (TLuaSession*)param;
// 	if (!pSession)
// 	{
// 		return 1;
// 	}
// 
// 	pSession->HeartBeatThread();
// 	return 0;
// }

/* 
����: �����Զ����µ������߳�
����:
����:
*/
// int TLuaSession::_LuaStartHeartBeat(int, int, int)
// {
// 	m_bHeartBeat = 1;
// 
// 	unsigned threadID;
// 	m_handleHeartBeat = (HANDLE)_beginthreadex(0, 0, HeartBeatThreadFun, this, 0, &threadID);
// // 	char *url = "http://demo.zksr.cn/gateway/pos_gateway!openAPI.action";
// // 	char *body = "method=heartBeat&key=201504291510112ABFBE";
// // 	char retMsg[1024];
// // 	CallLuaFunction(0, "HttpPost", "ss>s", url, body, retMsg);
// // 	printf("heart beat result: %s\n", retMsg);
// 	return 0;
// }

// �����߳�
// int TLuaSession::HeartBeatThread()
// {
// 	// ��ȡurl
// 	char url[1024];
// 	GetPrivateProfileString("port", "httpurl", "", url, sizeof(url), MAIN_CLIENT_CONFIG);
// 	printf("heart beat url: %s\n", url);
// 	
// 	// ��ȡ�ͻ���id
// 	char key[1024];
// 	GetPrivateProfileString("port", "key", "", key, sizeof(key), MAIN_CLIENT_CONFIG);
// 	printf("client id: %s\n", key);
// 
// 	char body[1024];
// 	sprintf_s(body, 1024, "method=heartbeat&key=%s", key);
// 	unsigned int nHeartBeatInterval = GetPrivateProfileInt("system", "heartbeatinterval", 1, SYSTEM_CFG_FILE);
// 	nHeartBeatInterval = nHeartBeatInterval<1 ? 1 : nHeartBeatInterval;
// 	printf("HeartBeatInterval=%d\n", nHeartBeatInterval);
// 	while(m_bHeartBeat != 0)
// 	{
// 		mprint("heart beat...");
// 		
// 		char retMsg[1024];
// 		CallLuaFunction(0, "HttpPost", "ss>s", url, body, retMsg);
// 		
// 		mprint("heart beat result: %s\n", retMsg);
// 		
// 		Sleep(nHeartBeatInterval * 1000);
// 	}
// 	mprint("heart beat thread over");
// 	return 0;
// }

/* 
����: �����Զ����µ������߳�
����:
����:
*/
// int TLuaSession::_LuaStopHeartBeat(int, int, int)
// {
// 	m_bHeartBeat = 0;
// 
// 	unsigned threadID;
// 	WaitForSingleObject(m_handleHeartBeat, INFINITE);
// 	// Destroy the thread object.
// 	CloseHandle(m_handleHeartBeat);
// 	return 0;
// }

/* 
����: ������һ����ת״̬
����:
status: �ظ�ֵ
����:
*/
int TLuaSession::_LuaSetNextState(string msgID, int status, string nextFuncName)
{
	//mprint("[0x%08X] %s[%d]", m_SessionID, msgID.c_str(), status);
	//mprint("[0x%08X]   -> %s", m_SessionID, nextFuncName.c_str());
	
	// ����rule��������rule��������
	// �����ִ��͵�msgID�ҵ���ֵ TODO:   ��ҪsID��iID��ӳ��
	int iID = m_pOwner->GetIIDBySID(msgID);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error]: SetNextState failed, unknown msgID=%s\n", m_SessionID, msgID.c_str());
		return 1;
	}
	
	Rule *rule = new Rule(iID, msgID.c_str(), (int)status, nextFuncName);
	if (0 != m_ruleManager.AddRule(rule))	// �ɹ�
	{
		mprint("[0x%08X][Error]: SetNextState failed", m_SessionID);
		return 1;
	}

	return 0;
}

/* 
����: �����־
����:
s: ��־����
����:
*/
int TLuaSession::_LuaPrintLog(string s, int, string)
{
	//mprint("[0x%08X]%s", m_SessionID, s.c_str());	
	mprint("%s", s.c_str());	
	return 0;
}

/* 
����: ����utm������ֵ
����:
	key ---- 
	value --
����:
*/
int TLuaSession::_LuaPutInt(string key, int value, string)
{
	mprint("[0x%08X] PutInt(%s, %d)", m_SessionID, key.c_str(), value);

	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}

	m_postUtm[iID] = (u32)value;
	return 0;
}

/* 
����: ��ȡutm����key��Ӧ���ִ�ֵ
����:
	key ---- ��ֵ
����:
	��ȡ��ֵ�ŵ�ȫ�ֱ��� _result �У�ֵ�����Ϳ���Ϊ string �� strarray��strarrayת��Ϊ�������ַ��ָ����ִ�����"s1&|&|s2&|&|s3"
*/
int TLuaSession::_LuaGetString(string key, string, string)
{  
	mprint("[0x%08X] GetString(%s)", m_SessionID, key.c_str());

	if (m_lastValidUtm.getMessageId() == INVALID_MESSID)
	{
		mprint("[0x%8x][Error] No valid utm", m_SessionID);
		return 1;
	}

	// �ҵ�key������ֵ
	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}
	
	string value = "";

	value = m_lastValidUtm[iID];
	
	// �����ֵ��L��ȫ�ֱ���
	lua_pushstring(m_L, value.c_str());
	lua_setglobal(m_L, LC_RESULT);

	return 0;
}

/* 
����: ��ȡutm����key��Ӧ������ֵ
����:
	key ---- ��ֵ
����:
	��ȡ��ֵ�ŵ�ȫ�ֱ��� _result ��
*/
int TLuaSession::_LuaGetInt(string key, string, string)
{  
	mprint("[0x%08X] GetInt(%s)", m_SessionID, key.c_str());

	if (m_lastValidUtm.getMessageId() == INVALID_MESSID)
	{
		mprint("[0x%8x][Error] No valid utm", m_SessionID);
		return 1;
	}

	// �ҵ�key������ֵ
	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}
	
	u32 value = 0;

	value = m_lastValidUtm[iID];
	
	// �����ֵ��L��ȫ�ֱ���
	lua_pushinteger(m_L, value);
	lua_setglobal(m_L, LC_RESULT);

	return 0;
}

/* 
����: ��ȡutm����key��Ӧ��doubleֵ (TODO: ȡ����number���Ȳ���)
����:
	key ---- ��ֵ
����:
	��ȡ��ֵ�ŵ�ȫ�ֱ��� _result ��
*/
int TLuaSession::_LuaGetNumber(string key, string, string)
{  
	mprint("[0x%08X] GetNumber(%s)", m_SessionID, key.c_str());

	if (m_lastValidUtm.getMessageId() == INVALID_MESSID)
	{
		mprint("[0x%08x][Error] No valid utm", m_SessionID);
		return 1;
	}

	// �ҵ�key������ֵ
	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}
	
	float value = 0;

	value = m_lastValidUtm[iID];
	
	// �����ֵ��L��ȫ�ֱ���
	lua_pushnumber(m_L, value);
	lua_setglobal(m_L, LC_RESULT);

	return 0;
}

/* 
����: �������ڷ��͵�utm��
����:
����:
*/
int TLuaSession::_LuaCreateUtm(string, string, string)
{
	mprint("[0x%08X] CreateUtm()", m_SessionID);

	ClearPostUtm();

	return 0;
}

/* 
����: ����utm������ϢID
����:
	msgID ----
����:
*/
int TLuaSession::_LuaSetMsgID(string msgID, string, string)
{
	mprint("[0x%08X] SetMsgID(%s)", m_SessionID, msgID.c_str());

	// ��ȡ������ϢID
	int iID = m_pOwner->GetIIDBySID(msgID);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error]: SetMsgID failed, unknown msgID=%s\n", m_SessionID, msgID.c_str());
		return 1;
	}

	m_postUtm.setMessageId(iID);

	return 0;
}

/* 
����: ����utm�����ִ�ֵ
����:
	key ----
	value --
����:
*/
int TLuaSession::_LuaPutString(string key, string value, string)
{
	mprint("[0x%08X] PutString(%s, \"%s\")", m_SessionID, key.c_str(), value.c_str());

	// ��ȡ������ϢID
	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}

	m_postUtm[iID] = value;

	return 0;
}

/* 
����: ����utm�����ִ�����ֵ
����:
	key ----
	value -- �ִ���
����:
*/
int TLuaSession::_LuaPutStringArray(string key, string value, string)
{
	mprint("[0x%08X] PutStringArray(%s, \"%s\")", m_SessionID, key.c_str(), value.c_str());

	// ��ȡ������ϢID
	int iID = m_pOwner->GetIIDBySID(key);
	if (iID == INVALID_MESSID)
	{
		mprint("[0x%08X][Error] Unknown key: %s", m_SessionID, key.c_str());
		return 1;
	}

	// ������ÿ���ִ�	
	strarray values = split(value, ";");
	m_postUtm[iID] = values;

	return 0;
}

/************************************************************************/
/* 
����: ��ȡ�����ļ�
����:
����:
0Ϊ�ɹ���1Ϊʧ��
*/
/************************************************************************/
int TLuaSession::_LuaReadConfig(string section,string key,string filename)
{
	char value[1024] = "";
	GetPrivateProfileString(section.c_str(), key.c_str(), "", value, 1024, filename.c_str());
	lua_pushstring(m_L, value);
	lua_setglobal(m_L, LC_RESULT);
	return 0;
}

/* 
����: ͬ��C��������ݵ�lua��ջ
˵��: Ŀǰͬ�����ݰ���: _status
����:
traceFlag: �Ƿ������־��־  0:����� ����:���
����:
*/
int TLuaSession::SyncFieldC2Lua(int traceFlag)
{	
	// 	����luaջ��ȫ�ֱ���LC_MSG_STATUS��ֵ
	lua_pushinteger(m_L, m_lastValidUtm.getReturn());
	lua_setglobal(m_L, LC_MSG_STATUS);

	return 0;
}

/* 
����: ͬ��lua��field��C
˵��: Ŀǰͬ�����ݰ���: _FieldNum _Field
����:
����:
*/
int TLuaSession::SyncFieldLua2C()
{
	return 0;
}

/* 
����: �����ת����
˵��: ��ÿ��c����lua����֮ǰ������ø÷���
����:
����:
*/
int TLuaSession::ClearRule()
{
	m_ruleManager.ClearRule();
	return 0;
}

/* 
����: ���� package.path
����:
����:
*/
int TLuaSession::SetLuaPath(lua_State* L, const char* path)
{
	lua_getglobal( L, "package" );
	lua_getfield( L, -1, "path" ); 								// get field "path" from table at top of stack (-1)
	std::string cur_path = lua_tostring( L, -1 );					// grab path string from top of stack
	cur_path.append( ";" ); 										// do your path magic here
	cur_path.append( path );
	cur_path.append( "?.lua;" );
	cur_path.append( path );
	cur_path.append( "?/init.lua;" );
	lua_pop( L, 1 ); 												// get rid of the string on the stack we just pushed on line 5
	lua_pushstring( L, cur_path.c_str() ); 						// push the new one
	lua_setfield( L, -2, "path" ); 								// set the field "path" in table at -2 with value at top of stack
	lua_pop( L, 1 ); 												// get rid of package table from top of stack
	return 0;
}

/************************************************************************/
/* 
���ܣ����ö�ʱ��
������val ---- ʱ��������λ��
���أ�
*/
/************************************************************************/
void TLuaSession::StartTimer(int tval)
{
	UserTransferMessagePtr utm(new UserTransferMessage);
	utm->setMessageId(_EvtTimeOut);
	(*utm)[_TagSessionID] = (u32)m_SessionID;
	
	m_timerID = m_pOwner->setTimer(tval * 1000, utm);
}

/************************************************************************/
/* 
���ܣ��رն�ʱ��
����
���أ�
*/
/************************************************************************/
void TLuaSession::StopTimer()
{
	if (m_timerID > 0)
	{
		m_pOwner->killTimer(m_timerID);
		m_timerID = 0;
	}
}

/************************************************************************/
/* 
���ܣ�����������Чutm��
������
���أ�
*/
/************************************************************************/
void TLuaSession::ClearLastValidUtm()
{
	// ��Ϊutmû����ղ������滻����Ϊ�� utm����ϢID��Ϊ IVALID
	UserTransferMessage tUtm;
	m_lastValidUtm = tUtm;	// �ø÷��ܽ�Utm���������
	m_lastValidUtm.setMessageId(INVALID_MESSID);
}

/************************************************************************/
/* 
���ܣ����post utm��
������
���أ�
*/
/************************************************************************/
void TLuaSession::ClearPostUtm()
{
	// ��Ϊutmû����ղ������滻����Ϊ�� utm����ϢID��Ϊ IVALID
	UserTransferMessage tUtm;
	m_postUtm = tUtm;	// �ø÷��ܽ�Utm���������
	m_postUtm.setMessageId(INVALID_MESSID);
 	ServiceIdentifier req = m_pOwner->myIdentifier(m_SessionID);
// 	req.m_appref = m_SessionID;
 	m_postUtm.setReq(req);		// ��session id����req��appref�ֶ���
 	//m_postUtm.setRes(req);

	m_selfSpecResAppref = -1;
}

/************************************************************************/
/* 
���ܣ��ַ����ָ��
������
	str------Դ��
	pattern--�ָ��ִ�
���أ�
	��ָ���ָ����ָ�����ִ�����
*/
/************************************************************************/
std::vector<std::string> TLuaSession::split(std::string str,std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str+=pattern;//��չ�ַ����Է������
	int size=str.size();

	for(int i=0; i<size; i++)
	{
		pos=str.find(pattern,i);
		if(pos<size)
		{
			std::string s=str.substr(i,pos-i);
			result.push_back(s);
			i=pos+pattern.size()-1;
		}
	}
	return result;
}

/************************************************************************/
/* 
���֣�CallLuaFunction
���ܣ�ִ��lua�ļ���ָ������
������
	lp ---- ��չ����
	func -- ������
	vfmt -- �������������ʽ "[ids]*[>[ids]*]"
	... --- ʵ��
���أ�
	0 ----- �ɹ�
	���� -- ʧ��
*/
/************************************************************************/
int TLuaSession::CallLuaFunction(void *lp, const char* func, char *vfmt, ...)
{	
	int retcode = 0;

	if (!m_L || !func || 0==strlen(func))
	{
		return 1;
	}
	
	int narg;
	int nret;
	int nindex;

	lua_getglobal(m_L, func);
	if(0 == lua_isfunction(m_L, -1))
	{
		lua_pop(m_L, 1);
		// lua����������
		mprint("[Error](%s) is not a lua function\n", func);
		return 1;
	}
	
	char *fmt = vfmt;
	va_list ap;
	va_start(ap, vfmt);
	for(narg=0; *fmt; ++narg, ++fmt){
		luaL_checkstack(m_L, 1, "too many arguments");
		
		switch(*fmt)
		{
		case 'd':
			lua_pushnumber(m_L, va_arg(ap, double));
			break;
		case 'i':
			lua_pushinteger(m_L, va_arg(ap, int));
			break;
		case 's':
			lua_pushstring(m_L, va_arg(ap, char*));
			break;
		case '>':
			++fmt;
			goto end_in;
		default:
			break;
		}
	}
end_in:
	
	nret = strlen(fmt);	// ���ز�������
	if(lua_pcall(m_L, narg, nret, 0) != 0)
	{
		// ����lua����ʧ��
		mprint("[Error]Call lua function (%s) failed, msg=%s\n", func, lua_tostring(m_L, -1));
		return 1;
	}
	
	nindex = -nret;
	
	// ȡ��lua��������ֵ
	while(*fmt)
	{
		switch(*fmt)
		{
		case 'd':
			if (!lua_isnumber(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type\n");
				retcode = 1;
				goto lua_end;
			}
			*va_arg(ap, double*) = lua_tonumber(m_L, nindex);
			break;
		case 'i':
			if (!lua_isnumber(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type\n");
				retcode = 1;
				goto lua_end;
			}
			*va_arg(ap, int*) = lua_tointeger(m_L, nindex);			
			break;
		case 's':
			if (!lua_isstring(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type\n");
				retcode = 1;
				goto lua_end;
			}
			strcpy((char*)va_arg(ap, const char**), lua_tostring(m_L, nindex));
			break;
		default:
			mprint("[ERROR] Invalid option(%c)\n", *fmt);
			retcode = 1;
			goto lua_end;
		}
		++fmt;
		++nindex;
	}

lua_end:
	lua_pop(m_L, nret);
	
	va_end(ap);
	
	return retcode;
}