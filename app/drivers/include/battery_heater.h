/**
 * @file battery_heater.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief   Header file for battery heater gpio toggling functions
 * @version 0.1
 * @date 2026-07-28
 */
#ifndef BATTERY_HEATER_H
#define BATTERY_HEATER_H

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief Sets the battery heater CTRL gpio to high/on
 * 
 * @return int - Returns 1 if toggled, 0 if toggle fails
 */
int battHeaterOn();

/**
 * @brief Sets the battery heater CTRL gpio to low/off
 * 
 * @return int Returns 1 if toggled, 0 if toggle fails
 */
int battHeaterOff();

#endif