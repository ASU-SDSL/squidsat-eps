/**
 * @file max17049.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Code for the MAX17049 IC that keeps track of the batteries' SOC (State of Charge: aka battery percent)
 * @date 2026-06-17
 * 
 */

#ifndef MAX17049_H
#define MAX17049_H
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
 
#define MAX17049 DT_NODE_LABEL(fuel_gauge)

uint8_t battGetSOC(uint16_t battSOCBuffer);

#endif