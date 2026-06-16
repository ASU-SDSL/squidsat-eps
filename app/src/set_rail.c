#include "set_rail.h"
#include <string.h>

#define PAYLOAD_NODE DT_ALIAS(rail0)
#define LUMO_NODE DT_ALIAS(rail1)
#define PEEC_NODE DT_ALIAS(rail2)
#define DANT_NODE DT_ALIAS(rail3)
#define NEXTAGE_NODE DT_ALIAS(rail4)
#define NDANT_NODE DT_ALIAS(rail5)

static const struct gpio_dt_spec payloadRail = GPIO_DT_SPEC_GET(PAYLOAD_NODE, gpios);
static const struct gpio_dt_spec lumoRail = GPIO_DT_SPEC_GET(LUMO_NODE, gpios);
static const struct gpio_dt_spec peecRail = GPIO_DT_SPEC_GET(PEEC_NODE, gpios);
static const struct gpio_dt_spec dantRail = GPIO_DT_SPEC_GET(DANT_NODE, gpios);
static const struct gpio_dt_spec nextageRail = GPIO_DT_SPEC_GET(NEXTAGE_NODE, gpios);
static const struct gpio_dt_spec ndantRail = GPIO_DT_SPEC_GET(NDANT_NODE, gpios);



int setRail(Payload payload, Command command){
    switch(payload){
        case PAYLOAD:
            if(command == ON){
                gpio_pin_set_dt(&payloadRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&payloadRail, 0);
            }
            break;
        case LUMONOSITY:
            if(command == ON){
                gpio_pin_set_dt(&lumoRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&lumoRail, 0);
            }
            break;
        case PEEC:
            if(command == ON){
                gpio_pin_set_dt(&peecRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&peecRail, 0);
            }
            break;
        case DANT:
            if(command == ON){
                gpio_pin_set_dt(&dantRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&dantRail, 0);
            }
            break;
        case NEXTAGE:
            if(command == ON){
                gpio_pin_set_dt(&nextageRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&nextageRail, 0);
            }
            break;
        case NDANT:
            if(command == ON){
                gpio_pin_set_dt(&ndantRail, 1); 
            }else if(command == OFF){
                gpio_pin_set_dt(&ndantRail, 0);
            }
            break;
        default:
            return 0;
        }
        return 1;
    
}