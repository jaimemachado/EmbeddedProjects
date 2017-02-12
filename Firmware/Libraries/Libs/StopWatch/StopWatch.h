#pragma once
#if !defined WIN32
	#if defined(ARDUINO) && ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif
#endif
#include "StopWatchConfig.h"

typedef unsigned long(*GetTimeStopWatchFunc)();

class StopWatch
{
	unsigned long currentTime_;
	GetTimeStopWatchFunc getTimeFunc_;
	unsigned long timeToCount_;
	bool timerStarted, timeExpired;

	unsigned long getTime_();

public:
	StopWatch();

	StopWatch(GetTimeStopWatchFunc getTimeFunc);

	void startTimer(unsigned long time);
	bool expired();
	void reset();
	unsigned long missingTime();
};

