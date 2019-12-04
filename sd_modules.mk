# List of all the board related files.
#Debug shell - often used on USART1 to debug firmware
ifeq ($(USE_DEBUG_SHELL),)
SD_SRC += ./sd_modules/dbg_shell_cmd/dbg_shell_cmd.c
SD_INC += ./sd_modules/dbg_shell_cmd
endif

#SailData shell - often used on USART1 to communicate with PC
ifeq ($(USE_SD_SHELL), TRUE)
SD_SRC += ./sd_modules/sd_shell_cmd/sd_shell_cmd.c
SD_INC += ./sd_modules/sd_shell_cmd
endif

#Service mode - manual configuration via shell
ifeq ($(USE_SERVICE_MODE), TRUE)
SD_SRC += ./sd_modules/sd_shell_cmd/service_mode.c
SD_INC += ./sd_modules/sd_shell_cmd
endif

#MPU9250 - 9-axis accel/gyro/magn chip
ifeq ($(USE_MPU_9250_MODULE), TRUE)
SD_SRC += ./sd_modules/mpu9250/mpu9250.c
#SD_SRC += ./sd_modules/mpu9250/MadgwickAHRS.c
#SD_SRC += ./sd_modules/mpu9250/quaternionFilters.c
SD_INC += ./sd_modules/mpu9250
endif

#HMC5883 - 3- axis magnetoresistive chip
ifeq ($(USE_HMC5883_MODULE), TRUE)
SD_SRC += ./sd_modules/hmc5883/hmc5883_i2c.c
SD_INC += ./sd_modules/hmc5883
endif

#HMC6343 - 6 - axis magnetoresistive chip
ifeq ($(USE_HMC6343_MODULE), TRUE)
SD_SRC += ./sd_modules/hmc6343/hmc6343_i2c.c
SD_INC += ./sd_modules/hmc6343
endif

#UBLOX NEO-M8 GPS modules series
ifeq ($(USE_UBLOX_GPS_MODULE), TRUE)
SD_SRC += ./sd_modules/neo-m8/neo-m8.c
SD_SRC += ./sd_modules/neo-m8/minmea.c
SD_SRC += ./sd_modules/neo-m8/neo_ubx.c
SD_INC += ./sd_modules/neo-m8
endif

#DIGI XBee modules
ifeq ($(USE_XBEE_868_MODULE), TRUE)
SD_SRC += ./sd_modules/xbee/xbee.c
SD_INC += ./sd_modules/xbee
endif

#EEPROM
ifeq ($(USE_EEPROM_MODULE), TRUE)
SD_SRC += ./sd_modules/eeprom/eeprom.c
SD_INC += ./sd_modules/eeprom
endif

#BLE
ifeq ($(USE_BLE_MODULE), TRUE)
SD_SRC += ./sd_modules/nina-b3/nina-b3.c
SD_INC += ./sd_modules/nina-b3
endif

#BNO055
ifeq ($(USE_BNO055_MODULE), TRUE)
SD_SRC += ./sd_modules/bno055/bno055.c
SD_SRC += ./sd_modules/bno055/bno055_i2c.c
SD_INC += ./sd_modules/bno055
endif

#BMX160
ifeq ($(USE_BMX160_MODULE), TRUE)
SD_SRC += ./sd_modules/bmx160/bmx160_i2c.c
SD_SRC += ./sd_modules/bmx160/bmi160.c
SD_SRC += ./sd_modules/bmx160/bmm150.c
SD_SRC += ./sd_modules/bmx160/MadgwickAHRS/MadgwickAHRS.c
#SD_SRC += ./sd_modules/bmx160/MahonyAHRS/MahonyAHRS.c
SD_INC += ./sd_modules/bmx160/bsx_lite/Inc
SD_INC += ./sd_modules/bmx160/MadgwickAHRS
#SD_INC += ./sd_modules/bmx160/MahonyAHRS
SD_INC += ./sd_modules/bmx160
endif

#LAG
ifeq ($(USE_LAG_MODULE), TRUE)
SD_SRC += ./sd_modules/lag/lag.c
SD_INC += ./sd_modules/lag
endif

#FATFS
ifeq ($(USE_MICROSD_MODULE), TRUE)
#SD_SRC += ./sd_modules/microsd/diskio.c
#SD_SRC += ./sd_modules/microsd/ff.c
#SD_SRC += ./sd_modules/microsd/ffsystem.c
#SD_SRC += ./sd_modules/microsd/ffunicode.c
SD_SRC += ./sd_modules/microsd/microsd.c
SD_INC += ./sd_modules/microsd
endif

ifeq ($(USE_MATH_MODULE), TRUE)
SD_SRC += ./sd_modules/math/sailDataMath.c
SD_SRC += ./sd_modules/math/sd_math.c
SD_INC += ./sd_modules/math
endif

#WIND sensor
ifeq ($(USE_WINDSENSOR_MODULE), TRUE)
SD_SRC += ./sd_modules/windsensor/windsensor.c
SD_INC += ./sd_modules/windsensor
endif

#ADC module
ifeq (${USE_ADC_MODULE}, TRUE)
SD_SRC += ./sd_modules/adc/adc.c
SD_INC += ./sd_modules/adc
endif

#BQ2560x charging module
ifeq (${USE_CHARGER_MODULE}, TRUE)
SD_SRC += ./sd_modules/bq2560x/bq2560x.c
SD_INC += ./sd_modules/bq2560x
endif
# Shared variables
ALLCSRC += $(SD_SRC)
ALLINC  += $(SD_INC)
