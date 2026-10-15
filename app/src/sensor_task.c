#include "sensor_task.h"


LOG_MODULE_REGISTER(sensorTask, LOG_LEVEL_INF);

const int SENSOR_JOB_INTERVAL_MS = 1000;

// TODO: add function call checks here
int getSensorData(ina219_data_t inaStorage[], float *temperature, uint16_t *battSOC, uint16_t *battVoltage){
	getBattTemp(temperature); 				// Read the temp
	readAllINA(inaStorage);					// Read all the INA sensor's information

	battCompensateForTemp(*temperature);	// Calibrate MAX17049 based on the temperature
	battGetSOC(battSOC, battVoltage);		// Remember the voltage is in milivolts (or microvolts, idk if that's a thing);
	return 1;
}


void sensor_task(){
    heartbeat_telemetry_t eps_hb;

    while(1){ // TODO: Ask Electrical (prob Alex J) or Tyler F about what the INAs should be watching for
		getSensorData(eps_hb.inaInfo, &eps_hb.battTemp, &eps_hb.battSOC, &eps_hb.battVoltage);

		// Monitoring battery board temperature
		if(eps_hb.battTemp >=25){
			LOG_INF("Batteries have been heated, turning off heater!");

			if(battHeaterOff()){
				LOG_INF("Battery Heater turned off");
			}else{
				LOG_WRN("Battery Heater failed to turn off...");
			}
		}else if(eps_hb.battTemp <= 5){
			LOG_WRN("Battery Board Temp getting low: %fC. Starting heater", (double)eps_hb.battTemp);

			if(battHeaterOn()){
				LOG_INF("Battery Heater turned on");
			}else{
				LOG_WRN("Battery Heater failed to turn on");
			}
		}else if (eps_hb.battTemp <= 0){
			LOG_WRN("BATTERY TEMP IS CRITICALLY LOW...");
			// TODO: See if we should retry the battery heater gpio toggle. May need to research function to check the signal on the
			//		 gpio line first
		}

		// Monitoring battey percentage
		if(eps_hb.battSOC <= 20){
			LOG_WRN("Battery is starting to get low (below 20%%)");
		}else if(eps_hb.battSOC <= 15){
			LOG_WRN("BATTERY SOC CRITICALLY LOW, SWITCHING TO LOW POWER...");
			// TODO: Add low power mode to satellite. Either command to OBC or something else
		}
		

		k_msleep(1000);
    }
}