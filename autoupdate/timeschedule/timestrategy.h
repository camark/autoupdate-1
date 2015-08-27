#pragma once

#include <time.h>
#include <stdio.h>

class TimeStrategy
{
public:
	~TimeStrategy() { printf("~TimeStrategy\n"); }
	virtual int CheckOnTime(struct tm curTime) = 0;

	virtual void SnapShot() = 0;
};

// ÿ��̶�ʱ��
class DayStrategy : public TimeStrategy
{
public:
	DayStrategy(unsigned int hour, unsigned int minute, unsigned int second);

	int CheckOnTime(struct tm curTime);

	void SnapShot();

private:
	unsigned int m_hour;									// Сʱ  ��Чֵ:0-23
	unsigned int m_minute;									// ����  ��Чֵ:0-59
	unsigned int m_second;									// ����  ��Чֵ:0-59
};

// ÿ�¹̶�ʱ��
class MonthStrategy : public TimeStrategy
{
public:
	MonthStrategy(unsigned int day, unsigned int hour, unsigned int minute, unsigned int second);

	int CheckOnTime(struct tm curTime);

	void SnapShot();

private:
	unsigned int m_day;										// ����  ��Чֵ:1-31
	unsigned int m_hour;									// Сʱ  ��Чֵ:0-23
	unsigned int m_minute;									// ����  ��Чֵ:0-59
	unsigned int m_second;									// ����  ��Чֵ:0-59
};