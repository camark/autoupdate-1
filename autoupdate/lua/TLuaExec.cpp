#include "TLuaExec.h"
#include "time.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern void mprint(const char* fmt, ...);

extern "C"
{ 
#include "lua.h"  
#include "lualib.h"
#include "lauxlib.h" 
}

struct lua_State* TLuaExec::m_L = luaL_newstate();

/************************************************************************/
/* 
���֣�ExecLua
���ܣ����ز�ִ��lua�ļ�
������
	luafile ---- lua�ļ��� .lua��.out��ʽ����
*/
/************************************************************************/
int TLuaExec::LoadAndExecLua(const char* luafile)
{
	// ����lua�ļ�
	int ret = luaL_loadfile(m_L, luafile);
	if (ret) 
	{                                  
		mprint("[ERROR] Load script failed, bad file %s, errno=%d, msg=%s", luafile, ret, lua_tostring(m_L, -1));
		return 1;
	}

	//��������Lua��
	luaL_openlibs(m_L);

	ret = lua_pcall(m_L, 0, -1, 0);
	if (ret)
	{
		mprint("Execute script failed, errno=%d, msg=%s", ret, lua_tostring(m_L, -1));
		return 2;
	}

	return 0;
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
int TLuaExec::CallLuaFunction(void *lp, const char* func, char *vfmt, ...)
{	
	//
	//printf("sizeof(lua_Integer) = %d\n", sizeof(lua_Integer));

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
		mprint("[Error](%s) is not a lua function", func);
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
		mprint("[Error]Call lua function (%s) failed, msg=%s", func, lua_tostring(m_L, -1));
		return 1;
	}
	
	nindex = -nret;
	
	// ȡ��lua��������ֵ
	while(*fmt)
	{
		//int t;
		switch(*fmt)
		{
		case 'd':
			if (!lua_isnumber(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type");
				retcode = 1;
				goto lua_end;
			}
			*va_arg(ap, double*) = lua_tonumber(m_L, nindex);
			break;
		case 'i':
			if (!lua_isnumber(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type");
				retcode = 1;
				goto lua_end;
			}
			//*va_arg(ap, int*) = lua_tonumber(m_L, nindex);			
			*va_arg(ap, int*) = (long long)lua_tonumber(m_L, nindex);	// ��long longת�����lua�������ݴ���0x80000000������			
			//t = lua_tonumber(m_L, nindex);
			//t = (long long)lua_tonumber(m_L, nindex);
			break;
		case 's':
			if (!lua_isstring(m_L, nindex))
			{
				mprint("[ERROR] Wrong result type");
				retcode = 1;
				goto lua_end;
			}
			strcpy((char*)va_arg(ap, const char**), lua_tostring(m_L, nindex));
			break;
		default:
			mprint("[ERROR] Invalid option(%c)", *fmt);
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