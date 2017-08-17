/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 *
 * DESCRIPTION
 * This sketch provides an example how to implement a distance sensor using HC-SR04
 * Use this sensor to measure KWH and Watt of your house meeter
 * You need to set the correct pulsefactor of your meeter (blinks per KWH).
 * The sensor starts by fetching current KWH value from gateway.
 * Reports both KWH and Watt back to gateway.
 *
 * Unfortunately millis() won't increment when the Arduino is in
 * sleepmode. So we cannot make this sensor sleep if we also want
 * to calculate/report watt-number.
 * http://www.mysensors.org/build/pulse_power
 */

//#define JMN_DEBUG_INFO
//#define JMN_DEBUG_VERBOSE
#define JMN_DEBUG_CALIB

// Enable debug prints
#define NUMBER_OF_SENSORS 3
#define NUMBER_READS_TO_UPDATE_KWH 10
#define CURRENT_REF_VOLTAGE 230.0
#define REFRESH_TIME 30
#define POWER_METER_PRECISION 0.001

//TIME
#define SYNC_CLOCK_PRERIOD_SEC 3600

//#define MY_DEBUG


// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#include <MySensors.h>

#include "DataStoraManager.h"

#include "TimeLibJMN.h"
#include <TimeLib.h>
#include "EnergyMonitorJMN.h"


 






bool allinited = false;
DataStoraManager dataManage(NUMBER_OF_SENSORS);


//TIME
bool timeInited = false;
unsigned long requestedTimeControl = 0;
time_t lastTimeUpdate;

unsigned long printTimeControl = 0;

int32_t testTime = 0;



EnergyMonitorJMN sensors[NUMBER_OF_SENSORS];


void updateTime()
{
	//Requesting Time
#ifdef JMN_DEBUG_VERBOSE
	Serial.println("Time requested!");
#endif
	requestTime();
}


void setup()
{

}

void receiveTime(unsigned long ts)
{
#ifdef JMN_DEBUG_VERBOSE
	Serial.println("Time Received!");
#endif
	setTime(ts);
	timeInited = true;
	lastTimeUpdate = now();
}

void receive(const MyMessage &message)
{
#ifdef JMN_DEBUG_INFO
	Serial.print("Received message Sensor:");
	Serial.print(message.sensor);
	Serial.print(" Command:");
	Serial.print(message.getCommand());
	Serial.print(" IsAck:");
	Serial.print(message.isAck());
	Serial.print(" Type:");
	Serial.println(message.type);
#endif
	for (int x = 0; x < NUMBER_OF_SENSORS; x++)
	{
		sensors[x].processMessage(message);
	}

}

void presentation()
{
	for(int x = 0; x < NUMBER_OF_SENSORS ; x++)
	{
		sensors[x].init(x);
	}
	sendSketchInfo("Energy Meter", "1.0");

	for (int x = 0; x < NUMBER_OF_SENSORS; x++)
	{
		sensors[x].presentSensor();
	}
}




void loop()
{
	
	if(timeInited == false)
	{
		if(millis() - requestedTimeControl > 5000)
		{
			updateTime();
			requestedTimeControl = millis();
		}
	}else
	{
		if (millis() - printTimeControl > 5000)
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("Time: ");
			Serial.print(hour());
			Serial.print(":");
			Serial.print(minute());
			Serial.print(":");
			Serial.println(second());
#endif
			printTimeControl = millis();
		}

		if((now() - lastTimeUpdate > SYNC_CLOCK_PRERIOD_SEC) && (millis() - requestedTimeControl > 5000) )
		{
			updateTime();
			requestedTimeControl = millis();
		}

		for (int x = 0; x < NUMBER_OF_SENSORS; x++)
		{
			sensors[x].run();
		}
	}
}


