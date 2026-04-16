#include "sensor_util.h"


// enum PowerState ChangePowerState(struct sensor_value value){
//  //Just an example, these values dont mean or do anything yet
// 	switch(value.val1){
// 		case (5):
// 			return Nominal;
// 		case (3):
// 			return Safe;
// 		case (2.9):
// 			return Low_Power;
// 		default:
// 			return Nominal;
// 	}
// }


int GetSensorData(const struct device *ina, struct sensor_value voltage, struct sensor_value vshunt, struct sensor_value current, struct sensor_value power){
	int init = sensor_sample_fetch(ina);
	if (init) {
		printf("Could not fetch sensor data.\n");
		return 0;
	}

	sensor_channel_get(ina, SENSOR_CHAN_VOLTAGE, &voltage);
	sensor_channel_get(ina, SENSOR_CHAN_VSHUNT, &vshunt);
	sensor_channel_get(ina, SENSOR_CHAN_POWER, &current);
	sensor_channel_get(ina, SENSOR_CHAN_CURRENT, &power);

	//float temp = GetTemp(ADCInstance);

	printf("----------------------------------------Sensor Data-------------------------------------\nVoltage is: %d.%dV, Current is: %d.%dA, Power is: %d.%dW, Shunt Voltage is: %d.%d\
		\n----------------------------------------------------------------------------------------\n\n\n", 
		   voltage.val1, voltage.val2, current.val1, current.val2, power.val1, power.val2, vshunt.val1, vshunt.val2);
	
	return 1;
}