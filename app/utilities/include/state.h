/**
 * @file state.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Struct for the states of the state machine
 */

#ifndef STATE_H
#define STATE_H

typedef enum {
	BOOT,
    WAKE,
    VITALS,
    DEPLOYING,
    REGULAR,
	SAFE,
    FAULT,
    RESTART
}State;

#endif