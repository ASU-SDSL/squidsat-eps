#include "ina219.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h> 
#include <zephyr/sys/util.h>

#define DT_DRV_COMPAT ti_ina219
#define GET_SENSOR_DEVICE(inst) DEVICE_DT_INST_GET(inst),

LOG_MODULE_REGISTER(ina219, LOG_LEVEL_INF);

/*	Orded by I2C addresses as of 5/12/26
	0 - Main INA
	1 - TPS3V
	2 - TPS5V
	3 - Solar A
	4 - Solar B
	5 - 5V RF
	6 - 12V
	7 - MPPC A
	8 - MPPC B
*/

// {voltage max, voltage min, current max, current min}
const inaRanges SAFE_RANGES[9] = {
	{8.4, 6.0, 12.0, 0.2},			// Main INA
	{3.6, 3.0, 3.0, 0.3},			// TPS 3.3V
	{5.5, 4.5, 3.0, 0.0},			// TPS 5V
	{6.0, 2.6, 2.0, 0.1},			// Solar A  ***PLACEHOLDER VALUES***
	{6.0, 2.6, 2.0, 0.1},			// Solar B  ***PLACEHOLDER VALUES***
	{4.9, 5.1, 3.0, 0.0},			// 5F RF
	{13.0, 11.0, 3.0, 0.0},			// 12V Reg
	{10.0, 8.0, 0.5, 0.0},			// MMPCA
	{10.0, 8.0, 0.5, 0.0}			// MPPCB
};

static const struct device *const inaNodes[] = {
    DT_INST_FOREACH_STATUS_OKAY(GET_SENSOR_DEVICE)
};

/*TODO - If we want to have the data as integers, look up fixed point arithmetic and casting since
 Zephyr sensors use a 32 bit buffer, but the INA only has 16 bits of information.
 Originally had 16 bit values, but reverted to doubles for simplicity, for now */
int readSingleINA(int nodeIndex, ina219_data_t *inaInstance){
	int init = sensor_sample_fetch(inaNodes[nodeIndex]);
	if (init) {
		LOG_WRN("Could not fetch sensor data.\n");
		return 0;
	}

	struct sensor_value tempCurrent;
	struct sensor_value tempVoltage;
	struct sensor_value tempPower;
	struct sensor_value tempVshunt;
	
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_CURRENT, &tempCurrent);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_VOLTAGE, &tempVoltage);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_POWER, &tempPower);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_VSHUNT, &tempVshunt);
	
	inaInstance->current = sensor_value_to_double(&tempCurrent);
    inaInstance->voltage = sensor_value_to_double(&tempVoltage);
	inaInstance->power = sensor_value_to_double(&tempPower);
    inaInstance->shuntVoltage = sensor_value_to_double(&tempVshunt);
	
	return 1;
}

int readAllINA(ina219_data_t inaSensors[]){
	for(int i = 0; i < 9; i++){
		readSingleINA(i, &inaSensors[i]);	// Read the sensor information
		inaSensors[i].id = (uint8_t)i; 	
			// TODO: Redundant, we dont need to assign ID's each time
		if ((inaSensors[i].voltage > SAFE_RANGES->voltageMax || inaSensors[i].voltage < SAFE_RANGES->voltageMin) ||
			(inaSensors[i].current > SAFE_RANGES->currentMax || inaSensors[i].current < SAFE_RANGES->currentMin)){
				LOG_ERR("INA %d Voltage or Current is beyond a safe range", i);
				// TODO: Shut off all payloads via load switches and power and send info to OBC/DANT
			}
	}
	
	return 1;
}