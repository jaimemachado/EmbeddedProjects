// 
// 
// 

#include "TimeLib.h"


unsigned long TimeLib::getCurrentTime()
{
	return millis();
}

unsigned long TimeLib::milliSecPassed(unsigned long time)
{
	unsigned long currentTime = getCurrentTime();

	unsigned long maxTime = 0;
	maxTime -= 1;

	if (currentTime > time)
	{
		return currentTime - time;
	}
	else
	{
		return ((maxTime - time + 1) + currentTime);
	}

}

unsigned long TimeLib::secPassed(unsigned long time)
{
	unsigned long millisecPassed = milliSecPassed(time);

	return millisecPassed / 1000;
}

unsigned long TimeLib::minPassed(unsigned long time)
{
	unsigned long millisecPassed = milliSecPassed(time);

	return millisecPassed / 60000;
}

unsigned long TimeLib::hoursPassed(unsigned long time)
{
	unsigned long millisecPassed = milliSecPassed(time);

	return millisecPassed / 3600000;
}
