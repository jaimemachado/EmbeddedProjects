


#include <gtest/gtest.h>
#include "StopWatch/StopWatch.h"

unsigned long currentTimeStopWatch;

unsigned long GetTimeStopWatch()
{
	return currentTimeStopWatch;
}


class StopWatchTest : public ::testing::Test {
protected:


	virtual void SetUp()
	{
		currentTimeStopWatch = 0;
		stopWatch = new StopWatch(GetTimeStopWatch);

	}
	void TearDown()
	{
		currentTimeStopWatch = 0;
	}

	// virtual void TearDown() {}
	StopWatch* stopWatch;
};


TEST_F(StopWatchTest, Check_Internal_Timer_StartTimer)
{
	unsigned long timeToCount = 10;
	stopWatch->startTimer(timeToCount);

	currentTimeStopWatch = 2;

	ASSERT_EQ(timeToCount - 2, stopWatch->missingTime());

	currentTimeStopWatch = 8;
	ASSERT_EQ(timeToCount - 8, stopWatch->missingTime());

	currentTimeStopWatch = 12;
	ASSERT_EQ(0, stopWatch->missingTime());

}


TEST_F(StopWatchTest, Check_Internal_Timer_StartTimer_WithOverFlow)
{
	unsigned long timeToCount = 30;

	currentTimeStopWatch = 0xFFFFFFF0;

	stopWatch->startTimer(timeToCount);

	currentTimeStopWatch = 0xFFFFFFF5;

	ASSERT_EQ(timeToCount - (0xFFFFFFF5 - 0xFFFFFFF0), stopWatch->missingTime());

	currentTimeStopWatch = 8;
	ASSERT_EQ(timeToCount -(0xFFFFFFFF - 0xFFFFFFF0 + 8), stopWatch->missingTime());

	currentTimeStopWatch = 30;
	ASSERT_EQ(0, stopWatch->missingTime());
}

TEST_F(StopWatchTest, Check_Internal_Timer_StartTimer_Expired)
{
	unsigned long timeToCount = 30;

	stopWatch->startTimer(timeToCount);

	currentTimeStopWatch = 35;

	ASSERT_TRUE(stopWatch->expired());
}

TEST_F(StopWatchTest, Check_Internal_Timer_StartTimer_Reset)
{
	unsigned long timeToCount = 30;

	stopWatch->startTimer(timeToCount);

	currentTimeStopWatch = 10;

	ASSERT_EQ(timeToCount - 10, stopWatch->missingTime());

	stopWatch->reset();

	ASSERT_EQ(0, stopWatch->missingTime());
}