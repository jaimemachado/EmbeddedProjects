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


// Enable debug prints

#include "SensorImpl.h"
#define NUMBER_OF_SENSORS 1
#define NUMBER_READS_TO_UPDATE_KWH 10
#define CURRENT_REF_VOLTAGE 230.0
#define REFRESH_TIME 5

#define MY_DEBUG

#define CALIBRATION_TYPE S_RGB_LIGHT
#define CALIBRATION_STORE_TYPE V_VAR1
#define KWH_STORE_TYPE V_VAR2

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#include <MySensors.h>

#include "DataStoraManager.h"
#include <EmonLib.h>
#include "TimeLib.h"

#define CHILD_ID_S0 100              // Entrada de energia na casa max 90A
#define GET_SENSOR_ADDRESS(X) CHILD_ID_S0 + X
#define GET_SENSOR_ADDRESS_STOREDATA(X) (CHILD_ID_S0 + X + 20)
#define GET_SENSOR_ARRAY_ID(X) X - CHILD_ID_S0 
#define GET_SENSOR_ARRAY_ID_STOREDATA(X) (X - CHILD_ID_S0 - 20)
 
enum SENSORSTATUS
{
	NOT_PRESENTED = 0,
	CALIB_NOT_PRESENTED = 1,
	WaitingCALIBRATION = 2,
	WaitingKWH = 3,
	READY = 4,
};

struct SensorsData
{
	double kwh;
	
	uint8_t id;
	MyMessage watMsg;
	MyMessage kwhMsg;
	MyMessage storeMsg;
	double calibrations;
	uint8_t status;
	uint8_t updatesStatus;
	unsigned long lastUpdate;
	double lastWattRead;
	double accumulateKW;
	uint8_t numberOfAdds;
	unsigned long lastTimeRecalcKWH;
	double tempKWH;

	void init(uint8_t configID)
	{
		id = configID;
		calibrations = 0.0;
		watMsg.setSensor(GetSensorAddress());
		watMsg.setType(V_WATT);
		kwhMsg.setSensor(GetSensorAddress());
		kwhMsg.setType(V_KWH);
		storeMsg.setSensor(GetSensorAddressCalibration());
		storeMsg.setType(KWH_STORE_TYPE);
		status = SENSORSTATUS::NOT_PRESENTED;
		lastUpdate = TimeLib::getCurrentTime();
		lastWattRead = 0;
		numberOfAdds = 0;
		accumulateKW = 0;
		tempKWH = 0;
	}

	void addWattRead(double readWatt)
	{
		lastWattRead = readWatt;
		accumulateKW += readWatt;
		numberOfAdds++;
		if(numberOfAdds == NUMBER_READS_TO_UPDATE_KWH)
		{
			unsigned long milis = TimeLib::milliSecPassed(lastTimeRecalcKWH);

			//tempKWH += ((accumulateKW / numberOfAdds)*milis);
			tempKWH += 36000010;

			lastTimeRecalcKWH = TimeLib::getCurrentTime();
			numberOfAdds = 0;
			Serial.print("TempKWH Sensor:");
			Serial.print(id);
			Serial.print(" Value:");
			Serial.println(tempKWH);
		}
		if(tempKWH > 36000000)
		{
			tempKWH -= 36000000;
			kwh += 0.01;
			Serial.print("kwh incrised sensor:");
			Serial.print(id);
			Serial.print(" Value:");
			Serial.println(kwh);
		}
	}

	double getLastWatt()
	{
		return lastWattRead;
	}

	uint8_t GetSensorAddress()
	{
		return GET_SENSOR_ADDRESS(id);
	}

	uint8_t GetSensorAddressCalibration()
	{
		return GET_SENSOR_ADDRESS_STOREDATA(id);
	}

	void presented()
	{
		if (status == SENSORSTATUS::NOT_PRESENTED)
		{
			status = CALIB_NOT_PRESENTED;
			Serial.print("Sensor:");
			Serial.print(id);
			Serial.println(" - NOT_PRESENTED -> CALIB_NOT_PRESENTED");
		}
	}

	void presentedCalibri()
	{
		if (status == SENSORSTATUS::CALIB_NOT_PRESENTED)
		{
			status = WaitingCALIBRATION;
			Serial.print("Sensor:");
			Serial.print(id);
			Serial.println(" - CALIB_NOT_PRESENTED -> WaitingCALIBRATION");
		}
	}

	void kwhReceived(unsigned long newKwh)
	{
		kwh = newKwh;
		status = SENSORSTATUS::READY;
		Serial.print("Sensor:");
		Serial.print(id);
		Serial.print(" NewValue:");
		Serial.print(kwh);
		Serial.println(" - WaitingKWH -> READY");
		lastTimeRecalcKWH = TimeLib::getCurrentTime();
	}
	
	void updateCalibrations(unsigned long newCalibration)
	{
		calibrations = ((double)(newCalibration)) / 100;
		Serial.print("Setting New Calibration Sensor:");
		Serial.print(id);
		Serial.print(" Value:");
		Serial.println(calibrations);
		storeMsg.setType(CALIBRATION_STORE_TYPE);
		send(storeMsg.set(calibrations*100, 2), true);
		storeMsg.setType(KWH_STORE_TYPE);
	}

	void calibrationReceived(unsigned long newCalibration)
	{
		if (status == SENSORSTATUS::WaitingCALIBRATION)
		{
			calibrations = newCalibration/100;
			status = SENSORSTATUS::WaitingKWH;
			Serial.print("Sensor:");
			Serial.print(id);
			Serial.println(" - WaitingCALIBRATION -> WaitingKWH");
			
		}
		else
		{
			if(calibrations != (newCalibration/100))
			{
				updateCalibrations(newCalibration);
			}
		}
	}
};

SensorsData sensors[NUMBER_OF_SENSORS];
EnergyMonitor emon;
bool allinited = false;


void prepareEMON(uint8_t id)
{
	Serial.print("Prepare Sensor:");
	Serial.print(id);
	Serial.print(" Calibri:");
	Serial.println(sensors[id].calibrations);
	emon.current(id, sensors[id].calibrations);
}

void setup()
{
}

void receive(const MyMessage &message)
{
	Serial.print("Received message Sensor:");
	Serial.print(message.sensor);
	Serial.print(" Command:");
	Serial.print(message.getCommand());
	Serial.print(" Type:");
	Serial.println(message.type);

	if(message.type == S_POWER && message.getCommand() == C_PRESENTATION)
	{
		uint8_t sensorID = GET_SENSOR_ARRAY_ID(message.sensor);
		sensors[sensorID].presented();
		return;
	}
	if (message.type == CALIBRATION_TYPE && message.getCommand() == C_PRESENTATION)
	{
		Serial.print("Presented CalibreSensor:");
		Serial.println(GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor));
		uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
		sensors[sensorID].presentedCalibri();
		return;
	}

	//Saved KWHData
	if (message.type == KWH_STORE_TYPE)
	{
		if (message.sensor >= 120 && (message.sensor < (120 + NUMBER_OF_SENSORS)))
		{
			uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
			sensors[sensorID].kwhReceived(message.getLong());
			Serial.print("Received last kwh consumption sensor ID:");
			Serial.print(sensorID);
			Serial.print(" Value:");
			Serial.println(sensors[sensorID].kwh);
			return;
		}
	}
	//Calibration
	if (message.type == CALIBRATION_STORE_TYPE)
	{
		if (message.sensor >= 120 && (message.sensor < (120 + NUMBER_OF_SENSORS)))
		{
			uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
			long calib = message.getLong();
			Serial.print("Received calibrations sensor ID:");
			Serial.print(sensorID);
			Serial.print(" Value:");
			Serial.println(calib);
			
			sensors[sensorID].calibrationReceived(calib);

			return;
		}
		
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
		int tryies = 5;
		while (sensors[x].status == NOT_PRESENTED && tryies !=0)
		{
			present(sensors[x].GetSensorAddress(), S_POWER, "Power Mesure", true);
			wait(200);
			tryies--;
		}
		tryies = 5;
		while (sensors[x].status == CALIB_NOT_PRESENTED && tryies != 0)
		{
			present(sensors[x].GetSensorAddressCalibration(), CALIBRATION_TYPE, "DummyStoreCalibration", true);
			wait(200);
			tryies--;
		}
	}
	
}

bool initialization()
{
	uint8_t allOK = NUMBER_OF_SENSORS;
	for( int sensorID = 0; sensorID < NUMBER_OF_SENSORS; sensorID++)
	{
		SensorsData* sensor = &(sensors[sensorID]);
		switch (sensors[sensorID].status)
		{
		case SENSORSTATUS::WaitingKWH:
			if (TimeLib::secPassed(sensor->lastUpdate) > 5)
			{
				request(sensor->GetSensorAddressCalibration(), KWH_STORE_TYPE);
				sensor->lastUpdate = TimeLib::getCurrentTime();
				wait(200);
			}
			break;

		case SENSORSTATUS::WaitingCALIBRATION:
			if (TimeLib::secPassed(sensor->lastUpdate) > 5)
			{
				request(sensor->GetSensorAddressCalibration(), CALIBRATION_STORE_TYPE);
				sensor->lastUpdate = TimeLib::getCurrentTime();
			}
			break;
		case SENSORSTATUS::READY:
			allOK--;
			continue;
		default:
			break;
		}
		wait(100);
	}
	return allOK == 0;
}

void ReadData()
{
	for (int sensorID = 0; sensorID < NUMBER_OF_SENSORS; sensorID++)
	{
		SensorsData* sensor = &(sensors[sensorID]);
		if (sensors[sensorID].status == SENSORSTATUS::READY)
		{
			prepareEMON(sensor->id);
			float Irms = emon.calcIrms(1480);
			float watt = Irms * CURRENT_REF_VOLTAGE;

			Serial.print("Read Sensor: ");
			Serial.print(sensorID);
			Serial.print("Aparent Power: ");
			Serial.print(watt);	       // Apparent power
			Serial.print(" Current: ");
			Serial.print(Irms);
			Serial.print(" AnalogRead: ");
			double val = analogRead(sensorID);
			Serial.println(val);
			sensor->addWattRead(watt);
		}
		wait(300);
	}
}

void SendData()
{
	for (int sensorID = 0; sensorID < NUMBER_OF_SENSORS; sensorID++)
	{
		if (sensors[sensorID].status == SENSORSTATUS::READY)
		{
			SensorsData* sensor = &(sensors[sensorID]);
			if (TimeLib::secPassed(sensor->lastUpdate) >= REFRESH_TIME)
			{
				send(sensor->watMsg.set(sensor->getLastWatt(), 2));
				wait(100);

				send(sensor->kwhMsg.set(sensor->kwh/100, 2));
				wait(100);
				send(sensor->storeMsg.set(sensor->kwh, 2));
				wait(100);
				sensor->lastUpdate = TimeLib::getCurrentTime();
			}
			wait(500);
		}
	}
}

void loop()
{
	if(!allinited)
	{
		allinited = initialization();
	}
	ReadData();
	SendData();
	
	wait(10000);
}

