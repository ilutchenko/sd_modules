/*
 * nina-b3.c
 *
 *  Created on: Apr 26, 2019
 *      Author: a-h
 */

#include "nina-b3.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "stdint.h"
#include "math.h"
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#ifdef SD_MODULE_TRAINER
	ble_charac_t *thdg;
	ble_charac_t *rdr;
	ble_charac_t *twd;
	ble_charac_t *tws;
	ble_charac_t *twa;
	ble_charac_t *bs;
	ble_charac_t *twa_tg;
	ble_charac_t *bs_tg;
	ble_charac_t *hdg;
	ble_charac_t *heel;
	ble_charac_t *charac_array[NUM_OF_CHARACTS];
#endif

#ifdef SD_SENSOR_BOX_RUDDER

#endif

#ifdef SD_SENSOR_BOX_LOG

#endif

#ifdef SD_SENSOR_BOX_WIND

#endif

ble_t *ble;
static const SerialConfig nina_config =
		{ 115200, 0, USART_CR2_STOP1_BITS, 0 };

static THD_WORKING_AREA(ble_parsing_thread_wa, 4096);
static THD_FUNCTION(ble_parsing_thread, arg);
static THD_WORKING_AREA(ble_thread_wa, 4096);
static THD_FUNCTION(ble_thread, arg);

void start_ble_module(void){
	chThdCreateStatic(ble_parsing_thread_wa, sizeof(ble_parsing_thread_wa), NORMALPRIO + 1,
			ble_parsing_thread, NULL);
	chThdCreateStatic(ble_thread_wa, sizeof(ble_thread_wa), NORMALPRIO + 1,
				ble_thread, NULL);
}

/*
 * Thread to process data collection and filtering from NEO-M8P
 */

static THD_FUNCTION(ble_parsing_thread, arg) {
	(void) arg;
	uint8_t token;
	int8_t megastring[256];
	uint16_t i = 0;
	uint8_t str_flag = 0;
	chRegSetThreadName("BLE Parsing Thread");
	sdStart(&NINA_IF, &nina_config);
	//systime_t prev = chVTGetSystemTime(); // Current system time.
	while (true) {
		token = sdGet(&NINA_IF);
		megastring[i] = token;
		i++;
		chprintf((BaseSequentialStream*) &SD1, "%c", token);
		if (token == '+'){
			str_flag = 1;
		}else if ((token == '\n') && (str_flag == 1)){
			nina_parse_command(megastring);
			memset(megastring, 0, 256);
			i = 0;
			str_flag = 0;
		}
		if (i == 256){
			i = 0;
		}

	//	palToggleLine(LINE_ORANGE_LED);
		//prev = chThdSleepUntilWindowed(prev, prev + TIME_MS2I(500));
	}
}

static THD_FUNCTION(ble_thread, arg) {
	(void) arg;
	uint8_t token;
	chRegSetThreadName("BLE Conrol Thread");
	nina_fill_memory();

	//systime_t prev = chVTGetSystemTime(); // Current system time.
	while (true) {
		chThdSleepMilliseconds(1000);
	//	palToggleLine(LINE_ORANGE_LED);
		//prev = chThdSleepUntilWindowed(prev, prev + TIME_MS2I(500));
	}
}

uint8_t nina_parse_command(int8_t *strp) {
	uint8_t scan_res;
	uint8_t scanned_vals[32];
	uint32_t val;
	uint64_t addr;
	chprintf((BaseSequentialStream*) &SD1, "Parsing\r\n");
	scan_res = sscanf(strp, "+UBTGSN:%d,%d,%x\r", &scanned_vals[0], &scanned_vals[1],	&val);
	if (scan_res == 1) {

		return 1;
	}

	scan_res = sscanf(strp, "+UBTGCHA:%d,%d\r", &scanned_vals[0],
			&scanned_vals[1]);
	if (scan_res == 2) {
		chprintf((BaseSequentialStream*) &SD1, "Scanned charac descript %d %d\r\n", scanned_vals[0], scanned_vals[1]);
		return 1;
	}

	scan_res = sscanf(strp, "+UBTGSER:%d,%d\r", &scanned_vals[0]);
	if (scan_res == 1) {

		return 1;
	}

	scan_res = sscanf(strp, "+UBTGCHA:%d,%d\r", &scanned_vals[0],
			&scanned_vals[1]);
	if (scan_res == 2) {

		return 1;
	}
	scan_res = sscanf(strp, "+UUBTACLC:%d,%d,%xr\r", &scanned_vals[0],
			&scanned_vals[1], &addr);
	if (scan_res == 3) {

		return 1;
	}
	scan_res = sscanf(strp, "+UUBTACLC:%d,%d,%xp\r", &scanned_vals[0],
			&scanned_vals[1], &addr);
	if (scan_res == 3) {
		chprintf((BaseSequentialStream*) &SD1, "Scanned connected dev %d %d %x\r\n", scanned_vals[0], scanned_vals[1], addr);
		return 1;
	}

	scan_res = sscanf(strp, "+UUBTACLD:%d\r", &scanned_vals[0]);
	if (scan_res == 1) {

		return 1;
	}

	scan_res = sscanf(strp, "+UUBTB:%xr,%d\r", &addr, &scanned_vals[0]);
	if (scan_res == 2) {

		return 1;
	}
	if (strstr(strp, "OK") != NULL){

		return 1;
	}
	if (strstr(strp, "ERROR") != NULL){
		return 1;
	}
	return -1;
}

void nina_fill_memory(void){
#ifdef SD_MODULE_TRAINER
	thdg = calloc(1, sizeof(ble_charac_t));
	rdr = calloc(1, sizeof(ble_charac_t));
	twd = calloc(1, sizeof(ble_charac_t));
	tws = calloc(1, sizeof(ble_charac_t));
	twa = calloc(1, sizeof(ble_charac_t));
	bs = calloc(1, sizeof(ble_charac_t));
	twa_tg = calloc(1, sizeof(ble_charac_t));
	bs_tg = calloc(1, sizeof(ble_charac_t));
	hdg = calloc(1, sizeof(ble_charac_t));
	heel = calloc(1, sizeof(ble_charac_t));

	charac_array[0] = thdg;
	charac_array[1] = rdr;
	charac_array[2] = twd;
	charac_array[3] = tws;
	charac_array[4] = twa;
	charac_array[5] = bs;
	charac_array[6] = twa_tg;
	charac_array[7] = bs_tg;
	charac_array[8] = hdg;
	charac_array[9] = heel;

#endif

#ifdef SD_SENSOR_BOX_RUDDER

#endif

#ifdef SD_SENSOR_BOX_LOG

#endif

#ifdef SD_SENSOR_BOX_WIND

#endif
}
void nina_send_at(void){
	chprintf(NINA_IFACE, "AT\r");
}

void nina_init_module(void){
	nina_init_services();
}

uint8_t nina_wait_response(int8_t *at_command){

return NINA_SUCCESS;
}

//AT+UBTGCHA=3A01,10,1,1,0F00FF,0,3
uint8_t nina_add_charac(ble_charac_t *charac, uint16_t uuid, uint8_t properties,
		uint8_t sec_read, uint8_t sec_write, uint32_t def_val,
		uint8_t read_auth, uint8_t max_len) {
	charac->max_lenth = max_len;
	charac->properties = properties;
	charac->read_auth = read_auth;
	charac->security_read = sec_read;
	charac->security_write = sec_write;
	charac->uuid = uuid;
	charac->value = def_val;
	chprintf(NINA_IFACE, "AT+UBTGCHA=%x,%d,%d,%d,%06x,%d,%d\r", uuid, properties,
			sec_read, sec_write, def_val, read_auth, max_len);
	//nina_wait_charac_handlers(charac);
}


void nina_wait_charac_handlers(ble_charac_t *charac){
	uint16_t val_handle;
	uint16_t cccd_handle;
	//parse
	charac->cccd_handle = cccd_handle;
	charac->value_handle = val_handle;
}

#ifdef SD_MODULE_TRAINER

uint8_t nina_init_services(void){
	chprintf(NINA_IFACE, "AT+UBTLE=3\r");
	if (nina_wait_response("+UBTLE\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(1000);
	chprintf(NINA_IFACE, "AT+UBTCFG=2,4\r");
	if (nina_wait_response("+UBTCFG\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+UBTLN=FastSkipper-COACH\r");
	if (nina_wait_response("+UBTLN\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(1000);
	// Create service for GATT server (send information to tablet) 16-bit
	//UUID = 3A00
	// Response send handle of the created service (int value).
	// +UBTGSER:SER_HAND
	chprintf(NINA_IFACE, "AT+UBTGSER=3A00\r");
	if (nina_wait_response("+UBTGSER\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	/*
# Create characteristic for service (values, witch coach complex send to tablet)
# Parametrs command:
# uuid (new, can be the same of service uuid),
# properties (notification support),
# security_read (no encryption required),
# security_write (no encryption required),
# initial value 1000,
# read_auth (Read Authorized. Any client can read data without host intervention)
# max_length (aximum length of the characteristic in bytes. The maximum value is 512 bytes)
*/
	//# THDG - uuid 3A01
	//# Response +UBTGCHA:THDG_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A01, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A01,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# RDR - uuid 3A02
	//# Response +UBTGCHA:RDR_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(rdr, 0x3A02, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A02,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# TWD - uuid 3A03
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A03, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A03,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# TWS - uuid 3A04
	//# Response +UBTGCHA:TWS_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(tws, 0x3A04, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A04,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# TWA - uuid 3A05
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(twa, 0x3A05, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A05,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# BS - uuid 3A06
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(bs, 0x3A06, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A06,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# TWA_TG - uuid 3A07
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A07, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A07,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# BS_TG - uuid 3A08
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A08, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A08,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# HDG - uuid 3A09
	//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A09, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
	chprintf(NINA_IFACE, "AT+UBTGCHA=3A09,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	*/
	chThdSleepMilliseconds(200);
	//# HEEL - uuid 3A0A
		//# Response +UBTGCHA:TWD_HAND,CCCD_HAND (zero if not use)
	nina_add_charac(thdg, 0x3A0A, 10, 1, 1, 0x0F00FF, 0, 3);
	/*
		chprintf(NINA_IFACE, "AT+UBTGCHA=3A0A,10,1,1,0F00FF,0,3\r");
		if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
			return -1;
		}
		*/
		chThdSleepMilliseconds(200);
		//# Save settings and reboot
		chprintf(NINA_IFACE, "AT&W\r");
		if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
		chprintf(NINA_IFACE, "AT+CPWROFF\r");
		if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
}
#endif

#ifdef SD_SENSOR_BOX_RUDDER
void nina_init_services(void){
	chprintf(NINA_IFACE, "AT+UBTLE=2\r");
	if (nina_wait_response("+UBTLE\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}

	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+UBTLN=FastSkipper-RUDDER\r");
	if (nina_wait_response("+UBTLN\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(1000);
	// Create service for GATT server (send information to tablet) 16-bit
	//UUID = 4A00
	// Response send handle of the created service (int value).
	// +UBTGSER:SER_HAND
	chprintf(NINA_IFACE, "AT+UBTGSER=5A00\r");
	if (nina_wait_response("+UBTGSER\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	/*
# Create characteristic for service (values, witch coach complex send to tablet)
# Parametrs command:
# uuid (new, can be the same of service uuid),
# properties (notification support),
# security_read (no encryption required),
# security_write (no encryption required),
# initial value 1000,
# read_auth (Read Authorized. Any client can read data without host intervention)
# max_length (aximum length of the characteristic in bytes. The maximum value is 512 bytes)
*/
	//# RDR - uuid 5A01
	//# Response +UBTGCHA:THDG_HAND,CCCD_HAND (zero if not use)
	chprintf(NINA_IFACE, "AT+UBTGCHA=5A01,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);

		//# Save settings and reboot
		chprintf(NINA_IFACE, "AT&W\r");
		if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
		chprintf(NINA_IFACE, "AT+CPWROFF\r");
		if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
}
#endif

#ifdef SD_SENSOR_BOX_LAG
void nina_init_services(void){
	chprintf(NINA_IFACE, "AT+UBTLE=2\r");
	if (nina_wait_response("+UBTLE\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}

	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+UBTLN=FastSkipper-LOG\r");
	if (nina_wait_response("+UBTLN\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT&W\r");
	if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	chprintf(NINA_IFACE, "AT+CPWROFF\r");
	if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(1000);
	// Create service for GATT server (send information to tablet) 16-bit
	//UUID = 4A00
	// Response send handle of the created service (int value).
	// +UBTGSER:SER_HAND
	chprintf(NINA_IFACE, "AT+UBTGSER=4A00\r");
	if (nina_wait_response("+UBTGSER\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);
	/*
# Create characteristic for service (values, witch coach complex send to tablet)
# Parametrs command:
# uuid (new, can be the same of service uuid),
# properties (notification support),
# security_read (no encryption required),
# security_write (no encryption required),
# initial value 1000,
# read_auth (Read Authorized. Any client can read data without host intervention)
# max_length (aximum length of the characteristic in bytes. The maximum value is 512 bytes)
*/
	//# LOG - uuid 4A01
	//# Response +UBTGCHA:THDG_HAND,CCCD_HAND (zero if not use)
	chprintf(NINA_IFACE, "AT+UBTGCHA=4A01,10,1,1,0F00FF,0,3\r");
	if (nina_wait_response("+UBTGCHA\r") != NINA_SUCCESS) {
		return -1;
	}
	chThdSleepMilliseconds(200);

		//# Save settings and reboot
		chprintf(NINA_IFACE, "AT&W\r");
		if (nina_wait_response("AT&W\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
		chprintf(NINA_IFACE, "AT+CPWROFF\r");
		if (nina_wait_response("+CPWROFF\r") != NINA_SUCCESS) {
			return -1;
		}
		chThdSleepMilliseconds(200);
}
#endif

#ifdef SD_SENSOR_BOX_WIND
void nina_init_services(void){

}
#endif
