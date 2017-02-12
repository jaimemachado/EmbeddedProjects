


#include <gtest/gtest.h>
#include "Timer/Timer.h"

unsigned long currentTimeTimer;

unsigned long GetTimeTimer()
{
	return currentTimeTimer;
}


class TimerTest : public ::testing::Test {
protected:


	virtual void SetUp() 
	{
		currentTimeTimer = 0;
		time = new Timer(GetTimeTimer);
		
	}
	void TearDown()
	{
		currentTimeTimer = 0;
	}

	// virtual void TearDown() {}
	Timer* time;
};


TEST_F(TimerTest, Check_Internal_Timer_Get_Time)
{
	unsigned long inittialValue = 10;
	unsigned long nextValue = 11;

	currentTimeTimer = inittialValue;

	ASSERT_EQ(inittialValue, time->getTime());

	currentTimeTimer = nextValue;

	ASSERT_EQ(nextValue, time->getTime());
	
}

TEST_F(TimerTest, Check_Internal_Timer_Get_Time_With_Overflow)
{
	unsigned long inittialValue = 0xFFFFFFF0;
	unsigned long nextValue = 10;

	currentTimeTimer = inittialValue;

	ASSERT_EQ(inittialValue, time->getTime());

	time->reset();

	currentTimeTimer = nextValue;

	ASSERT_EQ(0xFFFFFFFF - inittialValue + nextValue, time->getTime());

}

TEST_F(TimerTest, Check_Internal_Timer_Get_And_Reset)
{
	unsigned long inittialValue = 10;
	unsigned long nextValue = 11;

	currentTimeTimer = inittialValue;

	ASSERT_EQ(inittialValue, time->getTimeAndReset());

	currentTimeTimer = nextValue;

	ASSERT_EQ(nextValue - inittialValue, time->getTime());
}

TEST_F(TimerTest, Check_Internal_Timer_Reset)
{
	unsigned long inittialValue = 10;
	unsigned long nextValue = 11;

	currentTimeTimer = inittialValue;

	ASSERT_EQ(inittialValue, time->getTime());

	time->reset();

	currentTimeTimer = nextValue;

	ASSERT_EQ(nextValue - inittialValue, time->getTime());
}