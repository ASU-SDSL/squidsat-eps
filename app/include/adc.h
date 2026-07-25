#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

// Setting up the Analogue to Digital converter with Zephyr and the STM32
#define ADC_NODE DT_PATH(zephyr_user)
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET_BY_IDX(ADC_NODE, 0);

extern int16_t raw_ADC;

/**
 * @brief Initializes the STM32's Analog to digital converter
 * 
 * @return int - Returns a 1 if successful set up, 0 otherwise
 */
int adcInit();

/**
 * @brief Reads the Analog value from the STM32's ADC GPIO and converts it to a digital signal. To used
 * to retrieve the NTC thermistor's temperature
 * 
 * @return int 
 */
int adcRead();

#endif