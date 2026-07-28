#include "sensor_task.h"


LOG_MODULE_REGISTER(sensorTask, LOG_LEVEL_INF);

int getSensorData(ina219_data_t inaStorage[], float *temperature, uint16_t *battSOC, uint16_t *battVoltage){
	getBattTemp(temperature); 
	readAllINA(inaStorage);

	battCompensateForTemp(*temperature);
	battGetSOC(battSOC, battVoltage);		// Remember the voltage is in milivolts (or microvolts, idk if that's a thing);
	return 1;
}


void sensor_task(){
    heartbeat_telemetry_t eps_hb;

    while(1){ // TODO: Ask Electrical (prob Alex J) or Tyler F about what the INAs should be watching for
		getSensorData(eps_hb.inaInfo, &eps_hb.battTemp, &eps_hb.battSOC, &eps_hb.battVoltage);

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

			if (fault >= 1){
				switch(i) {
					case 0:
						break;
					case 1:
						break;
					case 2:
						break;
					case 3:
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;
					case 8:
						break;
				}
			}
		}

		// Monitoring battery board temperature
		if(eps_hb.battTemp >=25){
			LOG_INF("Batteries have been heated, turning off heater...");
			battHeaterOff();
		}else if(eps_hb.battTemp <= 5){
			LOG_WRN("Battery Board Temp getting low: %fC. Starting heater", (double)eps_hb.battTemp);
			battHeaterOn();
		}else if (eps_hb.battTemp <= 0){
			LOG_WRN("BATTERY TEMP IS CRITICALLY LOW");
		}

		// Monitoring battey percentage
		if(eps_hb.battSOC <= 20){
			LOG_WRN("Battery is starting to get low (below 20%%)");
		}else if(eps_hb.battSOC <= 15){
			LOG_WRN("BATTERY TEMP CRITICALLY LOW, SWITCHING TO LOW POWER");
			// TODO: Add low power mode to satellite. Either command to OBC or something else
		}
		

		k_msleep(500);
    }
}