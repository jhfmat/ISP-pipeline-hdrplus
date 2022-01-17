#include "TimeStatistic.h"
#include "SystemLog.h"
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#define TIME_FORMAT  "%-56s %6d ms"
std::stack<long long> TimerStatistics::m_nTime_stack;
bool TimerStatistics::m_bEnable = true;
bool TimerStatistics::m_bForbid = false;
void TimerStatistics::forbid(bool flag)
{
	m_bForbid = flag;
}
void TimerStatistics::enable(bool bEnable)
{
	if (m_bForbid) bEnable = false;
	m_bEnable = bEnable;
}
void TimerStatistics::start(void)
{
	if (m_bEnable)
	{
		m_nTime_stack.push(GetTickCountEx());
	}
}
void TimerStatistics::stop(const char* str)
{
	long long time = 0;
	if (m_bEnable)
	{
		if (!m_nTime_stack.empty()) time = GetTickCountEx() - m_nTime_stack.top();
#ifdef _WIN32
		LARGE_INTEGER litmp;
		QueryPerformanceFrequency(&litmp);
		time = 1000 * (time) / litmp.QuadPart;
#endif
#ifndef _WIN32
        LOGI(TIME_FORMAT, str, (int)time);
#endif	
        printf(TIME_FORMAT, str, (int)time);
		printf("\n");
		if (!m_nTime_stack.empty()) m_nTime_stack.pop();
	}
}
void TimerStatistics::reset(void)
{
	TimerStatistics::enable();
	m_nTime_stack.empty();
}
long long TimerStatistics::GetTickCountEx()
{
#ifdef WIN32
	LARGE_INTEGER litmp;
	QueryPerformanceCounter(&litmp);
	return litmp.QuadPart;
#else
	struct timeval res;
	gettimeofday(&res, NULL);
	return(res.tv_sec * 1000 + res.tv_usec / 1e3);
#endif
}