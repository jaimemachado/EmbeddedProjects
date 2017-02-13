#pragma once
#include <cstdint>
#define EERPON_SIZE 1024




class EEPROMWin
{
public:
	uint8_t eeprom_mem[EERPON_SIZE];
	uint8_t read(unsigned int address);
	void write(unsigned int address, uint8_t data);

};

extern EEPROMWin EEPROM;