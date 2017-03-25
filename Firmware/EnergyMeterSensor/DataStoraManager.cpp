// 
// 
// 

#include "DataStoraManager.h"
#include <EEPROMex.h>

DataStoraManager::DataStoraManager(uint8_t numberSensor):numSensors(numberSensor)
{
}

void DataStoraManager::init()
{
	uint8_t check1, check2;

	check1 = EEPROM.readByte(EEPROM_INIT_ADDRESS);
	check2 = EEPROM.readByte(EEPROM_INIT_ADDRESS + sizeof(uint8_t));

	if ((check1 + check2) != 0xFF)
	{
		setDefaultsCalibration();
	}
	else
	{
		Serial.println("DataStoraManager - Values Loaded from EEPROM!");
	}
}

void DataStoraManager::setDefaultsCalibration()
{
	Serial.println("DataStoraManager - Setting Daefaults!");
	for (int x = 0; x < numSensors; x++)
	{
		EEPROM.writeDouble(getAdressConfig(x), 18);
	}
	EEPROM.writeByte(EEPROM_INIT_ADDRESS, 0x55);
	EEPROM.writeByte(EEPROM_INIT_ADDRESS + sizeof(uint8_t), 0xFF - 0x55);
}


int DataStoraManager::getNumberOfConfig()
{
	return numSensors;
}

int DataStoraManager::getAdressConfig(uint8_t id)
{
	return EEPROM_INIT_ADDRESS + 2 + (sizeof(double) * id);
}


double DataStoraManager::getCalibration(int id)
{
	if(id >= numSensors)
	{
		Serial.println("DataStoraManager - get -InvalidCalibration");
		return 0;
	}
	return EEPROM.readDouble(getAdressConfig(id));
}

void DataStoraManager::setCalibration(int id, double newValue)
{
	if (id >= numSensors)
	{
		Serial.println("DataStoraManager - set -InvalidCalibration");
		return;
	}
	if(getCalibration(id) != newValue)
	{
		EEPROM.writeDouble(getAdressConfig(id), newValue);
	}
}
