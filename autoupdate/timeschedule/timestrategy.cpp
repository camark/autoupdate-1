#include "timestrategy.h"

//////////////////////////////////////////////////////////////////////////
DayStrategy::DayStrategy(unsigned int hour, unsigned int minute, unsigned int second)
{
	m_hour = hour;
	m_minute = minute;
	m_second = second;
}

/************************************************************************/
/* 
����: �ж��ڸò����£�ʱ���Ƿ���
����:
	curTime: ��ǰʱ��
����:
	1: ����
	0: δ����
*/
/************************************************************************/
int DayStrategy::CheckOnTime(struct tm curTime)
{
	if (m_hour != curTime.tm_hour)
	{
		return 0;
	}

	if (m_minute != curTime.tm_min)
	{
		return 0;
	}

	if (m_second != curTime.tm_sec)
	{
		return 0;
	}

	return 1;
}

void DayStrategy::SnapShot()
{
	printf("day strategy: each day %02d:%02d:%02d\n", m_hour, m_minute, m_second);
}

//////////////////////////////////////////////////////////////////////////
MonthStrategy::MonthStrategy(unsigned int day, unsigned int hour, unsigned int minute, unsigned int second)
{
	m_day = day;
	m_hour = hour;
	m_minute = minute;
	m_second = second;
}

/************************************************************************/
/* 
����: �ж��ڸò����£�ʱ���Ƿ���
����:
	curTime: ��ǰʱ��
����:
	1: ����
	0: δ����
*/
/************************************************************************/
int MonthStrategy::CheckOnTime(struct tm curTime)
{
	if (m_day != curTime.tm_mday)
	{
		return 0;
	}

	if (m_hour != curTime.tm_hour)
	{
		return 0;
	}

	if (m_minute != curTime.tm_min)
	{
		return 0;
	}

	if (m_second != curTime.tm_sec)
	{
		return 0;
	}

	return 1;
}

void MonthStrategy::SnapShot()
{
	printf("monthly strategy: each month %02d %02d:%02d:%02d\n", m_day, m_hour, m_minute, m_second);
}