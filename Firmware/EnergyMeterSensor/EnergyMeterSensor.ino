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
#define POWER_METER_PRECISION 0.0001

//TIME
#define SYNC_CLOCK_PRERIOD_SEC 3600

//#define MY_DEBUG

#define CALIBRATION_TYPE S_RGB_LIGHT
#define CALIBRATION_STORE_TYPE V_VAR1
#define KWH_STORE_TYPE V_VAR2
#define KWH_STORE_TYPE_DAY V_VAR3
#define KWH_STORE_TYPE_NIGHT V_VAR4

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#include <MySensors.h>

#include "DataStoraManager.h"
#include <EmonLib.h>
#include "TimeLibJMN.h"
#include <TimeLib.h>

#define CHILD_ID_S0 100            // Entrada de energia na casa max 90A
#define GET_SENSOR_ADDRESS(X) CHILD_ID_S0 + X*10
#define GET_SENSOR_ADDRESS_STOREDATA(X) (GET_SENSOR_ADDRESS(X) + 1)
#define GET_SENSOR_ADDRESS_DAY(X) (GET_SENSOR_ADDRESS(X) + 2)
#define GET_SENSOR_ADDRESS_NIGHT(X) (GET_SENSOR_ADDRESS(X) + 3)

#define GET_SENSOR_ARRAY_ID(X) (X - CHILD_ID_S0) % 10
#define GET_SENSOR_ARRAY_ID_STOREDATA(X) (GET_SENSOR_ARRAY_ID(X) - 1)
#define GET_SENSOR_ARRAY_ID_DAY(X) (GET_SENSOR_ARRAY_ID(X) - 2)
#define GET_SENSOR_ARRAY_ID_NIGHT(X) (GET_SENSOR_ARRAY_ID(X) - 4)
 
enum SENSORSTATUS
{
	NOT_PRESENTED = 0,
	CALIB_NOT_PRESENTED = 1,
	DAY_SENSOR_NOT_PRESENTED = 3,
	NIGHT_SENSOR_NOT_PRESENTED = 6,
	WaitingCALIBRATION = 2,
	WaitingKWH_DAY = 5,
	WaitingKWH_NIGHT = 8,
	WaitingKWH = 9,
	READY = 10,
};

const char *POWER_0 = "PTotal0";
const char *POWER_1 = "PTotal1";
const char *POWER_2 = "PTotal2";
const char *POWER_3 = "PTotal3";
const char *POWER_4 = "PTotal4";
const char *POWER_5 = "PTotal5";
const char *POWER_6 = "PTotal6";
const char *POWER_7 = "PTotal7";
const char *CALIBRI_0 = "Calib0";
const char *CALIBRI_1 = "Calib1";
const char *CALIBRI_2 = "Calib2";
const char *CALIBRI_3 = "Calib3";
const char *CALIBRI_4 = "Calib4";
const char *CALIBRI_5 = "Calib5";
const char *CALIBRI_6 = "Calib6";
const char *CALIBRI_7 = "Calib7";
const char *POWERD_0 = "PTotalD0";
const char *POWERD_1 = "PTotalD1";
const char *POWERD_2 = "PTotalD2";
const char *POWERD_3 = "PTotalD3";
const char *POWERD_4 = "PTotalD4";
const char *POWERD_5 = "PTotalD5";
const char *POWERD_6 = "PTotalD6";
const char *POWERD_7 = "PTotalD7";
const char *POWERN_0 = "PTotalN0";
const char *POWERN_1 = "PTotalN1";
const char *POWERN_2 = "PTotalN2";
const char *POWERN_3 = "PTotalN3";
const char *POWERN_4 = "PTotalN4";
const char *POWERN_5 = "PTotalN5";
const char *POWERN_6 = "PTotalN6";
const char *POWERN_7 = "PTotalN7";


EnergyMonitor emon;
bool allinited = false;
DataStoraManager dataManage(NUMBER_OF_SENSORS);


//TIME
bool timeInited = false;
unsigned long requestedTimeControl = 0;
time_t lastTimeUpdate;

unsigned long printTimeControl = 0;

int32_t testTime = 0;

struct SensorsData
{
	double kwh;
	double kwhDay;
	double kwhNight;
	double lastKWHSent;
	
	uint8_t sensorPort;
	
	//Total
	uint8_t id_total;

	//General
	MyMessage mysensorMsg;
	double calibrations;
	uint8_t status;
	unsigned long lastUpdate;
	double lastWattRead;
	double accumulateKW;
	uint8_t numberOfAdds;
	unsigned long lastTimeRecalcKWH;
	double tempKWH;

	void init(uint8_t configID)
	{
		id_total = configID;
		calibrations = 0.0;
		status = SENSORSTATUS::NOT_PRESENTED;
		lastUpdate = TimeLibJMN::getCurrentTime();
		lastWattRead = 0;
		numberOfAdds = 0;
		accumulateKW = 0;
		tempKWH = 0;
	}


	void presentSensor()
	{
		while (status == NOT_PRESENTED )
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("Presenting Power Mesure sensor TOTAL:");
			Serial.println(id_total);
#endif
			if (id_total == 0) present(GetSensorAddress(), S_POWER, POWER_0, true);
#if NUMBER_OF_SENSORS > 1
			if (id_total == 1) present(GetSensorAddress(), S_POWER, POWER_1, true); 
#endif
#if NUMBER_OF_SENSORS > 2
			if (id_total == 2) present(GetSensorAddress(), S_POWER, POWER_2, true);
#endif
#if NUMBER_OF_SENSORS > 3
			if (id_total == 3) present(GetSensorAddress(), S_POWER, POWER_3, true);
#endif
#if NUMBER_OF_SENSORS > 4
			if (id_total == 4) present(GetSensorAddress(), S_POWER, POWER_4, true);
#endif
#if NUMBER_OF_SENSORS > 5
			if (id_total == 5) present(GetSensorAddress(), S_POWER, POWER_5, true);
#endif
#if NUMBER_OF_SENSORS > 6
			if (id_total == 6) present(GetSensorAddress(), S_POWER, POWER_6, true);
#endif
#if NUMBER_OF_SENSORS > 7
			if (id_total == 7) present(GetSensorAddress(), S_POWER, POWER_7, true);
#endif
			wait(1000);
		}
		while (status == CALIB_NOT_PRESENTED)
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("Presenting Calibration sensor:");
			Serial.println(id_total);
#endif
			if (id_total == 0) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_0, true);
#if NUMBER_OF_SENSORS > 1
			if (id_total == 1) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_1, true);
#endif
#if NUMBER_OF_SENSORS > 2
			if (id_total == 2) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_2, true);
#endif
#if NUMBER_OF_SENSORS > 3
			if (id_total == 3) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_3, true);
#endif
#if NUMBER_OF_SENSORS > 4
			if (id_total == 4) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_4, true);
#endif
#if NUMBER_OF_SENSORS > 5
			if (id_total == 5) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_5, true);
#endif
#if NUMBER_OF_SENSORS > 6
			if (id_total == 6) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_6, true);
#endif
#if NUMBER_OF_SENSORS > 7
			if (id_total == 7) present(GetSensorAddressStoreData(), CALIBRATION_TYPE, CALIBRI_7, true);
#endif
			wait(1000);
		}
		while (status == DAY_SENSOR_NOT_PRESENTED)
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("Presenting Power Mesure sensor Day:");
			Serial.println(id_total);
#endif
			if (id_total == 0) present(GetSensorAddressDay(), S_POWER, POWERD_0, true);
#if NUMBER_OF_SENSORS > 1
			if (id_total == 1) present(GetSensorAddressDay(), S_POWER, POWERD_1, true);
#endif
#if NUMBER_OF_SENSORS > 2
			if (id_total == 2) present(GetSensorAddressDay(), S_POWER, POWERD_2, true);
#endif
#if NUMBER_OF_SENSORS > 3
			if (id_total == 3) present(GetSensorAddressDay(), S_POWER, POWERD_3, true);
#endif
#if NUMBER_OF_SENSORS > 4
			if (id_total == 4) present(GetSensorAddressDay(), S_POWER, POWERD_4, true);
#endif
#if NUMBER_OF_SENSORS > 5
			if (id_total == 5) present(GetSensorAddressDay(), S_POWER, POWERD_5, true);
#endif
#if NUMBER_OF_SENSORS > 6
			if (id_total == 6) present(GetSensorAddressDay(), S_POWER, POWERD_6, true);
#endif
#if NUMBER_OF_SENSORS > 7
			if (id_total == 7) present(GetSensorAddressDay(), S_POWER, POWERD_7, true);
#endif
			wait(1000);
		}
		while (status == NIGHT_SENSOR_NOT_PRESENTED)
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("Presenting Power Mesure sensor Night:");
			Serial.println(id_total);
#endif
			if (id_total == 0) present(GetSensorAddressNight(), S_POWER, POWERN_0, true);
#if NUMBER_OF_SENSORS > 1
			if (id_total == 1) present(GetSensorAddressNight(), S_POWER, POWERN_1, true);
#endif
#if NUMBER_OF_SENSORS > 2
			if (id_total == 2) present(GetSensorAddressNight(), S_POWER, POWERN_2, true);
#endif
#if NUMBER_OF_SENSORS > 3
			if (id_total == 3) present(GetSensorAddressNight(), S_POWER, POWERN_3, true);
#endif
#if NUMBER_OF_SENSORS > 4
			if (id_total == 4) present(GetSensorAddressNight(), S_POWER, POWERN_4, true);
#endif
#if NUMBER_OF_SENSORS > 5
			if (id_total == 5) present(GetSensorAddressNight(), S_POWER, POWERN_5, true);
#endif
#if NUMBER_OF_SENSORS > 6
			if (id_total == 6) present(GetSensorAddressNight(), S_POWER, POWERN_6, true);
#endif
#if NUMBER_OF_SENSORS > 7
			if (id_total == 7) present(GetSensorAddressNight(), S_POWER, POWERN_7, true);
#endif
			wait(1000);
		}
	}

	void processMessage(const MyMessage &message)
	{
		if ((GetSensorAddress() ==  message.sensor) ||
			(GetSensorAddressStoreData() == message.sensor))
		{
			//Process S_POWER_PRESENTATION
			if (message.type == S_POWER && message.getCommand() == C_PRESENTATION)
			{
				presented();
				return;
			}
			//Process STOREDATA_SENSOR_PRESENTATION
			if (message.type == CALIBRATION_TYPE && message.getCommand() == C_PRESENTATION)
			{
				presentedCalibri();
				return;
			}

			//Restore Saved Calibration Value
			if (message.type == CALIBRATION_STORE_TYPE)
			{
				uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
				long calib = message.getLong();
#ifdef JMN_DEBUG_VERBOSE
				Serial.print("Received calibrations sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(calib);
#endif
				calibrationReceived(calib);
				return;
			}

			//Restore Saved KWH Value
			if (message.type == KWH_STORE_TYPE)
			{
				uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
				kwhReceived(message.getLong());
#ifdef JMN_DEBUG_VERBOSE
				Serial.print("Received last kwh consumption sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(kwh);
#endif
				return;
			}

			//Restore Saved KWH Value DAY
			if (message.type == KWH_STORE_TYPE_DAY)
			{
				uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
				kwhReceived_DAY(message.getLong());
#ifdef JMN_DEBUG_VERBOSE
				Serial.print("Received last kwh consumption DAY sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(kwhDay);
#endif
				return;
			}

			//Restore Saved KWH Value DAY
			if (message.type == KWH_STORE_TYPE_NIGHT)
			{
				uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
				kwhReceived_NIGHT(message.getLong());
#ifdef JMN_DEBUG_VERBOSE
				Serial.print("Received last kwh consumption NIGHT sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(kwhNight);
#endif
				return;
			}
		}
		//DAY
		Serial.print("Sensor Day Addr:");
		Serial.println(GetSensorAddressDay());
		if (GetSensorAddressDay() == message.sensor)
		{
			Serial.println("Entered Day MSG Process!!!");
			//Process S_POWER_PRESENTATION
			if (message.type == S_POWER && message.getCommand() == C_PRESENTATION)
			{
				presentedDay();
				return;
			}


		}
		//NIGHT
		if (GetSensorAddressNight() == message.sensor)
		{
			Serial.println("Entered Night MSG Process!!");
			//Process S_POWER_PRESENTATION
			if (message.type == S_POWER && message.getCommand() == C_PRESENTATION)
			{
				presentedNight();
				return;
			}
		}
	}

	void presented()
	{
		if (status == SENSORSTATUS::NOT_PRESENTED)
		{
			status = CALIB_NOT_PRESENTED;
#ifdef JMN_DEBUG_INFO
			Serial.print("Sensor: ");
			Serial.print(id_total);
			Serial.println(" - NOT_PRESENTED -> CALIB_NOT_PRESENTED");
#endif
		}
	}

	void presentedDay()
	{
		if (status == SENSORSTATUS::DAY_SENSOR_NOT_PRESENTED)
		{
			status = SENSORSTATUS::NIGHT_SENSOR_NOT_PRESENTED;
#ifdef JMN_DEBUG_INFO
			Serial.print("Day Sensor: ");
			Serial.print(id_total);
			Serial.println(" - DAY_SENSOR_NOT_PRESENTED -> NIGHT_SENSOR_NOT_PRESENTED");
#endif
		}
	}

	void presentedNight()
	{
		if (status == SENSORSTATUS::NIGHT_SENSOR_NOT_PRESENTED)
		{
			status = SENSORSTATUS::WaitingCALIBRATION;
#ifdef JMN_DEBUG_INFO
			Serial.print("Night Sensor: ");
			Serial.print(id_total);
			Serial.println(" - NIGHT_SENSOR_NOT_PRESENTED -> WaitingCALIBRATION");
#endif
		}
	}

	void presentedCalibri()
	{
		if (status == SENSORSTATUS::CALIB_NOT_PRESENTED)
		{
			status = DAY_SENSOR_NOT_PRESENTED;
#ifdef JMN_DEBUG_INFO
			Serial.print("Sensor: ");
			Serial.print(id_total);
			Serial.println(" - CALIB_NOT_PRESENTED -> DAY_SENSOR_NOT_PRESENTED");
#endif
		}
	}

	void calibrationReceived(unsigned long newCalibration)
	{
#ifdef JMN_DEBUG_CALIB
		Serial.print("CalibrationReceived - New:");
		Serial.print(newCalibration);
		Serial.print(" - Old:");
		Serial.println(calibrations);
#endif
		double newCalibretionValue = (double)newCalibration / 100;
		if (status == SENSORSTATUS::WaitingCALIBRATION)
		{
			calibrations = newCalibretionValue;
			status = SENSORSTATUS::WaitingKWH_DAY;
#ifdef JMN_DEBUG_CALIB
			Serial.print("Sensor:");
			Serial.print(id_total);
			Serial.print(" Calibra:");
			Serial.print(newCalibretionValue);
			Serial.println(" - WaitingCALIBRATION -> WaitingKWH_DAY");
#endif
		}
		else
		{
			if (calibrations != newCalibretionValue)
			{
				updateCalibrations(newCalibretionValue);
			}
		}
	}

	void updateCalibrations(double newCalibration)
	{
		calibrations = newCalibration;
#ifdef JMN_DEBUG_CALIB
		Serial.print("Setting New Calibration Sensor:");
		Serial.print(id_total);
		Serial.print(" Value:");
		Serial.println(calibrations);
#endif
		mysensorMsg.setSensor(GetSensorAddressStoreData());
		mysensorMsg.setType(CALIBRATION_STORE_TYPE);
		send(mysensorMsg.set(calibrations * 100, 2));
	}

	void kwhReceived(unsigned long newKwh)
	{
		kwh = (double)newKwh / 100;
		status = SENSORSTATUS::READY;
#ifdef JMN_DEBUG_INFO
		Serial.print("Sensor:");
		Serial.print(id_total);
		Serial.print(" NewValue:");
		Serial.print(kwh);
		Serial.println(" - WaitingKWH -> READY");
#endif
		lastTimeRecalcKWH = TimeLibJMN::getCurrentTime();
	}

	void kwhReceived_DAY(unsigned long newKwh)
	{
		kwhDay = (double)newKwh / 100;
		status = SENSORSTATUS::WaitingKWH_NIGHT;
#ifdef JMN_DEBUG_INFO
		Serial.print("Sensor DAY:");
		Serial.print(id_total);
		Serial.print(" NewValue:");
		Serial.print(kwh);
		Serial.println(" - WaitingKWH_DAY -> WaitingKWH_NIGHT");
#endif
		lastTimeRecalcKWH = TimeLibJMN::getCurrentTime();
	}

	void kwhReceived_NIGHT(unsigned long newKwh)
	{
		kwhNight = (double)newKwh / 100;
		status = SENSORSTATUS::WaitingKWH;
#ifdef JMN_DEBUG_INFO
		Serial.print("Sensor NIGHT:");
		Serial.print(id_total);
		Serial.print(" NewValue:");
		Serial.print(kwh);
		Serial.println(" - WaitingKWH_Night -> WaitingKWH");
#endif
		lastTimeRecalcKWH = TimeLibJMN::getCurrentTime();
	}

	bool isDay()
	{
		if(hour() >= 22 || hour() <= 11)
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.println("Is Night!");
#endif
			return false;
		}
		else
		{
#ifdef JMN_DEBUG_VERBOSE
			Serial.println("Is Day!");
#endif			
			return true;
		}
	}

	void addWattRead(double readWatt)
	{
		lastWattRead = readWatt;
		accumulateKW += readWatt;
		numberOfAdds++;
		if (numberOfAdds == NUMBER_READS_TO_UPDATE_KWH)
		{
			unsigned long milis = TimeLibJMN::milliSecPassed(lastTimeRecalcKWH);

			tempKWH += ((accumulateKW / numberOfAdds)*milis);

			accumulateKW = 0;
			lastTimeRecalcKWH = TimeLibJMN::getCurrentTime();
			numberOfAdds = 0;
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("TempKWH Sensor:");
			Serial.print(id_total);
			Serial.print(" Millis:");
			Serial.print(milis);
			Serial.print(" Value:");
			Serial.println(tempKWH);
#endif
		}
		if (tempKWH > (3600000000 * POWER_METER_PRECISION))
		{
			bool isDay_ = isDay();
			while (tempKWH > (3600000000 * POWER_METER_PRECISION))
			{
				tempKWH -= (3600000000 * POWER_METER_PRECISION);
				kwh += POWER_METER_PRECISION;
				if(isDay_)
				{
					kwhDay += POWER_METER_PRECISION;
				}
				else
				{
					kwhNight += POWER_METER_PRECISION;
				}
			}
#ifdef JMN_DEBUG_VERBOSE
			Serial.print("kwh incrised sensor:");
			Serial.print(id_total);
			Serial.print(" Value:");
			Serial.println(kwh);
#endif
		}
	}


	double getLastWatt()
	{
		return lastWattRead;
	}

	uint8_t GetSensorAddress()
	{
		return GET_SENSOR_ADDRESS(id_total);
	}

	uint8_t GetSensorAddressStoreData()
	{
		return GET_SENSOR_ADDRESS_STOREDATA(id_total);
	}

	uint8_t GetSensorAddressDay()
	{
		return GET_SENSOR_ADDRESS_DAY(id_total);
	}


	uint8_t GetSensorAddressNight()
	{
		return GET_SENSOR_ADDRESS_NIGHT(id_total);
	}


	void SendData()
	{
		if (TimeLibJMN::secPassed(lastUpdate) >= REFRESH_TIME)
		{
			mysensorMsg.setSensor(GetSensorAddress());
			mysensorMsg.setType(V_WATT);
			send(mysensorMsg.set(getLastWatt(), 2));
			wait(100);

			lastUpdate = TimeLibJMN::getCurrentTime();
		}
		if(kwh != lastKWHSent)
		{
			lastKWHSent = kwh;
			mysensorMsg.setSensor(GetSensorAddress());
			mysensorMsg.setType(V_KWH);
			send(mysensorMsg.set(kwh, 2));
			wait(100);

			mysensorMsg.setSensor(GetSensorAddressStoreData());
			mysensorMsg.setType(KWH_STORE_TYPE);
			send(mysensorMsg.set(kwh * 100, 2));
			wait(100);

			mysensorMsg.setSensor(GetSensorAddressDay());
			mysensorMsg.setType(V_KWH);
			send(mysensorMsg.set(kwhDay, 2));
			wait(100);

			mysensorMsg.setSensor(GetSensorAddressStoreData());
			mysensorMsg.setType(KWH_STORE_TYPE_DAY);
			send(mysensorMsg.set(kwhDay * 100, 2));
			wait(100);

			mysensorMsg.setSensor(GetSensorAddressNight());
			mysensorMsg.setType(V_KWH);
			send(mysensorMsg.set(kwhNight, 2));
			wait(100);

			mysensorMsg.setSensor(GetSensorAddressStoreData());
			mysensorMsg.setType(KWH_STORE_TYPE_NIGHT);
			send(mysensorMsg.set(kwhNight * 100, 2));
			wait(100);

		}

		wait(500);
	}

	void prepareEMON()
	{
#ifdef JMN_DEBUG_VERBOSE
		Serial.print("Prepare Sensor:");
		Serial.print(id_total);
		Serial.print(" Calibri:");
		Serial.println(calibrations);
#endif
		emon.current(id_total, calibrations);
	}

	void ReadData()
	{
		prepareEMON();
		float Irms = emon.calcIrms(1480);
		float watt = Irms * CURRENT_REF_VOLTAGE;
#ifdef JMN_DEBUG_CALIB
		Serial.print("Read Sensor: ");
		Serial.print(id_total);
		Serial.print(" Aparent Power: ");
		Serial.print(watt);	       // Apparent power
		Serial.print(" Current: ");
		Serial.print(Irms);
		Serial.print(" AnalogRead: ");
		double val = analogRead(id_total);
		Serial.println(val);
#endif
		addWattRead(watt);
		wait(300);
	}

	void run()
	{
			switch (status)
			{
			case SENSORSTATUS::WaitingKWH_DAY:
				if (TimeLibJMN::secPassed(lastUpdate) > 5)
				{
					request(GetSensorAddressStoreData(), KWH_STORE_TYPE_DAY);
					lastUpdate = TimeLibJMN::getCurrentTime();
					wait(200);
				}
				break;
			
			case SENSORSTATUS::WaitingKWH_NIGHT:
				if (TimeLibJMN::secPassed(lastUpdate) > 5)
				{
					request(GetSensorAddressStoreData(), KWH_STORE_TYPE_NIGHT);
					lastUpdate = TimeLibJMN::getCurrentTime();
					wait(200);
				}
				break;

			case SENSORSTATUS::WaitingKWH:
				if (TimeLibJMN::secPassed(lastUpdate) > 5)
				{
					request(GetSensorAddressStoreData(), KWH_STORE_TYPE);
					lastUpdate = TimeLibJMN::getCurrentTime();
					wait(200);
				}
				break;

			case SENSORSTATUS::WaitingCALIBRATION:
				if (TimeLibJMN::secPassed(lastUpdate) > 5)
				{
					request(GetSensorAddressStoreData(), CALIBRATION_STORE_TYPE);
					lastUpdate = TimeLibJMN::getCurrentTime();
				}
				break;
			case SENSORSTATUS::READY:
				ReadData();
				SendData();
			default:
				break;
			}
			wait(100);
	}

};

SensorsData sensors[NUMBER_OF_SENSORS];


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


