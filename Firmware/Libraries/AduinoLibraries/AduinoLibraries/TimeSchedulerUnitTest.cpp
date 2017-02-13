#include "Mocks/EEPROMWin.h"
#include <gtest/gtest.h>
#include "TimeScheduler/TimeScheduler.h"
#include <windows.h>
//#include <unistd.h>

unsigned long currentTime_TimeScheduler;

unsigned long GetTime_TimeScheduler()
{
	return currentTime_TimeScheduler;
}


class TimeSchedulerTest : public ::testing::Test, public TimeScheduler {
protected:


	virtual void SetUp()
	{
		currentTime_TimeScheduler = 0;

	}
	void TearDown()
	{
		currentTime_TimeScheduler = 0;
		initDataOnNVM();
	}

	// virtual void TearDown() {}

};


TEST_F(TimeSchedulerTest, Check_Writing_Config_To_EEPROM)
{
	SchedulerStructure* config = getConfigById(0);
	config->weekDays = 2;
	config->onTimeHour = 10;
	config->onTimeMinute = 15;
	config->offTimeHour = 11;
	config->offTimeMinute = 20;
	writeConfigId(0);

	SchedulerStructure* configRead = (SchedulerStructure*)(&EEPROM.eeprom_mem[SCHEDULER_EEPROM_DATA_START]);

	ASSERT_EQ(config->weekDays, configRead->weekDays);
	ASSERT_EQ(config->onTimeHour, configRead->onTimeHour);
	ASSERT_EQ(config->onTimeMinute, configRead->onTimeMinute);
	ASSERT_EQ(config->offTimeHour, configRead->offTimeHour);
	ASSERT_EQ(config->offTimeMinute, configRead->offTimeMinute);

}
