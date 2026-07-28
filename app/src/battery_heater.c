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

#define BATT_HEATER DT_ALIAS(rail6)
static const struct gpio_dt_spec batteryHeater = GPIO_DT_SPEC_GET(BATT_HEATER, gpios);

int battHeaterOn(){
    gpio_pin_set_dt(&batteryHeater, 1);
    return 1;
}

int battHeaterOff(){
    gpio_pin_set_dt(&batteryHeater, 0);
    return 1;
}