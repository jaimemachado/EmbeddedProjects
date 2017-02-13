// 
// 
// 

#include "TimeScheduler.h"

void TimeScheduler::readDataFromNVM()
{
	int eepromInitValue = SCHEDULER_EEPROM_DATA_START;

	for (int x = 0; x < NUMBER_OF_SCHEDULER_CONFIGS; x++)
	{
		config[x].weekDays = EEPROM.read(eepromInitValue++);
		config[x].onTimeHour = EEPROM.read(eepromInitValue++);
		config[x].onTimeMinute = EEPROM.read(eepromInitValue++);
		config[x].offTimeHour = EEPROM.read(eepromInitValue++);
		config[x].offTimeMinute = EEPROM.read(eepromInitValue++);
	}
}

void TimeScheduler::initDataOnNVM()
{
	for(int x = SCHEDULER_EEPROM_DATA_START; x < sizeof(SchedulerStructure)*NUMBER_OF_SCHEDULER_CONFIGS ; x++)
	{
		EEPROM.write(x, 0);
	}
	int eepromInitValue = SCHEDULER_EEPROM_START;
	EEPROM.write(eepromInitValue++, 0xA0);
	EEPROM.write(eepromInitValue++, 0x0A);
	readDataFromNVM();
}

void TimeScheduler::writeConfigId(uint8_t id)
{
	int eepromInitValue = SCHEDULER_EEPROM_DATA_START + sizeof(SchedulerStructure)*id;
	EEPROM.write(eepromInitValue++, config[id].weekDays);
	EEPROM.write(eepromInitValue++, config[id].onTimeHour);
	EEPROM.write(eepromInitValue++, config[id].onTimeMinute);
	EEPROM.write(eepromInitValue++, config[id].offTimeHour);
	EEPROM.write(eepromInitValue++, config[id].offTimeMinute);
}

void TimeScheduler::init()
{
	int eepromInitValue = SCHEDULER_EEPROM_START;
	uint8_t byteCheck1 = EEPROM.read(eepromInitValue++);
	uint8_t byteCheck2 = EEPROM.read(eepromInitValue++);
	if(byteCheck1 == 0xA0 && byteCheck2 == 0x0A)
	{
		readDataFromNVM();
	}else
	{
		initDataOnNVM();
	}

}

TimeScheduler::TimeScheduler()
{
	//struct tm *date = gmtime(your_time);
}

bool TimeScheduler::ReadyToUse()
{
	return timeStatus() == timeStatus_t::timeSet;
}

SchedulerStructure* TimeScheduler::getConfigById(uint8_t id)
{
	if (id >= NUMBER_OF_SCHEDULER_CONFIGS)
		return NULL;
	return &config[id];
}

bool TimeScheduler::updateConfigId(uint8_t id)
{
	if (id >= NUMBER_OF_SCHEDULER_CONFIGS)
		return false;
	writeConfigId(id);
	return true;

}
