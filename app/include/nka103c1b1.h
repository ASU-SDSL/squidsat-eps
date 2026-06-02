#ifndef NKA103C1B1_H
#define NKA103C1B1_H

#include <stdint.h>
#include <math.h>

/* Beta Model Equation for getting the tempurature
*   1/T = (1/T_o) + (1/beta) * ln(R_Therm/R_nominal)
*
*  R_Therm Equation(s)
*
*   R_reference * (V_ADC/VCC - V_ADC)   or   R_reference * ((ADC_max/ADC_reading) - 1)
*/

float getEPSTemp(uint32_t ADC_Value);

#endif