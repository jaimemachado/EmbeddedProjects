


#include <gtest/gtest.h>
#include "Time/Time.h"
#include <windows.h>
//#include <unistd.h>

unsigned long currentTime_Time;

unsigned long GetTime_Time()
{
	return currentTime_Time;
}


class TimeTest : public ::testing::Test {
protected:


	virtual void SetUp()
	{
		currentTime_Time = 0;

	}
	void TearDown()
	{
		currentTime_Time = 0;
	}

	// virtual void TearDown() {}

};


TEST_F(TimeTest, Check_Internal_Timer_StartTimer)
{
	setTime(22, 00, 00, 1, 1, 2017);
	ASSERT_EQ(22, hour());
	Sleep(1200);
	ASSERT_EQ(01, second());

}

