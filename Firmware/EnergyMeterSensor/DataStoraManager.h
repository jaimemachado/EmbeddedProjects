// DataStoraManager.h


#ifndef _CONFIGMANAGER_h
#define _CONFIGMANAGER_h

#define EEPROM_INIT_ADDRESS 300

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif



class DataStoraManager
{
	//double DEFAULT_CALIBRI_VALUES[NUMBER_OF_SENSORS] = { 51.28, 20, 20, 20, 11.1111, 11.1111, 11.1111, 11.1111 };

	//ConfigMangerRecordData config;
	uint8_t numSensors;

public:
	DataStoraManager(uint8_t numSensor);

	void init();
	void setDefaultsCalibration();
	int getNumberOfConfig();

	int getAdressConfig(uint8_t id);

	double getCalibration(int id);
	void setCalibration(int id, double newValue);
};


#endif

