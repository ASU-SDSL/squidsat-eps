/**
 * @file set_rail.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Skeleton code that models how we might turn off power to certain payloads in the future
 * @version 0.2
 */

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
    NO_PAYLOAD
}Payload;


typedef enum{
    ON,
    OFF,
    NO_CMD
}Command;
#endif