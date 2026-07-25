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

		// Monitoring INA219 readings
		// TODO: use set rail functions per failure
		for(int i = 0; i < 9; i++){
			int fault = 0; // Counts if any of the INA instance's reading are abnormal

			// Checking INA instance's current
			if(eps_hb.inaInfo[i].current >= 10){
				LOG_WRN("WARNING --- HAS TOO MUCH CURRENT");
				fault++;
			}

			// Checking INA instance's voltage
			if(eps_hb.inaInfo[i].voltage >= 20){
				LOG_WRN("WARNING --- IS OVER VOLTAGE");
				fault++;
			}

			if(eps_hb.inaInfo[i].power >= 300){
				LOG_WRN("WARNING --- IS CONSUMING TOO MUCH POWER");
				fault++;
			}
		}

		// Monitoring battery board temperature
		if(eps_hb.battTemp <= 5){
			LOG_WRN("Battery Board Temp getting low: %fC. Starting heater", (double)eps_hb.battTemp);
			// TODO: Do I need to add something to signal the battery heater to start?
		}else if (eps_hb.battTemp <= 0){
			LOG_WRN("BATTERY TEMP IS CRITICALLY LOW");
		}
		

		k_msleep(500);
    }
}