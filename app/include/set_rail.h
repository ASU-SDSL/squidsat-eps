#ifndef SET_RAIL_H
#define SET_RAIL_H

#include <zephyr/drivers/gpio.h>

typedef enum {
    PAYLOAD,
    LUMONOSITY,
    PEEC,
    DANT,
    NEXTAGE,
    NDANT,
    NONE
}Payload;


typedef enum{
    ON,
    OFF,
    NO_CMD
}Command;
#endif