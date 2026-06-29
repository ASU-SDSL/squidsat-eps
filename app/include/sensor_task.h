/**
 * @file sensor_task.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Code that is supposed to read the INA and tempurature sensors and print them out, as well as storing all the
 * INA information into a large uint16_t array
 * @version 1.3
 * 
 */

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "ntcle101.h" 
#include <stdint.h>


typedef struct {
    int16_t battTemp;
    int16_t inaInfo[36];
} heartbeat_telemetry_t;

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
 * @brief Read information from all the sensors (INAs and thermistors) and print the information in a readable format
 * 
 * @param inaStorage Data buffer to store the data of all the INA's
 * @param rawTempADC ADC value from the Battery Board thermistor, to be used for the getBattTemp function
 * @return int Returns 1 if no errors
 */
int getSensorData(int16_t inaStorage[], uint32_t rawTempADC);

/**
 * @brief Retrieve information from the sensors on the EPS and check for low/high temp, low power, etc.
 * 
 */
void sensor_task();

#endif