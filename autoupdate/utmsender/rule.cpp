#include "rule.h"

//#include "global.h"

Rule::Rule(int iEvent, string sEvent, int status, State state) : m_iEvent(iEvent), m_sEvent(sEvent), m_status(status), m_state(state)
{

}

int Rule::operator==(const Rule& r)
{
	if (r.m_iEvent != this->m_iEvent)
	{
		return 0;
	}
	
	if (this->m_status == -1)		// statusΪ-1��ʾƥ���κ�״ֵ̬
	{
		return 1;
	}
	
	return this->m_status==r.m_status;
}

Rule& Rule::operator=(const Rule& r)
{	
	this->m_iEvent = r.m_iEvent;
	this->m_sEvent = r.m_sEvent;
	this->m_status = r.m_status;
	this->m_state = r.m_state;
	return *this;
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
int RuleManager::GetMatchedRule(const Rule& rule, Rule& retRule)
{
	// __TW__DEBUG
	//SnapShot();
	
	vector<Rule*>::iterator it = m_rules.begin();
	for (; it!=m_rules.end(); ++it)
	{
		if (**it == rule)
		{
			retRule = **it;
			return 0;
		}
	}

	return 1;
}

/* 
����: ��ӹ���
����:
	rule: ����
����:
	0: �ɹ�
	1: ʧ��
*/
int RuleManager::AddRule(Rule* rule)
{
	if (!rule)
	{
		return 1;
	}
	
	vector<Rule*>::iterator it = m_rules.begin();
	for (; it!=m_rules.end(); ++it)
	{
		if (**it == *rule)
		{
			return 1;
		}
	}

	m_rules.push_back(rule);	
	return 0;
}

/* 
����: ��չ���
����:
����:
	0: �ɹ�
	1: ʧ��
*/	
int RuleManager::ClearRule()
{
	for (vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); ++it)
  {
    Rule* tmp = *it;
    delete tmp;
  }

  m_rules.clear();
  
	return 0;
}

void RuleManager::SnapShot()
{
	//PrintConsole("RuleManager", "Rule manager snapshot:");
	vector<Rule*>::iterator it = m_rules.begin();
	for (; it!=m_rules.end(); ++it)
	{
		//PrintConsole("RuleManager", "rule: %s %d %s", (*it)->m_sEvent.c_str(), (*it)->m_status, (*it)->m_state.GetFuncName().c_str());	
	}
}
