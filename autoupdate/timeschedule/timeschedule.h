#pragma once

#include <list>
using namespace std;

class TimeObserver;
class TimeStrategy;

class ObserverInfo
{
public:
	ObserverInfo(TimeObserver* observer, TimeStrategy* strategy);

private:
	TimeObserver* m_observer;
	TimeStrategy* m_strategy;

	friend class TimeSchedule;
};

// ʱ�������
class TimeSchedule
{
public:
	TimeSchedule();
	~TimeSchedule();

	void Start();
	void Stop() { m_bStop = 1; }
	int Runing() { return m_bStop; }

	void Register(TimeObserver* observer, TimeStrategy* strategy);
	void UnRegister(TimeObserver* observer, TimeStrategy* strategy);

	void CheckOnTime();

	void SnapShot();

private:
	list<ObserverInfo*> m_observerList;

	int m_bStop;											// 1�ر� 0����
};