#include "timeschedule.h"
#include "timeobserver.h"
#include "timestrategy.h"
#include <windows.h>
#include <process.h>

// �̺߳���
void ThreadFun(void* p)
{
	TimeSchedule *ts = (TimeSchedule*)p;
	while (0 == ts->Runing())
	{
		Sleep(1000);
		ts->CheckOnTime();
	}
}

ObserverInfo::ObserverInfo(TimeObserver* observer, TimeStrategy* strategy) 
: m_observer(observer), m_strategy(strategy)
{

}

TimeSchedule::TimeSchedule()
{
	m_bStop = 0;
}

TimeSchedule::~TimeSchedule()
{
	Stop();

	for (list<ObserverInfo*>::iterator it = m_observerList.begin(); it != m_observerList.end(); ++it)
	{
		ObserverInfo *p = *it;
		delete p;
		p = NULL;
	}

	m_observerList.clear();
}

/************************************************************************/
/* 
����: ��ʼ��ʱ
����:
����:
*/
/************************************************************************/
void TimeSchedule::Start()
{
	_beginthread(ThreadFun, 0, this);
}

/************************************************************************/
/* 
����: ����������ע��ʱ��۲���
����:
	observer: �۲���
	strategy: ʱ�����
����:
*/
/************************************************************************/
void TimeSchedule::Register(TimeObserver* observer, TimeStrategy* strategy)
{
	ObserverInfo *pInfo = new ObserverInfo(observer, strategy);

	m_observerList.push_back(pInfo);
}

/************************************************************************/
/* 
����: �ڱ���������ע��ʱ��۲���
����:
	observer: �۲���
	strategy: ʱ�����
����:
*/
/************************************************************************/
void TimeSchedule::UnRegister(TimeObserver* observer, TimeStrategy* strategy)
{
	ObserverInfo *pInfo = new ObserverInfo(observer, strategy);

	for (list<ObserverInfo*>::iterator it = m_observerList.begin(); it != m_observerList.end();)
	{
		if (observer == (*it)->m_observer && strategy == (*it)->m_strategy)
		{
			ObserverInfo *p = *it;
			delete p;
			p = NULL;

			m_observerList.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

/************************************************************************/
/* 
����: ��鱾������������ʱ��۲����Ƿ���
����:
����:
*/
/************************************************************************/
void TimeSchedule::CheckOnTime()
{
	time_t rawTime;
	time(&rawTime);

	struct tm timeInfo;
	localtime_s(&timeInfo, &rawTime);
	
	// �������й۲��ߣ����е����ж�
	for (list<ObserverInfo*>::iterator it = m_observerList.begin(); it != m_observerList.end(); ++ it)
	{
		if ((*it)->m_strategy->CheckOnTime(timeInfo))
		{
			(*it)->m_observer->Notify();
		}
	}
}

/************************************************************************/
/* 
����: ���������Ϣ
����:
����:
*/
/************************************************************************/
void TimeSchedule::SnapShot()
{
	printf("Snapshot:\n");
	int count = 1;
	for (list<ObserverInfo*>::iterator it = m_observerList.begin(); it != m_observerList.end(); ++ it)
	{
		printf("observer info %d:\n", count++);
		//printf("\tobserver: 0x%08x\n", (void*)(*it)->m_observer);
		printf("\t");
		(*it)->m_strategy->SnapShot();
	}
}