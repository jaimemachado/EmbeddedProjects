// TimeScheduler.h
//#include <MySensors.h>

#ifndef _TIMESCHEDULER_h
#define _TIMESCHEDULER_h

#define DEBUG_TIMESCHEDULER Serial


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <TimeLib.h>

#define UPDATE_TIME_FREQ  1 * 1000 * 60 //60 * 1000 * 60
typedef  void(*UpdateFuncPtr)();
class TimeScheduler
{
	bool timeReceived;
	time_t lastUpdate;
	time_t lastRequestUpdate;
	UpdateFuncPtr funcPtr_;
	bool shouldRequestTimeUpdate();
public:
	TimeScheduler();
	void init(UpdateFuncPtr funcptr);
	void setTime_(unsigned long time);
	void handleTimeUpdates();
	bool timeInited();
	void printTime();


};




#endif

