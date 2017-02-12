#pragma once

#ifdef __UNITTEST__
#include <ctime>

inline unsigned long getTimeWin()
{
	return std::time(0);
}
#define STOPWATCH_GET_TIME_FUNC getTimeWin
#else
#define STOPWATCH_GET_TIME_FUNC millis()

#endif