#include "adc.h"

LOG_MODULE_REGISTER(adc, LOG_LEVEL_INF);

struct adc_sequence sequence = {
    	.channels = BIT(adc_channel.channel_id),
    	.buffer = &raw_ADC,
    	.buffer_size = sizeof(raw_ADC),
    	.resolution = adc_channel.resolution,
};

int16_t raw_ADC = 0; // Initializing to shut the compiler up, ong leave me alone
int adcFault;

int adcInit(){
    if(!adc_is_ready_dt(&adc_channel)){
        LOG_ERR("ADC device is not ready");
        return 1;
    }

    adcFault = adc_channel_setup_dt(&adc_channel);
    if(adcFault < 0){
        LOG_ERR("Could not setup adc channel");
        return 1;
    }
    return 0;
}

int adcRead(){
    //raw_ADC = 0; // initialize so we dont get compile errors
    adcFault = adc_sequence_init_dt(&adc_channel, &sequence);
    if(adcFault < 0){
        LOG_ERR("ADC sequence init failed");
        return 0;
    }

    adcFault = adc_read_dt(&adc_channel, &sequence);
    if (adcFault < 0) {
        LOG_ERR("Could not read adc for battery temperature");
        return 0;
    } 
    return 1;
}