#ifndef INA219_H
#define INA219_H

#include <stdint.h>

typedef struct {
    uint8_t id;
    float current;
    float voltage;
    float power;
    float shuntVoltage;
} ina219_data_t;

/**
 * @brief:	The goal of this function is to read the value of a single INA sensor .
 * @param 	nodeIndex Index to the specific INA Zephyr device-tree node you want to read
 * @param   inaInstance Pointer to the struct where the INA's data is stored after being read
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readSingleINA(int nodeIndex, ina219_data_t *inaInstance);

/**
 * @brief:	The goal of this function is to read all the EPS INA's and store them in a data buffer to be used for a heartbeat. 
 * @param   inaStorage - Data buffer to store the data of all the INA's
 * 
 * @return  Returns a 1 if the method was able to read and store the data
 */
int readAllINA(ina219_data_t inaSensors[]);

#endif