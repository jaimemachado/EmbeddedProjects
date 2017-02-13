#include "EEPROMWin.h"

EEPROMWin EEPROM;

uint8_t EEPROMWin::read(unsigned address)
{
	if (address >= EERPON_SIZE)
		return 0;
	return eeprom_mem[address];
}

void EEPROMWin::write(unsigned address, uint8_t data)
{
	if (address >= EERPON_SIZE)
		return;
	eeprom_mem[address] = data;
}
