/************************************************************************/
/* TLuaExec.h
����: lua��ز�����
*/
/************************************************************************/

#pragma once

struct lua_State;

class TLuaExec
{
public:
	static int LoadAndExecLua(const char* luafile);
	static int CallLuaFunction(void *lp, const char* func, char *fmt, ...);

private:
	static struct lua_State	*m_L; 
};