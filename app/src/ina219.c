#include "ina219.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h> 
#include <zephyr/sys/util.h>


#define DT_DRV_COMPAT ti_ina219

#define GET_SENSOR_DEVICE(inst) DEVICE_DT_INST_GET(inst),

static const struct device *const inaNodes[] = {
    DT_INST_FOREACH_STATUS_OKAY(GET_SENSOR_DEVICE)
};


int readSingleINA(int nodeIndex, ina219_data_t inaInstance){
	int init = sensor_sample_fetch(inaNodes[nodeIndex]);
	if (init) {
		printf("Could not fetch sensor data.\n");
		return 0;
	}

	struct sensor_value tempVoltage;
	struct sensor_value tempVshunt;
	struct sensor_value tempCurrent;
	struct sensor_value tempPower;

	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_VOLTAGE, &tempVoltage);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_VSHUNT, &tempVshunt);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_POWER, &tempCurrent);
	sensor_channel_get(inaNodes[nodeIndex], SENSOR_CHAN_CURRENT, &tempPower);

    inaInstance.voltage = sensor_value_to_double(&tempVoltage);
    inaInstance.shuntVoltage = sensor_value_to_double(&tempVshunt);
	inaInstance.current = sensor_value_to_double(&tempCurrent);
	inaInstance.power = sensor_value_to_double(&tempPower);
	
	return 1;
}


int readAllINA(ina219_data_t inaSensors[]){
	for(int i = 0; i < 9; i++){
		readSingleINA(i, inaSensors[i]);
		inaSensors[i].id = (uint8_t)i;
	}
	
	return 1;
}