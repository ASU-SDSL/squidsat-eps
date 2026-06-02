#ifndef INA219_UTIL_H
#define INA219_UTIL_H

#include <zephyr/drivers/sensor.h>
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
 * @param 	ina - Pointer to INA219 instance that we want to get our electrical data from. 
 * @param   voltage - Pointer to the signed voltage variable
 * @param   vshunt - Pointer to the signed vshunt variable
 * @param   current - Pointer to the signed current variable
 * @param   power - Pointer to the signed power variable
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readSingleINA(const struct device *ina, int16_t *voltage, int16_t *vshunt, int16_t *current, int16_t *power);

/**
 * @brief:	The goal of this function is to read all the EPS INA's and store them in a data buffer to be used for a heartbeat. 
 * @param   inaX - Pointer to INA instance instatiated in the .c file
 * @param   inaStorage - Data buffer to store the data of all the INA's
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readAllINA(const struct device *ina1, const struct device *ina2, const struct device *ina3,
                const struct device *ina4, const struct device *ina5, const struct device *ina6, 
                const struct device *ina7, const struct device *ina8, const struct device *ina9, int16_t inaStorage[]);

#endif