#ifndef __RULE__H__
#define __RULE__H__

#include <string>
#include <vector>
using namespace std;

#include "state.h"

class Rule
{
public:
	Rule(){}
	Rule(int iEvent, string sEvent, int status, State state);

	int operator==(const Rule& r);
	Rule& operator=(const Rule& r);

public:
	int m_iEvent;				// �¼���Ӧ������ֵ
	string m_sEvent;		// �¼���Ӧ���ַ���ֵ
	int m_status;				// �¼�״̬
	State m_state;			// ״̬
};

class RuleManager
{
public:
	RuleManager(){}

	int GetMatchedRule(const Rule& rule, Rule& retRule);
	int AddRule(Rule* rule);
	int ClearRule();
	
	void SnapShot();
	
private:
	vector<Rule*> m_rules;
};
#endif
