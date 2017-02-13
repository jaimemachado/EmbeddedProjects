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
#include "Time.h"
#include "EEPROM.h"
#else
#include "Time/Time.h"
#include "Mocks/EEPROMWin.h"
#endif


#define SCHEDULER_EEPROM_START 512
#define SCHEDULER_EEPROM_DATA_START 514
#define NUMBER_OF_SCHEDULER_CONFIGS 10
//Size Used SizeOf(SchedulerStructure) * NUMBER_OF_SCHEDULER_CONFIGS + 2(InitCheck)


struct SchedulerStructure
{
	uint8_t weekDays; // B( 0 , Sunday, Saturday, Friday, Thursday, Wednesday, Tuesday, Monday)
	uint8_t onTimeHour;
	uint8_t onTimeMinute;
	uint8_t offTimeHour;
	uint8_t offTimeMinute;
};

typedef  void(*UpdateFuncPtr)();
class TimeScheduler
{
protected:
	SchedulerStructure config[NUMBER_OF_SCHEDULER_CONFIGS];
	void readDataFromNVM();
	void initDataOnNVM();
	void writeConfigId(uint8_t id);
public:
	void init();
	TimeScheduler();
	bool ReadyToUse();
	SchedulerStructure* getConfigById(uint8_t id);
	bool updateConfigId(uint8_t id);


};




#endif

