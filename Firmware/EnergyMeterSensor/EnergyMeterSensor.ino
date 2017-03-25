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
#define NUMBER_OF_SENSORS 1
#define NUMBER_READS_TO_UPDATE_KWH 10
#define CURRENT_REF_VOLTAGE 230.0
#define REFRESH_TIME 5
#define POWER_METER_PRECISION 0.0001

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

EnergyMonitor emon;
bool allinited = false;
DataStoraManager dataManage(NUMBER_OF_SENSORS);

struct SensorsData
{
	double kwh;
	double lastKWHSent;
	
	uint8_t id;
	MyMessage mysensorMsg;
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
		status = SENSORSTATUS::NOT_PRESENTED;
		lastUpdate = TimeLib::getCurrentTime();
		lastWattRead = 0;
		numberOfAdds = 0;
		accumulateKW = 0;
		tempKWH = 0;
	}


	void presentSensor()
	{
		int tryies = 5;
		while (status == NOT_PRESENTED && tryies != 0)
		{
			present(GetSensorAddress(), S_POWER, "Power Mesure", true);
			wait(200);
			tryies--;
		}
		tryies = 5;
		while (status == CALIB_NOT_PRESENTED && tryies != 0)
		{
			present(GetSensorAddressStoreData(), CALIBRATION_TYPE, "DummyStoreCalibration", true);
			wait(200);
			tryies--;
		}
	}

	void processMessage(const MyMessage &message)
	{
		if ((id == GET_SENSOR_ARRAY_ID(message.sensor)) ||
			(id == GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor)))
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
				Serial.print("Received calibrations sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(calib);
				calibrationReceived(calib);
				return;
			}

			//Restore Saved KWH Value
			if (message.type == KWH_STORE_TYPE)
			{
				uint8_t sensorID = GET_SENSOR_ARRAY_ID_STOREDATA(message.sensor);
				kwhReceived(message.getLong());
				Serial.print("Received last kwh consumption sensor ID:");
				Serial.print(sensorID);
				Serial.print(" Value:");
				Serial.println(kwh);
				return;
			}
		}
	}

	void presented()
	{
		if (status == SENSORSTATUS::NOT_PRESENTED)
		{
			status = CALIB_NOT_PRESENTED;
			Serial.print("Sensor: ");
			Serial.print(id);
			Serial.println(" - NOT_PRESENTED -> CALIB_NOT_PRESENTED");
		}
	}

	void presentedCalibri()
	{
		if (status == SENSORSTATUS::CALIB_NOT_PRESENTED)
		{
			status = WaitingCALIBRATION;
			Serial.print("Sensor: ");
			Serial.print(id);
			Serial.println(" - CALIB_NOT_PRESENTED -> WaitingCALIBRATION");
		}
	}

	void calibrationReceived(unsigned long newCalibration)
	{
		double newCalibretionValue = (double)newCalibration / 100;
		if (status == SENSORSTATUS::WaitingCALIBRATION)
		{
			calibrations = newCalibretionValue;
			status = SENSORSTATUS::WaitingKWH;
			Serial.print("Sensor:");
			Serial.print(id);
			Serial.print(" Calibra:");
			Serial.print(newCalibretionValue);
			Serial.println(" - WaitingCALIBRATION -> WaitingKWH");

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
		Serial.print("Setting New Calibration Sensor:");
		Serial.print(id);
		Serial.print(" Value:");
		Serial.println(calibrations);

		mysensorMsg.setSensor(GetSensorAddressStoreData());
		mysensorMsg.setType(CALIBRATION_STORE_TYPE);
		send(mysensorMsg.set(calibrations * 100, 2));
	}

	void kwhReceived(unsigned long newKwh)
	{
		kwh = (double)newKwh / 100;
		status = SENSORSTATUS::READY;
		Serial.print("Sensor:");
		Serial.print(id);
		Serial.print(" NewValue:");
		Serial.print(kwh);
		Serial.println(" - WaitingKWH -> READY");
		lastTimeRecalcKWH = TimeLib::getCurrentTime();
	}


	void addWattRead(double readWatt)
	{
		lastWattRead = readWatt;
		accumulateKW += readWatt;
		numberOfAdds++;
		if (numberOfAdds == NUMBER_READS_TO_UPDATE_KWH)
		{
			unsigned long milis = TimeLib::milliSecPassed(lastTimeRecalcKWH);

			tempKWH += ((accumulateKW / numberOfAdds)*milis);

			accumulateKW = 0;
			lastTimeRecalcKWH = TimeLib::getCurrentTime();
			numberOfAdds = 0;
			Serial.print("TempKWH Sensor:");
			Serial.print(id);
			Serial.print(" Millis:");
			Serial.print(milis);
			Serial.print(" Value:");
			Serial.println(tempKWH);
		}
		if (tempKWH > (3600000000 * POWER_METER_PRECISION))
		{
			while (tempKWH > (3600000000 * POWER_METER_PRECISION))
			{
				tempKWH -= (3600000000 * POWER_METER_PRECISION);
				kwh += POWER_METER_PRECISION;
			}
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

	uint8_t GetSensorAddressStoreData()
	{
		return GET_SENSOR_ADDRESS_STOREDATA(id);
	}

	void SendData()
	{
		if (TimeLib::secPassed(lastUpdate) >= REFRESH_TIME)
		{
			mysensorMsg.setSensor(GetSensorAddress());
			mysensorMsg.setType(V_WATT);
			send(mysensorMsg.set(getLastWatt(), 2));
			wait(100);

			lastUpdate = TimeLib::getCurrentTime();
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
		}

		wait(500);
	}

	void prepareEMON()
	{
		Serial.print("Prepare Sensor:");
		Serial.print(id);
		Serial.print(" Calibri:");
		Serial.println(calibrations);
		emon.current(id, calibrations);
	}

	void ReadData()
	{
		prepareEMON();
		float Irms = emon.calcIrms(1480);
		float watt = Irms * CURRENT_REF_VOLTAGE;

		Serial.print("Read Sensor: ");
		Serial.print(id);
		Serial.print(" Aparent Power: ");
		Serial.print(watt);	       // Apparent power
		Serial.print(" Current: ");
		Serial.print(Irms);
		Serial.print(" AnalogRead: ");
		double val = analogRead(id);
		Serial.println(val);
		addWattRead(watt);
		wait(300);
	}

	void run()
	{
			switch (status)
			{
			case SENSORSTATUS::WaitingKWH:
				if (TimeLib::secPassed(lastUpdate) > 20)
				{
					request(GetSensorAddressStoreData(), KWH_STORE_TYPE);
					lastUpdate = TimeLib::getCurrentTime();
					wait(200);
				}
				break;

			case SENSORSTATUS::WaitingCALIBRATION:
				if (TimeLib::secPassed(lastUpdate) > 5)
				{
					request(GetSensorAddressStoreData(), CALIBRATION_STORE_TYPE);
					lastUpdate = TimeLib::getCurrentTime();
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





void setup()
{

}

void receive(const MyMessage &message)
{
	Serial.print("Received message Sensor:");
	Serial.print(message.sensor);
	Serial.print(" Command:");
	Serial.print(message.getCommand());
	Serial.print(" IsAck:");
	Serial.print(message.isAck());
	Serial.print(" Type:");
	Serial.println(message.type);

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
	for (int x = 0; x < NUMBER_OF_SENSORS; x++)
	{
		sensors[x].run();
	}
}

