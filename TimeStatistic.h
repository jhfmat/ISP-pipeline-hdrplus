#ifndef __TIME_STATISTIC_H_
#define __TIME_STATISTIC_H_
#include <stack>
#include "SystemLog.h"
class TimerStatistics
{
public:
	static void start(void);
	static void stop(const char* str = NULL);
	static void reset(void);
	static void enable(bool bEnable = true);
	static long long GetTickCountEx(void);
	static void forbid(bool flag);
private:
	static std::stack<long long> m_nTime_stack;
	static bool m_bEnable;
	static bool m_bForbid;
};

#endif