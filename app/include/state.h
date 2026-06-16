#ifndef STATE_H
#define STATE_H

typedef enum {
	BOOT,
    WAKE,
    VITALS,
    REGULAR,
	SAFE,
    FAULT,
    RESTART
}State;

State currentState;

#endif