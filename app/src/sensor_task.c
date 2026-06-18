#include "sensor_task.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <string.h>


int getSensorData(int16_t inaStorage[], uint32_t rawTempADC){
	float temperature = getBattTemp(rawTempADC); 

	// TODO: Read the INA and/or call function to do so
	printk("Temperature of the Batt Board: %f\n", (double)temperature);
	return 1;
}


void sensor_task(){
    heartbeat_telemetry_t eps_hb;

    while(1){ // TODO: Ask Electrical (prob Alex J) or Tyler F about what the INAs should be watching for
		eps_hb.battTemp = 0;

		k_msleep(500);
    }
}