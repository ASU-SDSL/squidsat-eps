#include "sensor_task.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <string.h>

LOG_MODULE_REGISTER(sensorTask, LOG_LEVEL_INF);

int getSensorData(ina219_data_t inaStorage[], float *temperature){
	getBattTemp(temperature); 
	readAllINA(inaStorage);
	return 1;
}


void sensor_task(){
    heartbeat_telemetry_t eps_hb;

    while(1){ // TODO: Ask Electrical (prob Alex J) or Tyler F about what the INAs should be watching for
		getSensorData(eps_hb.inaInfo, &eps_hb.battTemp);

		for(int i = 0; i < 9; i++){
			//Checking INA instances current
			if(eps_hb.inaInfo[i].current <= 20){
				printk("WARNING --- IS DRAWING TOO MUCH POWER");
			}
		}
		

		k_msleep(500);
    }
}