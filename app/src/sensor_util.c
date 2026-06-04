#include "sensor_util.h"
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



int readSingleINA(const struct device *ina, int16_t *voltage, int16_t *vshunt, int16_t *current, int16_t *power){
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

	*voltage = (tempVoltage.val1 * 1000) + (tempVoltage.val2 / 1000); //mV
	*current = (tempVshunt.val1 * 1000) + (tempVshunt.val2 / 1000); //mA
	*vshunt = (tempCurrent.val1 * 1000) + (tempCurrent.val2 / 1000); //mA
	*power = (tempPower.val1 * 1000) + (tempPower.val2 / 1000); //mA
	
	return 1;
}


int readAllINA(int16_t inaStorage[]){
	
	readSingleINA(inaMain, &inaStorage[0], &inaStorage[1], &inaStorage[2], &inaStorage[3]);
	readSingleINA(inaTPS3_3V, &inaStorage[4], &inaStorage[5], &inaStorage[6], &inaStorage[7]);
	readSingleINA(inaTPS5V, &inaStorage[8], &inaStorage[9], &inaStorage[10], &inaStorage[11]);
	readSingleINA(inaSolarA, &inaStorage[12], &inaStorage[13], &inaStorage[14], &inaStorage[15]);
	readSingleINA(inaSolarB, &inaStorage[16], &inaStorage[17], &inaStorage[18], &inaStorage[19]);
	readSingleINA(ina5VRF, &inaStorage[20], &inaStorage[21], &inaStorage[22], &inaStorage[23]);
	readSingleINA(ina12V, &inaStorage[24], &inaStorage[25], &inaStorage[26], &inaStorage[27]);
	readSingleINA(inaMPPCA, &inaStorage[28], &inaStorage[29], &inaStorage[30], &inaStorage[31]);
	readSingleINA(inaMPPCB, &inaStorage[32], &inaStorage[33], &inaStorage[34], &inaStorage[35]);

	return 1;
}


int getSensorData(int16_t inaStorage[], uint32_t rawTempADC){
	float temperature = getBattTemp(rawTempADC);

	if(readAllINA(inaStorage)){
		for (int i = 0; i < 9; i+=4){
			printk("INA Main: %dmV, %dmV (vshunt), %dmA, %dmW\n", inaStorage[i], inaStorage[i+1], inaStorage[i+2], inaStorage[i+3]);
		}
	}else{
		return 0;
	}
	printk("Temperature of the Batt Board: %f\n", (double)temperature);
	return 1;
}