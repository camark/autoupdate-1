/************************************************************************/
/* 
���ܣ�session��������
*/
/************************************************************************/

#pragma once

#include <map>
using namespace std;

class TLuaSession;

class TSessionManager
{
public:
	//RTCS_Lock   m_Lock;

private:
	int m_curSessionID;
	int m_nSessionMaxNum;
	map<int, TLuaSession*> m_sessionTable;

public:
	inline void SetSessionMaxNum(int nSessionMaxNum)
	{
		m_nSessionMaxNum = nSessionMaxNum;
	};

public:
	TSessionManager();
	~TSessionManager();

	void ClearAll();
	int  AddSessionObject(TLuaSession* Obj);
	TLuaSession* GetSessionObject(int SessionID);
	void  DeleteSessionObject(int SessionID);

	//����ΨһSession�ŵĺ���
	int GenerateSessionID();

	void Snapshot(FILE* fp);

	int GetCount() { return (int)m_sessionTable.size(); }
};