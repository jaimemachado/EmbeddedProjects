// 
// 
// 

#include "TimeScheduler.h"

TimeScheduler::TimeScheduler()
{
	timeReceived = false;
	lastUpdate = 0;
	lastRequestUpdate = 0;
	setTime(0);
}

void TimeScheduler::init(UpdateFuncPtr ptr)
{
	funcPtr_ = ptr;
}

void TimeScheduler::setTime_(unsigned long time)
{
	printTime();
	
	setTime(time);
	timeReceived = true;
    lastUpdate = millis();;
	
}

void TimeScheduler::handleTimeUpdates()
{
	if(shouldRequestTimeUpdate())
	{
		DEBUG_TIMESCHEDULER.println("Requesting Time Update!");
		funcPtr_();
	}

}

bool TimeScheduler::shouldRequestTimeUpdate()
{
	unsigned long currentTime = millis();

	DEBUG_TIMESCHEDULER.print("Currrent Time:");
	DEBUG_TIMESCHEDULER.print(currentTime);
	DEBUG_TIMESCHEDULER.print(" lastRequestUpdate:");
	DEBUG_TIMESCHEDULER.print(lastRequestUpdate);
	DEBUG_TIMESCHEDULER.print(" lastUpdate:");
	DEBUG_TIMESCHEDULER.print(lastUpdate);
	DEBUG_TIMESCHEDULER.println();

	// If no time has been received yet, request it every 10 second from controller
	// When time has been received, request update every hour
	if (   (!timeReceived && (currentTime - lastRequestUpdate > 10 * 1000))
		|| (timeReceived && (currentTime - lastUpdate > UPDATE_TIME_FREQ))) {
		// Request time from controller.
		DEBUG_TIMESCHEDULER.println("Should Resquest TimeUpdate!");
		lastRequestUpdate = currentTime;
		return true;
	}
	return false;
}

bool TimeScheduler::timeInited()
{
	return timeReceived;
}

void TimeScheduler::printTime()
{
	DEBUG_TIMESCHEDULER.print("Time Set:");
	DEBUG_TIMESCHEDULER.print(hour());
	DEBUG_TIMESCHEDULER.print(":");
	DEBUG_TIMESCHEDULER.print(minute());
	DEBUG_TIMESCHEDULER.print(":");
	DEBUG_TIMESCHEDULER.print(second());
	DEBUG_TIMESCHEDULER.print(" ");
	DEBUG_TIMESCHEDULER.print(day());
	DEBUG_TIMESCHEDULER.print(" ");
	DEBUG_TIMESCHEDULER.print(month());
	DEBUG_TIMESCHEDULER.print(" ");
	DEBUG_TIMESCHEDULER.print(year());
	DEBUG_TIMESCHEDULER.println();
}
