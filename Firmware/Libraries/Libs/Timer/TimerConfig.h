#pragma once

#ifdef __UNITTEST__
#include <ctime>

inline unsigned long getTimeWin()
{
	return std::time(0);
}
#define TIMER_GET_TIME_FUNC getTimeWin
#else
#define TIMER_GET_TIME_FUNC millis()

#endif