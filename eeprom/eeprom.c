/*
 * mpu9250.c
 *
 *  Created on: Mar 14, 2019
 *      Author: a-h
 */

#include "eeprom.h"
#include "config.h"
extern struct ch_semaphore usart1_semaph;




void eeprom_write_hw_version(void) {
	uint8_t txbuff[3];
	msg_t status;
	txbuff[0] = EEPROM_HW_VER_ADDR >> 8;
	txbuff[1] = EEPROM_HW_VER_ADDR & 0xFF;
	txbuff[2] = 1;
	i2cAcquireBus(&I2CD1);
	status = i2cMasterTransmitTimeout(&I2CD1, EEPROM_ADDRESS, txbuff, 3, NULL,
			0, 1000);
	i2cReleaseBus(&I2CD1);
	if (status != MSG_OK) {
		chSemWait(&usart1_semaph);
		chprintf((BaseSequentialStream*) &SD1,
				"Shit happened: status is %d\r\n", i2cGetErrors(&I2CD1));
		chSemSignal(&usart1_semaph);
	}
}

void eeprom_check_i2c_bus(void) {
	uint8_t txbuff[3];
	uint8_t addr = 0;
	msg_t status;
	txbuff[0] = EEPROM_HW_VER_ADDR >> 8;
	txbuff[1] = EEPROM_HW_VER_ADDR & 0xFF;
	txbuff[2] = 1;
	while (1) {
		i2cAcquireBus(&I2CD1);
		status = i2cMasterTransmitTimeout(&I2CD1, EEPROM_ADDRESS, txbuff, 3, NULL, 0,
				1000);
		i2cReleaseBus(&I2CD1);
		if (status != MSG_OK) {
			chSemWait(&usart1_semaph);
			chprintf((BaseSequentialStream*) &SD1,
					"Shit happened: status is %d\r\n", i2cGetErrors(&I2CD1));
			chSemSignal(&usart1_semaph);
		} else {
			chSemWait(&usart1_semaph);
			chprintf((BaseSequentialStream*) &SD1, "Asked %d\r\n", EEPROM_ADDRESS);
			chSemSignal(&usart1_semaph);
		}
	}
}

void eeprom_read_hw_version(void) {
	uint8_t txbuff[2];
	uint8_t rxbuff[1];
	msg_t status;
	txbuff[0] = EEPROM_HW_VER_ADDR >> 8;
	txbuff[1] = EEPROM_HW_VER_ADDR & 0xFF;
	i2cAcquireBus(&I2CD1);
	status = i2cMasterTransmitTimeout(&I2CD1, EEPROM_ADDRESS, txbuff, 2, rxbuff,
			1, 1000);
	i2cReleaseBus(&I2CD1);
	if (status != MSG_OK) {
		chSemWait(&usart1_semaph);
		chprintf((BaseSequentialStream*) &SD1,
				"Shit happened: status is %d\r\n", i2cGetErrors(&I2CD1));
		chSemSignal(&usart1_semaph);
	}
	chSemWait(&usart1_semaph);
			chprintf((BaseSequentialStream*) &SD1,
					"Readed from EEPROM %d\r\n", rxbuff[0]);
			chSemSignal(&usart1_semaph);
}

void bno055_read_id(void) {
	uint8_t txbuff[2];
	uint8_t rxbuff[1];
	msg_t status;
	txbuff[0] = BNO055_CHIP_ID_ADDR;
	//txbuff[1] = EEPROM_HW_VER_ADDR & 0xFF;
	status = i2cMasterTransmitTimeout(&I2CD1, BNO055_ADDRESS, txbuff, 1, rxbuff,
			1, 1000);
	if (status != MSG_OK) {
		chSemWait(&usart1_semaph);
		chprintf((BaseSequentialStream*) &SD1,
				"Shit happened: status is %d\r\n", i2cGetErrors(&I2CD1));
		chSemSignal(&usart1_semaph);
	}
	chSemWait(&usart1_semaph);
	chprintf((BaseSequentialStream*) &SD1, "CHIP_ID from BNO055: %d\r\n",
			rxbuff[0]);
	chSemSignal(&usart1_semaph);
}
