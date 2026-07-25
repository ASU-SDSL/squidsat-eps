#include "max17049.h"

LOG_MODULE_REGISTER(max17049, LOG_LEVEL_INF);

const struct device *max17049 = DEVICE_DT_GET(MAX17049);


uint8_t battGetSOC(uint16_t *battSOCBuffer, int16_t *battVoltageBuffer){
    int init;
    battData data;

    if(!device_is_ready(max17049)){
        LOG_ERR("Fuel Gauge not ready");
        return 0;
    }

    fuel_gauge_prop_t properties[] = {
        FUEL_GAUGE_RELATIVE_STATE_OF_CHARGE,
        FUEL_GAUGE_VOLTAGE
    };

    union fuel_gauge_prop_val tempData[ARRAY_SIZE(properties)];

    init = fuel_gauge_get_props(max17049, properties, tempData, ARRAY_SIZE(properties));

    if(init < 0){
        LOG_ERR("Could not get MAX170 properties");
        return 0;
    }else{
        data.soc = tempData[0].relative_state_of_charge;
        data.voltageMv = (uint16_t)((tempData[1].voltage * 2) / 1000);
        *battSOCBuffer = data.soc;
        *battVoltageBuffer = data.voltageMv;
        LOG_INF("The battery is at %d%%, with a voltage of %dmv", data.soc, data.voltageMv);
        return 1;
    }

    return 0;
}

// TODO: Maybe include the ntc header file and call the getBattTemp function instead of passing in the temperature as a param. Will look into later
uint8_t battCompensateForTemp(float temperature){
    fuel_gauge_prop_t property = FUEL_GAUGE_TEMPERATURE;

    union fuel_gauge_prop_val value;
    value.temperature = (temperature * 10) + 2732;

    if(fuel_gauge_set_prop(max17049, property, value) < 0){
        LOG_ERR("Failed to set MAX170 temperature calibration");
        return 0;
    }else{
        LOG_INF("MAX170 recalibrated from temperature!");
        return 1;
    }
    
    return 0;
}