#include "ntcle101.h" 
//TODO: Get and congigure the adc value. Research DMA again cause you'll probably forget

/* Beta Parameter Equation for getting the tempurature
*   1/T = (1/T_o) + (1/beta) * ln(R_Therm/R_nominal)
*
*  R_Therm Equation(s)
*
*   R_reference * (V_ADC/VCC - V_ADC)   or   R_reference * ((ADC_max/ADC_reading) - 1)
*/

//TODO Make variables static and screaming case :) since Gio said so
static const float R_REF = 1000.0; //1k Ohms
static const float BETA_COEF = 3534.0; //Kelvin
static const float TEMP_NOM = 25.0;
static const float ADC_MAX = 4095.0;

float GetTemp(int ADC_VALUE){
    //Getting the measured Resistance of the Thermistor
    float R_therm = R_REF * ((ADC_MAX/ADC_VALUE) - 1);

    float temp_k = R_therm/R_REF;
    temp_k = log(temp_k);
    temp_k /= BETA_COEF;
    temp_k += 1.0 / (TEMP_NOM + 273.15);
    temp_k = 1.0 / temp_k;

    return (temp_k - 273.15);
}