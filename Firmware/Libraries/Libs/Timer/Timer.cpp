

#include "Timer.h"

#define UNSIGNED_LONG_SIZE 0xFFFFFFFF

unsigned long Timer::getTime_()
{
	unsigned long tempTime = getTimeFunc_();

	if(tempTime > currentTime_)
	{
		return tempTime - currentTime_;
	}
	else
	{
		return (UNSIGNED_LONG_SIZE - currentTime_) + tempTime;
	}
}

Timer::Timer()
{
	getTimeFunc_ = TIMER_GET_TIME_FUNC;
	currentTime_ = getTimeFunc_();
}

Timer::Timer(GetTimeTimerFunc getTimeFunc)
{
	getTimeFunc_ = getTimeFunc;
	currentTime_ = getTimeFunc();
}

unsigned long Timer::getTime()
{
	return getTime_();
}

unsigned long Timer::getTimeAndReset()
{
	unsigned long ret = getTime_();
	reset();
	return ret;
}

void Timer::reset()
{
	currentTime_ = getTimeFunc_();
}
