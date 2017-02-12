// TimeScheduler.h
//#include <MySensors.h>

#ifndef _TIMESCHEDULER_h
#define _TIMESCHEDULER_h

#define DEBUG_TIMESCHEDULER Serial


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#endif


#define UPDATE_TIME_FREQ  1 * 1000 * 60 //60 * 1000 * 60
#pragma once

#include <ctime>
#if !defined WIN32
	#if defined(ARDUINO) && ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif
#endif

typedef  void(*UpdateFuncPtr)();
class TimeScheduler
{

public:
	TimeScheduler();
};




#endif

