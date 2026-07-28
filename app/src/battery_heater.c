/**
 * @file battery_heater.c
 * @author Aidan Doyle (Doyle-Squared)
 * @brief  Functions and Zephyr device tree nodes to turn the battery
 * heater on and off.
 * @version 0.1
 * @date 2026-07-28
 * 
 */

#include "battery_heater.h"

LOG_MODULE_REGISTER(batteryHeater, LOG_LEVEL_INF);

#define BATT_HEATER DT_ALIAS(rail6)
static const struct gpio_dt_spec batteryHeater = GPIO_DT_SPEC_GET(BATT_HEATER, gpios);

int battHeaterOn(){
    if(!gpio_pin_set_dt(&batteryHeater, 1)){
        LOG_INF("Battery heater has been turned on");
        return 1;
    };
    LOG_WRN("Battery Heater failed to turn on");
    return 0;
}

int battHeaterOff(){
    if(!gpio_pin_set_dt(&batteryHeater, 0)){
        LOG_INF("Battery heater has been turned off");
        return 1;
    };
    LOG_WRN("Battery Heater failed to turn ff");
    return 0;
}