#ifndef SENSOR_UTIL_H
#define SENSOR_UTIL_H

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include "ntcle101.h" 
#include <stdint.h>

#define INA_MAIN DT_NODELABEL(ina219_0)
#define INA_TPS3_3V DT_NODELABEL(ina219_1)
#define INA_TPS5V DT_NODELABEL(ina219_2)
#define INA_SOLARA DT_NODELABEL(ina219_3)
#define INA_SOLARB DT_NODELABEL(ina219_4)
#define INA_5VRF DT_NODELABEL(ina219_5)
#define INA_12V DT_NODELABEL(ina219_6)
#define INA_MPPCA DT_NODELABEL(ina219_7)
#define INA_MPPCB DT_NODELABEL(ina219_8)


/**
 * @brief:	The goal of this function is to change the power mode of the EPS board. The buit in INA 
 * 			file may be enough, but I dont know for now. BAREONES CODE until flatsat/mcu provided for.
 * 			testing as well as more detailed requirements. All code is subject to change 
 * @param 	value - gets the struct of the value used to determine power state. value.val1 is the whole
 * 			number, while value.val2 (if used) is the decimal
 * @return 	enum PowerState - Returns a basic enum for now with the power levels listed in the Airtable
 */
// enum PowerState ChangePowerState(struct sensor_value value);


/**
 * @brief:	The goal of this function is to read the value of a single INA sensor and store it in four 16 bit variables.
 * @param 	ina Pointer to INA219 instance that we want to get our electrical data from. 
 * @param   inaBuffer Buffer to store the INA Data following the format: Voltage, VShunt, Current, Power
 * @param   indx The starting index of where our data will be stored in the INA buffer. Added to make readAllINA() more simple
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readSingleINA(const struct device *ina, int16_t inaBuffer[], uint8_t indx);

/**
 * @brief:	The goal of this function is to read all the EPS INA's and store them in a data buffer to be used for a heartbeat. 
 * @param   inaStorage - Data buffer to store the data of all the INA's
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readAllINA(int16_t inaStorage[]);


/**
 * @brief Read information from all the sensors (INAs and thermistors) and print the information in a readable format
 * 
 * @param inaStorage Data buffer to store the data of all the INA's
 * @param rawTempADC ADC value from the Battery Board thermistor, to be used for the getBattTemp function
 * @return int Returns 1 if no errors
 */
int getSensorData(int16_t inaStorage[], uint32_t rawTempADC);

#endif