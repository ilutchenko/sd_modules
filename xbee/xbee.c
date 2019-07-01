/*
 * xbee.c
 *
 *  Created on: Mar 23, 2019
 *      Author: a-h
 */
#include "xbee.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
extern thread_reference_t xbee_trp;
extern thread_reference_t xbee_poll_trp;
extern uint8_t payload[];
extern struct ch_semaphore usart1_semaph;
xbee_struct_t xbee_struct;
xbee_struct_t *xbee = &xbee_struct;
tx_box_t tx_struct;
tx_box_t *tx_box = &tx_struct;


void xbee_read(SPIDriver *SPID, uint8_t rxlen, uint8_t *at_msg, uint8_t *rxbuff){
		uint8_t len;
		uint8_t txbuff[20];
		memset(txbuff, 0x00, 20);
		//chprintf((BaseSequentialStream*)&SD1, "Reading %s command \n\r", at_msg);
		chThdSleepMilliseconds(10);
		len = xbee_create_at_read_message(at_msg, txbuff);
	    /*chprintf((BaseSequentialStream*)&SD1, "SPI ");
	    for (i = 0; i < len; i++){
	    	chprintf((BaseSequentialStream*)&SD1, "%x ", txbuff[i]);
	    }
	    chprintf((BaseSequentialStream*)&SD1, "\n\r");
*/
	    xbee_send(SPID, txbuff, len);
		chThdSleepMilliseconds(10);
		xbee_receive(SPID, rxlen, rxbuff);
		spiReleaseBus(SPID); // Ownership release.
}


void xbee_write(BaseSequentialStream* chp, int argc, char* argv[]){
	if (argc == 3){
		uint8_t len;
		uint8_t txbuffer[50];
		char *at = argv[1];
		uint32_t var = atoi(argv[2]);
		chprintf(chp, "Write %s %x command \n\r", at, var);
		len = xbee_create_at_read_message((uint8_t*)argv[2], &txbuffer[0]);

		xbee_send(&SPID1, &txbuffer[0], len);
	}else{
		chprintf(chp, "Usage: xbee write <AT command> <value>\n\r \n\r");
	}
}

void xbee_attn(BaseSequentialStream* chp, int argc, char* argv[]){
	(void)argv;
	if (argc == 1){
	uint8_t stat = palReadLine(LINE_RF_868_SPI_ATTN);
	chprintf(chp, "ATIIN pin: %d \n\r", stat);
	}else{
		chprintf(chp, "Usage: xbee attn\n\r \n\r");
	}
}

uint8_t xbee_create_at_read_message(uint8_t *at, uint8_t *buffer){
	buffer[0] = 0x7E;	// Start delimiter
	buffer[1] = 0x00;	// Length MSB
	buffer[2] = 0x04;	// Length LSB
	buffer[3] = XBEE_AT_FRAME;	// Frame type - AT command
	buffer[4] = at[0];	// Frame ID - it will return back
	buffer[5] = at[0];
	buffer[6] = at[1];	// AT command - two symbols
	buffer[7] = xbee_calc_CRC(&buffer[3], 4);
	return 8;	// Return length of packet
}

uint8_t xbee_create_at_write_message(char *at, uint8_t *buffer, uint8_t *data, uint8_t num){
	uint8_t i = 0;
	buffer[0] = 0x7E;	// Start delimiter
	buffer[1] = 0x00;	// Length MSB
	buffer[2] = 0x04 + num;	// Length LSB
	buffer[3] = XBEE_AT_FRAME;	// Frame type - AT command
	buffer[4] = 0x52;	// Frame ID - it will return back
	buffer[5] = at[0];
	buffer[6] = at[1];	// AT command - two symbols
	for (i = 0; i < num; i++){
		buffer[7+i] = data[i];
	}
	buffer[7+num] = xbee_calc_CRC(&buffer[3], 4 + num);
	return 8 + num;		// Return length of packet
}

uint8_t xbee_create_data_read_message(uint8_t *at, uint8_t *buffer){
	buffer[0] = 0x7E;	// Start delimiter
	buffer[1] = 0x00;	// Length MSB
	buffer[2] = 0x04;	// Length LSB
	buffer[3] = XBEE_TRANSMIT_REQ_FRAME;	// Frame type - AT command
	buffer[4] = at[0];	// Frame ID - it will return back
	buffer[5] = at[0];
	buffer[6] = at[1];	// AT command - two symbols
	buffer[7] = xbee_calc_CRC(&buffer[3], 4);
	return 8;	// Return length of packet
}

uint8_t xbee_create_data_write_message(uint8_t *buffer, uint8_t *data, uint8_t num){
	uint8_t i = 0;
	buffer[0] = 0x7E;	// Start delimiter
	buffer[1] = (num + 14) << 8;	// Length MSB
	buffer[2] = (num + 14) & 0xFF;	// Length LSB
	buffer[3] = XBEE_TRANSMIT_REQ_FRAME;	// Frame type - AT command
	buffer[4] = 'T';	// Frame ID - it will return back
	buffer[5] = (uint8_t)(xbee->dest_addr_h >> 24);	// Destination address MSB
	buffer[6] = (uint8_t)(xbee->dest_addr_h >> 16);	// Destination address MSB
	buffer[7] = (uint8_t)(xbee->dest_addr_h >> 8);	// Destination address MSB
	buffer[8] = (uint8_t)(xbee->dest_addr_h);	// Destination address MSB
	buffer[9] = (uint8_t)(xbee->dest_addr_l >> 24);	// Destination address LSB
	buffer[10] = (uint8_t)(xbee->dest_addr_l >> 16);	// Destination address LSB
	buffer[11] = (uint8_t)(xbee->dest_addr_l >> 8);	// Destination address LSB
	buffer[12] = (uint8_t)(xbee->dest_addr_l);	// Destination address LSB
	buffer[13] = 0xFF;				// Reserved, 0xff required
	buffer[14] = 0xFE;				// Reserved, 0xfe required
	buffer[15] = 0;					// Broadcast radius (num of mesh hops)
	buffer[16] = 1 << 6;			// Delivery method (point-multipoint now)
	// Payload copying
	for (i = 0; i < num; i++){
		buffer[17+i] = data[i];
	}
	buffer[17+num] = xbee_calc_CRC(&buffer[3], 14 + num);
	return 17 + num + 1;		// Return length of packet
}

void xbee_receive(SPIDriver *SPID, uint8_t len, uint8_t *rxbuf){
	uint8_t txbuf[len];
	memset(txbuf, 0xff, len);
	spiAcquireBus(SPID);              	/* Acquire ownership of the bus.    */
	palClearLine(LINE_RF_868_CS);
	chThdSleepMilliseconds(1);
	spiExchange(SPID, len, txbuf, rxbuf); // Atomic transfer operations.
	spiReleaseBus(SPID); 				/* Ownership release.               */
	palSetLine(LINE_RF_868_CS);
	//chThdSleepMilliseconds(1);
}

void xbee_send(SPIDriver *SPID, uint8_t *txbuf, uint8_t len){
	//palSetLine(LINE_RED_LED);
	spiAcquireBus(SPID);              	/* Acquire ownership of the bus.    */
	palClearLine(LINE_RF_868_CS);
	chThdSleepMilliseconds(1);
	spiSend(SPID, len, txbuf);
	//spiExchange(SPID, 8, rxbuf, txbuf); 			/* Atomic transfer operations.      */
	spiReleaseBus(SPID); 				/* Ownership release.               */
	palSetLine(LINE_RF_868_CS);
	//chThdSleepMilliseconds(1);
	//palClearLine(LINE_RED_LED);
}

void xbee_read_no_cs(SPIDriver *SPID, uint8_t len, uint8_t *rxbuff){
	uint8_t txbuff[len];
	memset(txbuff, 0xFF, len);
	spiAcquireBus(SPID);              	/* Acquire ownership of the bus.    */

	if(palReadLine(LINE_RF_868_CS))
	{
		palClearLine(LINE_RF_868_CS);
		chThdSleepMilliseconds(1);
	}else{
		palClearLine(LINE_RF_868_CS);
	}

	//chThdSleepMilliseconds(1);
	//spiSend(SPID, len, txbuf);
	spiExchange(SPID, len, txbuff, rxbuff); 			/* Atomic transfer operations.      */
	spiReleaseBus(SPID); 				/* Ownership release.               */

/*	chSemWait(&usart1_semaph);
	chprintf((BaseSequentialStream*)&SD1, "polled1: %x\r\n", rxbuff[0]);
	chSemSignal(&usart1_semaph);
*/
	//palSetLine(LINE_RF_868_CS);
	//chThdSleepMilliseconds(1);
}

void xbee_read_release_cs(SPIDriver *SPID, uint8_t len, uint8_t *rxbuff){
	uint8_t txbuff[len];
	memset(txbuff, 0xFF, len);
	spiAcquireBus(SPID);              	/* Acquire ownership of the bus.    */
	palClearLine(LINE_RF_868_CS);
	//chThdSleepMilliseconds(1);
	//spiSend(SPID, len, txbuf);
	spiExchange(SPID, len, txbuff, rxbuff); 			/* Atomic transfer operations.      */
	spiReleaseBus(SPID); 				/* Ownership release.               */
	palSetLine(LINE_RF_868_CS);
	chThdSleepMilliseconds(1);
}

uint8_t xbee_check_attn(void){
	uint8_t i = 0;
	uint8_t rxbuff[50];
	uint8_t txbuf[50];
	txbuf[0] = 0xFF;
	if (!palReadLine(LINE_RF_868_SPI_ATTN)){
				//palSetLine(LINE_GREEN_LED); // LED ON.
				spiAcquireBus(&SPID1);              // Acquire ownership of the bus.
				palClearLine(LINE_RF_868_CS);
				chThdSleepMilliseconds(1);
				while(!palReadLine(LINE_RF_868_SPI_ATTN)){
					spiExchange(&SPID1, 1, txbuf, &rxbuff[i++]); // Atomic transfer operations.
					spiReleaseBus(&SPID1); // Ownership release.
					chThdSleepMilliseconds(1);
					//palClearLine(LINE_GREEN_LED); // LED OFF
				}
				palSetLine(LINE_RF_868_CS);
				chThdSleepMilliseconds(1);
				return 1;
			}else{
				i = 0;
				return 0;
			}
}

uint8_t xbee_calc_CRC(uint8_t *buffer, uint8_t num){
	uint8_t i;
	uint16_t sum = 0;
	for (i = 0; i < num; i++){
		sum += buffer[i];
	}
	sum &= 0xFF;
	sum = (uint8_t)(0xFF - sum);
	return (uint8_t)sum;
}


/*=============================================================
 *
 * Xbee thread wakeup functions - callbacks for shell commands
 *
 ==============================================================*/
void xbee_get_addr(void){
	xbee->suspend_state = 0;
	//chSysLock();
	chThdResume(&xbee_trp, (msg_t)XBEE_GET_OWN_ADDR);  /* Resuming the thread with message.*/
	//chSysUnlock();
}

void xbee_get_rssi(void){
	//chSysLockFromISR();
	chThdResume(&xbee_trp, (msg_t)XBEE_GET_RSSI);  /* Resuming the thread with message.*/
	//chSysUnlockFromISR();
}

void xbee_get_stat(void){
	//chSysLockFromISR();
	chThdResume(&xbee_trp, (msg_t)XBEE_GET_STAT);  /* Resuming the thread with message.*/
	//chSysUnlockFromISR();
}

void xbee_get_lb_status(void){
	chprintf((BaseSequentialStream*)&SD1, "XBee loopback state: %d\r\n", xbee->loopback_mode);
}

void xbee_set_loopback(char* argv[]){
	if (strcmp(argv[1], "on") == 0){
		xbee->loopback_mode = true;
	}else if (strcmp(argv[1], "off") == 0){
		xbee->loopback_mode = false;
	}else{
		chprintf((BaseSequentialStream*)&SD1, "Usage: xbee lb <on|off>\n\r");
	}
}

void xbee_get_ping(void){
	//chSysLockFromISR();
	chThdResume(&xbee_trp, (msg_t)XBEE_GET_PING);  /* Resuming the thread with message.*/
	//chSysUnlockFromISR();
}

void xbee_get_channels(void){
	//chSysLockFromISR();
	chThdResume(&xbee_trp, (msg_t)XBEE_GET_CHANNELS);  /* Resuming the thread with message.*/
	//chSysUnlockFromISR();
}


void xbee_thread_execute(uint8_t command){
	xbee->suspend_state = 0;
	  chSysLock();
	  chThdResume(&xbee_trp, command);  /* Resuming the thread with message.*/
	  chSysUnlock();
}
/*========================================================
 *
 * Xbee communicating funcs - used by thread or standalone
 *
 =========================================================*/

void xbee_read_own_addr(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	uint32_t address_l, address_h;
	xbee_read(&SPID1, 8+6, (uint8_t*)"SL", packet);
	/*uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "ADDR_L ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r"); */
	address_l = packet[8] << 24 | packet[9] << 16 | packet[10] << 8 | packet[11];
	xbee_read(&SPID1, 8+6, (uint8_t*)"SH", packet);
	/*	chprintf((BaseSequentialStream*)&SD1, "ADDR_H ");
			    for (i = 0; i < 15; i++){
			    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
			    }
			    chprintf((BaseSequentialStream*)&SD1, "\n\r"); */
	address_h = packet[8] << 24 | packet[9] << 16 | packet[10] << 8 | packet[11];
	xbee_str->own_addr_h = address_h;
	xbee_str->own_addr_l = address_l;
	chSemWait(&usart1_semaph);
	chprintf((BaseSequentialStream*)&SD1, "ADDR is %x%x\n\r", address_h, address_l);
	chSemSignal(&usart1_semaph);
}

uint16_t xbee_get_attn_pin_cfg(xbee_struct_t *xbee_str){
	uint8_t packet[15];
	(void)xbee_str;
	xbee_read(&SPID1, 8+6, (uint8_t*)"P9", packet);
	/*uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "RSSI ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
    return (packet[7] << 8) | packet[8];
}

uint16_t xbee_read_last_rssi(xbee_struct_t *xbee_str){
	uint8_t len;
		uint8_t txbuffer[20];
		uint8_t i;
		uint8_t zero_byte = 0;
			len = xbee_create_at_read_message("DB", &txbuffer[0]);
			/*chSemWait(&usart1_semaph);
							chprintf((BaseSequentialStream*)&SD1, "Write DB %d command \n\r");
										    for (i = 0; i < len; i++){
										    	chprintf((BaseSequentialStream*)&SD1, "%x ", txbuffer[i]);
										    }
										    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
						chSemSignal(&usart1_semaph);*/
		xbee_send(&SPID1, &txbuffer[0], len);

 return 0;
}

uint32_t xbee_read_channels(xbee_struct_t *xbee_str){
	uint8_t packet[17];
	(void)xbee_str;
	xbee_read(&SPID1, 8+8, (uint8_t*)"CM", packet);
	uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "RSSI ");
		    for (i = 0; i < 17; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
    return (packet[7] << 24) | (packet[8] << 16) | (packet[9] << 8) | packet[10];
}

uint16_t xbee_read_baudrate(xbee_struct_t *xbee_str){
	uint8_t packet[15];
	(void)xbee_str;
	xbee_read(&SPID1, 8+6, (uint8_t*)"BR", packet);
	uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "Baudrate ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
    return (packet[7] << 8) | packet[8];
}

uint16_t xbee_get_packet_payload(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"NP", packet);
	/*uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "Packet payload: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->packet_payload = ((packet[8] << 8) | packet[9]);
	return ((packet[8] << 8) | packet[9]);
}

uint16_t xbee_get_bytes_transmitted(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"BC", packet);
	/*uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "BC: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->bytes_transmitted = ((packet[8] << 8) | packet[9]);
	return ((packet[7] << 8) | packet[8]);
}

uint16_t xbee_get_good_packets_res(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"GD", packet);
	/*
	 uint8_t i;
	chprintf((BaseSequentialStream*)&SD1, "GD: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->good_packs_res = ((packet[8] << 8) | packet[9]);
	return ((packet[8] << 8) | packet[9]);
}

uint16_t xbee_get_received_err_count(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"ER", packet);
	/*
	  uint8_t i;
	  chprintf((BaseSequentialStream*)&SD1, "ER: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->rec_err_count = ((packet[8] << 8) | packet[9]);
	return ((packet[8] << 8) | packet[9]);
}

uint16_t xbee_get_transceived_err_count(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"TR", packet);
/*
 	 uint8_t i;
 	 chprintf((BaseSequentialStream*)&SD1, "TR: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->trans_errs = ((packet[8] << 8) | packet[9]);
	return ((packet[8] << 8) | packet[9]);
}

uint16_t xbee_get_unicast_trans_count(xbee_struct_t *xbee_str){
	uint8_t packet[15];

	xbee_read(&SPID1, 8+6, (uint8_t*)"UA", packet);
	/*
	  uint8_t i;
	  chprintf((BaseSequentialStream*)&SD1, "UA: ");
		    for (i = 0; i < 15; i++){
		    	chprintf((BaseSequentialStream*)&SD1, "%x ", packet[i]);
		    }
		    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");*/
	xbee_str->unicast_trans_count = ((packet[8] << 8) | packet[9]);
	return ((packet[8] << 8) | packet[9]);
}

void xbee_send_ping_message(xbee_struct_t *xbee_strc){

	uint8_t buffer[64];
	uint8_t len;
	xbee_strc->dest_addr_h = 0x013A200;
	xbee_strc->dest_addr_l = 0x418856C9;
	//xbee_strc->dest_addr_h = 0x00;
	//xbee_strc->dest_addr_l = 0x0000FFFF;
	len = xbee_create_data_write_message(buffer, (uint8_t*)"HELLO", 5);
	uint8_t i;
	chSemWait(&usart1_semaph);
	chprintf((BaseSequentialStream*)&SD1, "addr_h: %x, addr_l: %x\r\n", xbee_strc->dest_addr_h, xbee_strc->dest_addr_l);
	chprintf((BaseSequentialStream*)&SD1, "TX: ");
			    for (i = 0; i < len; i++){
			    	chprintf((BaseSequentialStream*)&SD1, "%x ", buffer[i]);
			    }
			    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
	chSemSignal(&usart1_semaph);
	xbee_send(&SPID1, buffer, len);

}

void xbee_send_rf_message(xbee_struct_t *xbee_strc, uint8_t *buffer, uint8_t len){
	uint8_t txbuff[128];
	uint8_t pack_len;
	xbee_strc->dest_addr_h = 0x013A200;
	xbee_strc->dest_addr_l = 0x418856C9;
	//xbee_strc->dest_addr_h = 0x00;
	//xbee_strc->dest_addr_l = 0x0000FFFF;
	pack_len = xbee_create_data_write_message(txbuff, buffer, len);
/*	uint8_t i;
	chSemWait(&usart1_semaph);
	//chprintf((BaseSequentialStream*)&SD1, "addr_h: %x, addr_l: %x\r\n", xbee_strc->dest_addr_h, xbee_strc->dest_addr_l);
	chprintf((BaseSequentialStream*)&SD1, "TX: ");
	for (i = 0; i < pack_len; i++){
		chprintf((BaseSequentialStream*)&SD1, "%x ", txbuff[i]);
	}
	chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
	chSemSignal(&usart1_semaph); */
	if (xbee->tx_ready)
	{
		xbee_send(&SPID1, txbuff, pack_len);
		xbee->tx_ready = 0;
	}

}

void xbee_send_rf_message_back(xbee_struct_t *xbee_strc, uint8_t *buffer, uint8_t len){
	uint8_t txbuff[128];
	uint8_t pack_len;
	xbee_strc->dest_addr_h = 0x013A200;
	xbee_strc->dest_addr_l = 0x418856B2;
	//xbee_strc->dest_addr_h = 0x00;
	//xbee_strc->dest_addr_l = 0x0000FFFF;
	pack_len = xbee_create_data_write_message(txbuff, buffer, len);
/*	uint8_t i;
	chSemWait(&usart1_semaph);
	//chprintf((BaseSequentialStream*)&SD1, "addr_h: %x, addr_l: %x\r\n", xbee_strc->dest_addr_h, xbee_strc->dest_addr_l);
	chprintf((BaseSequentialStream*)&SD1, "TX: ");
	for (i = 0; i < pack_len; i++){
		chprintf((BaseSequentialStream*)&SD1, "%x ", txbuff[i]);
	}
	chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
	chSemSignal(&usart1_semaph); */
	while(!palReadLine(LINE_RF_868_SPI_ATTN)){
					xbee_polling();
				}
	if (xbee->tx_ready)
	{
		xbee_send(&SPID1, txbuff, pack_len);
		xbee->tx_ready = 0;
	}


}

void xbee_attn_event(void){
	chSysLockFromISR();
	chThdResumeI(&xbee_poll_trp, (msg_t)0x01);  /* Resuming the thread with message.*/
	chSysUnlockFromISR();
	//palToggleLine(LINE_RED_LED);
}


void xbee_polling(void){
	//const uint16_t pack_len = 4;
	uint8_t message[4];
	uint8_t i = 0;
	//chSemWait(&spi2_semaph);
	//chSemWait(&usart1_semaph);
	//	chprintf((BaseSequentialStream*)&SD1, "Xbee polling\r\n");
	//	chSemSignal(&usart1_semaph);
	while (i < 50){
		if(!palReadLine(LINE_RF_868_SPI_ATTN))
		{
			xbee_read_no_cs(&SPID1, 1, &message[0]);
			//chSemWait(&usart1_semaph);
			//chprintf((BaseSequentialStream*)&SD1, "polled1: %x\r\n", message[0]);
			//	chSemSignal(&usart1_semaph);
			if ((message[0] == 0x7E)){
				xbee_read_no_cs(&SPID1, 3, &message[1]);
				i = 50;
				/*chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "polled: %x %x %x %x\r\n", message[0], message[1], message[2], message[3]);
				chSemSignal(&usart1_semaph);*/
				switch(message[3]){
				case XBEE_AT_FRAME:
					xbee_process_at_frame(message);
					break;
				case XBEE_AT_QUEUE_FRAME:
					xbee_process_at_queue_frame(message);
					break;
				case XBEE_TRANSMIT_REQ_FRAME:
					xbee_process_tx_req_frame(message);
					break;
				case XBEE_EXPLICIT_ADDR_FRAME:
					xbee_process_explicit_frame(message);
					break;
				case XBEE_REMOTE_AT_FRAME:
					xbee_process_remote_at_frame(message);
					break;
				case XBEE_AT_RESPONSE_FRAME:
					xbee_process_at_response(message);
					break;
				case XBEE_MODEM_STAT_FRAME:
					xbee_process_modem_stat_frame(message);
					break;
				case XBEE_TRANSMIT_STAT_FRAME:
					xbee_process_tx_stat(message);
					break;
				case XBEE_ROUTE_INF_FRAME:
					xbee_process_route_inf_frame(message);
					break;
				case XBEE_AGGREGATE_ADDR_FRAME:
					xbee_process_aggregade_addr_frame(message);
					break;
				case XBEE_RECEIVE_PACKET_FRAME:
					xbee_process_receive_packet_frame(message);
					break;
				case XBEE_EXPLICIT_RX_FRAME:
					xbee_process_explicit_rx_frame(message);
					break;
				case XBEE_DATA_SAMPLE_FRAME:
					xbee_process_data_sample_frame(message);
					break;
				case XBEE_NODE_ID_FRAME:
					xbee_process_node_id_frame(message);
					break;
				case XBEE_REMOTE_RESPONSE_FRAME:
					xbee_process_remote_response_frame(message);
					break;
				default:
					return;
					break;
				}
				return;
			}else{
				i++;
			}
		}else{
			i = 50;
		}
	}
	//chSemSignal(&spi2_semaph);
}

void xbee_process_at_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee at frame id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}
void xbee_process_at_queue_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee at queue id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_tx_req_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee tx req frame id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_explicit_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee explicit frame id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_remote_at_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee remote at id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_at_response(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			/*chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee at response id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);*/
			if (rxbuff[1] == 'D' && rxbuff[2] == 'B')
			{
				xbee->rssi = rxbuff[3] << 8 | rxbuff[4];
			}
}

void xbee_process_modem_stat_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee modem stat id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_tx_stat(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
		/*	chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee tx stat id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
			*/
			xbee->tx_ready = 1;
}

void xbee_process_route_inf_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee route inf id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_aggregade_addr_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee aggregade addr id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_receive_packet_frame(uint8_t* buffer){

	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
	uint8_t rxbuff[RF_PACK_LEN + 1];
	uint8_t i;
	xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
	/*chSemWait(&usart1_semaph);
		chprintf((BaseSequentialStream*)&SD1, "Xbee received len %d \r\n", payload_len);
					    for (i = 0; i < payload_len; i++){
					    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
					    }
					    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
	chSemSignal(&usart1_semaph);*/
	xbee_parse_rf_packet(rxbuff);
	if (xbee->loopback_mode){
		//xbee_send_payoad
	}
	xbee_read_last_rssi(xbee);
	//chSemWait(&usart1_semaph);
	//chprintf((BaseSequentialStream*)&SD1, "RSSI: %d\r\n", xbee->rssi);
	//chSemSignal(&usart1_semaph);
}

void xbee_parse_rf_packet(uint8_t *rxbuff){
	uint8_t packet_type;
	packet_type = rxbuff[11];

	switch (packet_type){
	case RF_GPS_PACKET:
		xbee_parse_gps_packet(rxbuff);
		//xbee_parse_gps_packet_back(rxbuff);
		break;
	default:
		break;
	}
}

void xbee_parse_gps_packet_back(uint8_t *rxbuff){
	int32_t lat, lon;
	float flat, flon;
	uint16_t dist;
	uint8_t hour, min, sec, sat, speed;
	uint8_t databuff[16];

	tx_box->lat = rxbuff[12] << 24 | rxbuff[13] << 16 | rxbuff[14] << 8 | rxbuff[15];
	tx_box->lon = rxbuff[16] << 24 | rxbuff[17] << 16 | rxbuff[18] << 8 | rxbuff[19];
	flat = tx_box->lat / 10000000.0f;
	flon = tx_box->lon / 10000000.0f;
	tx_box->hour = rxbuff[20];
	tx_box->min = rxbuff[21];
	tx_box->sec = rxbuff[22];
	tx_box->sat = rxbuff[23];
	tx_box->dist = rxbuff[24] << 8 | rxbuff[25];
	tx_box->speed = rxbuff[26];

	//chSemWait(&usart1_semaph);
	//chprintf((BaseSequentialStream*)&SD1, "%f,%f,%d:%d:%d:%d,%d,%d,%d\r\n",
	//					flat, flon, tx_box->hour, tx_box->min, tx_box->sec, tx_box->sat, tx_box->dist, tx_box->speed, xbee->rssi);
	//chSemSignal(&usart1_semaph);

	databuff[0] = RF_GPS_PACKET;
			databuff[1] = (uint8_t)(tx_box->lat >> 24);
			databuff[2] = (uint8_t)(tx_box->lat >> 16 );
			databuff[3] = (uint8_t)(tx_box->lat >> 8);
			databuff[4] = (uint8_t)(tx_box->lat);
			//memcpy(&tx_box->lon, &databuff[4], sizeof(float));
			databuff[5] = (uint8_t)(tx_box->lon >> 24);
			databuff[6] = (uint8_t)(tx_box->lon >> 16);
			databuff[7] = (uint8_t)(tx_box->lon >> 8);
			databuff[8] = (uint8_t)(tx_box->lon);
			databuff[9] = tx_box->hour;
			databuff[10] = tx_box->min;
			databuff[11] = tx_box->sec;
			databuff[12] = tx_box->sat;
			databuff[13] = (uint8_t)(tx_box->dist >> 8);
			databuff[14] = (uint8_t)(tx_box->dist);
			databuff[15] = (uint8_t)(tx_box->speed);
			xbee_send_rf_message_back(xbee, databuff, 16);


}

void xbee_parse_gps_packet(uint8_t *rxbuff){
	int32_t lat, lon;
	float flat, flon, headMot, headVeh;
	uint16_t dist, headMot_int;
	int16_t yaw, pitch, roll;
	uint8_t hour, min, sec, sat, speed, bat;

	lat = rxbuff[12] << 24 | rxbuff[13] << 16 | rxbuff[14] << 8 | rxbuff[15];
	lon = rxbuff[16] << 24 | rxbuff[17] << 16 | rxbuff[18] << 8 | rxbuff[19];
	flat = lat / 10000000.0f;
	flon = lon / 10000000.0f;
	hour = rxbuff[20];
	min = rxbuff[21];
	sec = rxbuff[22];
	sat = rxbuff[23];
	dist = rxbuff[24] << 8 | rxbuff[25];
	speed = rxbuff[26];
	yaw = rxbuff[27] << 8 | rxbuff[28];
	pitch = rxbuff[29] << 8 | rxbuff[30];
	roll = rxbuff[31] << 8 | rxbuff[32];
	bat = rxbuff[33];

	headMot = rxbuff[34] << 24 | rxbuff[35] << 16 | rxbuff[36] << 8 | rxbuff[37];
	headVeh = rxbuff[38] << 24 | rxbuff[39] << 16 | rxbuff[40] << 8 | rxbuff[41];
	headMot_int = (uint16_t)(headMot / 100000);
	//json_create_message
	chSemWait(&usart1_semaph);
	/*chprintf((BaseSequentialStream*)&SD1, "%f,%f,%d:%d:%d:%d,%d,%d,%d\r\n",
						flat, flon, hour, min, sec, sat, dist, speed, xbee->rssi);*/
	chprintf((BaseSequentialStream*)&SD1, "\r\n{\"msg_type\":\"boats_data\",\r\n\t\t\"boat_1\":{\r\n\t\t\t");
	chprintf((BaseSequentialStream*)&SD1, "\"hour\":%d,\r\n\t\t\t", hour);
	chprintf((BaseSequentialStream*)&SD1, "\"min\":%d,\r\n\t\t\t", min);
	chprintf((BaseSequentialStream*)&SD1, "\"sec\":%d,\r\n\t\t\t", sec);
	chprintf((BaseSequentialStream*)&SD1, "\"lat\":%f,\r\n\t\t\t", flat);
	chprintf((BaseSequentialStream*)&SD1, "\"lon\":%f,\r\n\t\t\t", flon);
	chprintf((BaseSequentialStream*)&SD1, "\"speed\":%d,\r\n\t\t\t", speed);
	chprintf((BaseSequentialStream*)&SD1, "\"dist\":%d,\r\n\t\t\t", dist);
	chprintf((BaseSequentialStream*)&SD1, "\"yaw\":%d,\r\n\t\t\t", yaw);
	chprintf((BaseSequentialStream*)&SD1, "\"pitch\":%d,\r\n\t\t\t", pitch);
	chprintf((BaseSequentialStream*)&SD1, "\"roll\":%d,\r\n\t\t\t", roll);
	chprintf((BaseSequentialStream*)&SD1, "\"headMot\":%d,\r\n\t\t\t", headMot_int);
	chprintf((BaseSequentialStream*)&SD1, "\"sat\":%d,\r\n\t\t\t", sat);
	chprintf((BaseSequentialStream*)&SD1, "\"rssi\":%d,\r\n\t\t\t", xbee->rssi);
	chprintf((BaseSequentialStream*)&SD1, "\"bat\":0\r\n\t\t\t");
	chprintf((BaseSequentialStream*)&SD1, "}\r\n\t}");
	//chprintf((BaseSequentialStream*)&SD1, "");
	chSemSignal(&usart1_semaph);
}

void xbee_process_explicit_rx_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee explicit rx id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}

void xbee_process_data_sample_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee data sample id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}


void xbee_process_node_id_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
		uint8_t rxbuff[payload_len + 1];
		uint8_t i;
		xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
		chSemWait(&usart1_semaph);
			chprintf((BaseSequentialStream*)&SD1, "Xbee node id %d\r\n", payload_len);
						    for (i = 0; i < payload_len; i++){
						    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
						    }
						    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
		chSemSignal(&usart1_semaph);
		if (xbee->loopback_mode){
			//xbee_send_payoad
		}
}

void xbee_process_remote_response_frame(uint8_t* buffer){
	uint16_t payload_len = (buffer[1] << 8) | buffer[2];
			uint8_t rxbuff[payload_len + 1];
			uint8_t i;
			xbee_read_release_cs(&SPID1, payload_len + 1, rxbuff);
			chSemWait(&usart1_semaph);
				chprintf((BaseSequentialStream*)&SD1, "Xbee remote response id %d\r\n", payload_len);
							    for (i = 0; i < payload_len; i++){
							    	chprintf((BaseSequentialStream*)&SD1, "%x ", rxbuff[i]);
							    }
							    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
			chSemSignal(&usart1_semaph);
}


void xbee_set_10kbs_rate(void){
	uint8_t len;
	uint8_t txbuffer[20];
	uint8_t i;
	uint8_t zero_byte = 0;
		len = xbee_create_at_write_message("BR", &txbuffer[0], &zero_byte, 1);
		/*chSemWait(&usart1_semaph);
						chprintf((BaseSequentialStream*)&SD1, "Write BD %d command \n\r", zero_byte);
									    for (i = 0; i < len; i++){
									    	chprintf((BaseSequentialStream*)&SD1, "%x ", txbuffer[i]);
									    }
									    chprintf((BaseSequentialStream*)&SD1, "\n\r\n\r");
					chSemSignal(&usart1_semaph);*/
	xbee_send(&SPID1, &txbuffer[0], len);
}

void xbee_set_80kbs_rate(void){
	uint8_t len;
	uint8_t txbuffer[20];
	uint8_t true_byte = 1;
	chprintf((BaseSequentialStream*)&SD1, "Write BD %d command \n\r", true_byte);
		len = xbee_create_at_write_message("BR", &txbuffer[0], &true_byte, 1);
	xbee_send(&SPID1, &txbuffer[0], len);
}
