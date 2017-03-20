// TimeLib.h

#ifndef _TIMELIB_h
#define _TIMELIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class TimeLib
{
public:
	static unsigned long getCurrentTime();
	static unsigned long milliSecPassed(unsigned long time);
	static unsigned long secPassed(unsigned long time);
	static unsigned long minPassed(unsigned long time);
	static unsigned long hoursPassed(unsigned long time);
};


#endif

