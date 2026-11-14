/**
 * @file fault_info.h
 * @author Aidan Doyle (Doyle-Squared)
 * @brief Enums for EPS fault reporting to the EPS
 * @date 2026-08-11
 */

 #ifndef FAULT_INFO_H
 #define FAULT_INFO_H

typedef enum {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
}Severity;


 #endif