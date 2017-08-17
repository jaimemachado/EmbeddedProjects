// TimeLibJMN.h

#ifndef _TIMELIBJMN_h
#define _TIMELIBJMN_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class TimeLibJMN
{
public:
	static unsigned long getCurrentTime();
	static unsigned long milliSecPassed(unsigned long time);
	static unsigned long secPassed(unsigned long time);
	static unsigned long minPassed(unsigned long time);
	static unsigned long hoursPassed(unsigned long time);
};


#endif

