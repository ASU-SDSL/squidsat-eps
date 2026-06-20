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

uint8_t battGetSOC(uint16_t battSOCBuffer);

uint8_t battCompensateForTemp(const struct device *max17049, float temp);

#endif