#include "StopWatch.h"

#define UNSIGNED_LONG_SIZE 0xFFFFFFFF

unsigned long StopWatch::getTime_()
{
	unsigned long tempTime = getTimeFunc_();

	if (tempTime > currentTime_)
	{
		return tempTime - currentTime_;
	}
	else
	{
		return (UNSIGNED_LONG_SIZE - currentTime_) + tempTime;
	}
}

StopWatch::StopWatch()
{
	getTimeFunc_ = STOPWATCH_GET_TIME_FUNC;
	currentTime_ = getTimeFunc_();
	timeExpired = false;
	timerStarted = false;
}

StopWatch::StopWatch(GetTimeStopWatchFunc getTimeFunc)
{
	getTimeFunc_ = getTimeFunc;
	currentTime_ = getTimeFunc();
	timeExpired = false;
	timerStarted = false;
}

void StopWatch::startTimer(unsigned long time)
{
	timeToCount_ = time;
	currentTime_ = getTimeFunc_();
	timerStarted = true;
}

bool StopWatch::expired()
{
	missingTime();
	return timeExpired;
}

void StopWatch::reset()
{
	timeExpired = false;
	timerStarted = false;
	currentTime_ = getTimeFunc_();
}

unsigned long StopWatch::missingTime()
{
	if(!timerStarted || timeExpired)
	{
		return 0;
	}

	unsigned time = getTime_();
	if(time > timeToCount_)
	{
		timeExpired = true;
		return 0;
	}
	return timeToCount_ - time;
}
