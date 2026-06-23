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
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/fuel_gauge.h>
 
#define MAX17049 DT_NODELABEL(fuel_gauge)

typedef struct {
    uint8_t soc;
    uint16_t voltageMv;
}battData;

/**
 * @brief This function gets the State of Charge (SOC) aka the battery percentage, as well as the voltage.
 * 
 * @param battSOCBuffer A 16-bit unsigned integer pointer to store the battery's SOC value
 * @param battVoltageBuffer A 16-bit signed integer pointer to store the battery's Voltage
 * @return uint8_t Returns a 1 for success, 0 for failure
 */
uint8_t battGetSOC(uint16_t *battSOCBuffer, int16_t *battVoltageBuffer);

/**
 * @brief This function takes the temperature from the ntc thermistor on the battery board and sends the
 * value to the MAX17049's R_COMP register for the IC to recalibrate its SOC calculations based off ambient 
 * temperature. Refer to page 8 of Maxim's MAX17049 data sheet
 * 
 * @param max17049 The Zephyr device-tree node for the MAX17049
 * @param temp Ntc thermistor's temperature from the battery board
 * @return uint8_t Returns a 1 for success, 0 for failure
 */
uint8_t battCompensateForTemp(const struct device *max17049, float temp);

#endif