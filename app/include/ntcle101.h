/**
 * @file ntcle101.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Code to get the temperature of the thermistor located on the Battery Board and store the temperature in a float
 */

#ifndef NTCLE101_H
#define NTCLE101_H

#include <stdint.h>
#include <math.h>

/* Beta Model Equation for getting the tempurature
*   1/T = (1/T_o) + (1/beta) * ln(R_Therm/R_nominal)
*
*  R_Therm Equation(s)
*
*   R_reference * (V_ADC/VCC - V_ADC)   or   R_reference * ((ADC_max/ADC_reading) - 1)
*/

/**
 * @brief Get the ambient temperature of the Battery Board in Celcius
 * 
 * @param ADC_Value Converted analogue value from the thermistor, used to find the temperature
 * @return float Returns the temperature of the Battery Board in Celcius
 */
float getBattTemp(uint32_t ADC_Value);

#endif