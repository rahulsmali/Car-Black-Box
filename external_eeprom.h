/* 
 * File:   external_eeprom.h
 * Author: DATTTA
 *
 * Created on 1 April, 2026, 9:18 AM
 */

#ifndef EXTERNAL_EEPROM_H
#define	EXTERNAL_EEPROM_H

#define SLAVE_READ_E		0xA1
#define SLAVE_WRITE_E		0xA0


void write_external_eeprom(unsigned char address1,  unsigned char data);
unsigned char read_external_eeprom(unsigned char address1);
void write_external_eeprom_string(unsigned char addr, char *str);



#endif	/* EXTERNAL_EEPROM_H */

