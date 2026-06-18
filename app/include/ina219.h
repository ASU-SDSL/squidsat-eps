#ifndef INA219_H
#define INA219_H

#include <stdint.h>

typedef struct {
    uint8_t id;
    float voltage;
    float shuntVoltage;
    float current;
    float power;
} ina219_data_t;

/**
 * @brief:	The goal of this function is to read the value of a single INA sensor and store it in four 16 bit variables.
 * @param 	ina Pointer to INA219 instance that we want to get our electrical data from. 
 * @param   inaBuffer Buffer to store the INA Data following the format: Voltage, VShunt, Current, Power
 * @param   indx The starting index of where our data will be stored in the INA buffer. Added to make readAllINA() more simple
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readSingleINA(int nodeIndex, ina219_data_t inaInstance);

/**
 * @brief:	The goal of this function is to read all the EPS INA's and store them in a data buffer to be used for a heartbeat. 
 * @param   inaStorage - Data buffer to store the data of all the INA's
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readAllINA(ina219_data_t inaSensors[]);

#endif