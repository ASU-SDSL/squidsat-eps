#include "sensor_task.h"
#include <stdint.h>

//New intatiation of a test INA struct based off the Zephyr INA219 API. Ignore the error squiggles
const struct device *inaMain = DEVICE_DT_GET(INA_MAIN);
const struct device *inaTPS3_3V = DEVICE_DT_GET(INA_TPS3_3V);
const struct device *inaTPS5V = DEVICE_DT_GET(INA_TPS5V);
const struct device *inaSolarA = DEVICE_DT_GET(INA_SOLARA);
const struct device *inaSolarB = DEVICE_DT_GET(INA_SOLARB);
const struct device *ina5VRF = DEVICE_DT_GET(INA_5VRF);
const struct device *ina12V = DEVICE_DT_GET(INA_12V);
const struct device *inaMPPCA = DEVICE_DT_GET(INA_MPPCA);
const struct device *inaMPPCB = DEVICE_DT_GET(INA_MPPCB);



int readSingleINA(const struct device *ina, int16_t inaBuffer[], uint8_t indx){
	int init = sensor_sample_fetch(ina);
	if (init) {
		printf("Could not fetch sensor data.\n");
		return 0;
	}

	struct sensor_value tempVoltage;
	struct sensor_value tempVshunt;
	struct sensor_value tempCurrent;
	struct sensor_value tempPower;

	sensor_channel_get(ina, SENSOR_CHAN_VOLTAGE, &tempVoltage);
	sensor_channel_get(ina, SENSOR_CHAN_VSHUNT, &tempVshunt);
	sensor_channel_get(ina, SENSOR_CHAN_POWER, &tempCurrent);
	sensor_channel_get(ina, SENSOR_CHAN_CURRENT, &tempPower);

	inaBuffer[indx] = (tempVoltage.val1 * 1000) + (tempVoltage.val2 / 1000); 		//mV 	VOLTAGE
	inaBuffer[indx + 1] = (tempVshunt.val1 * 1000) + (tempVshunt.val2 / 1000); 		//mV	SHUNT VOLTAGE
	inaBuffer[indx + 2] = (tempCurrent.val1 * 1000) + (tempCurrent.val2 / 1000); 	//mA	CURRENT
	inaBuffer[indx + 3] = (tempPower.val1 * 1000) + (tempPower.val2 / 1000); 		//mW? 	POWER
	
	return 1;
}


int readAllINA(int16_t inaStorage[]){
	
	readSingleINA(inaMain, inaStorage, 0);
	readSingleINA(inaTPS3_3V, inaStorage, 4);
	readSingleINA(inaTPS5V, inaStorage, 8);
	readSingleINA(inaSolarA, inaStorage, 12);
	readSingleINA(inaSolarB, inaStorage, 16);
	readSingleINA(ina5VRF, inaStorage, 20);
	readSingleINA(ina12V, inaStorage, 24);
	readSingleINA(inaMPPCA, inaStorage, 28);
	readSingleINA(inaMPPCB, inaStorage, 32);

	return 1;
}


int getSensorData(int16_t inaStorage[], uint32_t rawTempADC){
	//float temperature = getBattTemp(rawTempADC); // Commented out since Idk what temp we are reading yet/ if we can read the EPS thermistor

	if(readAllINA(inaStorage)){
		for (int i = 0; i < 36; i+=4){
			printk("INA Main: %dmV, %dmV (vshunt), %dmA, %dmW\n", inaStorage[i], inaStorage[i+1], inaStorage[i+2], inaStorage[i+3]);
		}
	}else{
		return 0;
	}
	//printk("Temperature of the Batt Board: %f\n", (double)temperature);
	return 1;
}


void sensor_task(){
    // int16_t INAStorage[36];
	int16_t singleINABuffer[4];
    
    while(1){ // TODO: Ask Electrical (prob Alex J) or Tyler F about what the INAs should be watching for
        //readAllINA(INAStorage); // All INA Info is loaded into the buffer
		
		//This is just a simple test to check only the voltage of a single INA (U9)
		readSingleINA(inaTPS3_3V, singleINABuffer, 0);	// test
		if(singleINABuffer[0] < 2970){	// 2970mV = 2.97V -> Unsafe electronic operation
			printk("Satellite not recieving 3.3V...");
		}else{
			printk("3.3V rail Nominal at %fV", (double)(singleINABuffer[0])/1000.0); 	// Converting to Volts for readibility
		}

		k_msleep(500);
    }
}