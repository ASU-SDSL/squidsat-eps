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

int setRail(char * payload, char* command){
    if(strcmp(payload, "payload") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&payloadRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&payloadRail, 1); 
        }
    }else if(strcmp(payload, "lumo") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&lumoRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&lumoRail, 1); 
        }
    }else if(strcmp(payload, "peec") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&peecRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&peecRail, 1); 
        }
    }else if(strcmp(payload, "dant") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&dantRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&dantRail, 1); 
        }
    }else if(strcmp(payload, "nextage") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&nextageRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&nextageRail, 1); 
        }
    }else if(strcmp(payload, "ndant") == 0){
        if (strcmp(command, "off") == 0) {
            gpio_pin_set_dt(&ndantRail, 0); 
        }
        if (strcmp(command, "on") == 0) {
            gpio_pin_set_dt(&ndantRail, 1); 
        }
    }

    return 1;
}