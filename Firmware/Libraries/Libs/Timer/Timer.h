#pragma once
#if !defined WIN32
	#if defined(ARDUINO) && ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif
#endif
#include "TimerConfig.h"

typedef unsigned long(*GetTimeTimerFunc)();

class Timer
{
	unsigned long currentTime_;
	GetTimeTimerFunc getTimeFunc_;

	unsigned long getTime_();

public:
	Timer();

	Timer(GetTimeTimerFunc getTimeFunc);

	unsigned long getTime();

	unsigned long getTimeAndReset();

	void reset();
};

