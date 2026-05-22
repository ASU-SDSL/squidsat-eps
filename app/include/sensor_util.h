#ifndef INA219_UTIL_H
#define INA219_UTIL_H

#include <zephyr/drivers/sensor.h>
#include "ntcle101.h" 

#define INA_MAIN DT_NODELABEL(ina219_0)
// #define INA_TPS3_3V DT_NODELABEL(ina219_1)
// #define INA_TPS5V DT_NODELABEL(ina219_2)
// #define INA_SOLAR DT_NODELABEL(ina219_3)
// #define INA_MPPC DT_NODELABEL(ina219_4)
// #define INA_5VRF DT_NODELABEL(ina219_5)
// #define INA_5VRF DT_NODELABEL(ina219_6)

// typedef struct{
//     const struct device *ina;
//     struct sensor_value bus_voltage;
//     struct sensor_value current;
//     struct sensor_value power;
// }INA_Data;

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
 * @brief:	The goal of this function is to print out the sensor data for the sensors/information
 * 			that we have at the moment. Waiting for EPS Rev 1 to be finished, all code is subject
 * 			to change.
 * @param 	ina - Pointer to INA219 instance that we want to get our electrical data from. 
 * @param   type - Zephyr Sensor enum. For the INA all we need is SENSOR_CHAN_VOLTAGE, SENSOR_CHAN_CURRENT, and
 * 			SENSOR_CHAN_POWER. Shunt voltage can be obtained the same way if needed.
 * @param   val - struct used to store the values obtained from INA219 register. val.val1 is the whole number and
 *          val.val2 is the decimal.
 */
int GetSensorData(const struct device *ina, struct sensor_value voltage, struct sensor_value vshunt, struct sensor_value current, struct sensor_value power);

#endif