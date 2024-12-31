/******************************************************************************
;       Program		: main.h
;       Function	: Declare main function & variable
;       Chip		: Cypress CY8C4149AZI-S598
;       Clock		: IMO Internal 48MHz
;       Date		: 2021 / 10 / 07
;       Author		: 
******************************************************************************/
#pragma once

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

/* -- Marco Define -- */
// #define CX430_TDDI_NT51926				true
// #define U717_TDDI_NT51926				false
// #define BX726_TDDI_NT51926				false

// #define CX430_TDDI_NT51926				false
// #define U717_TDDI_NT51926				false
// #define BX726_TDDI_NT51926				true

// #define CX430_TDDI_NT51926				false
// #define U717_TDDI_NT51926				true
// #define BX726_TDDI_NT51926				false

/*C132IAA01.0*/
/*Note: U717 is also applicable to the NOVA version of U625.*/ 
#define U717_TDDI_NT51926				false    /*61.FC147.009*/

/*C132IAA01.1*/
#define CX430_TDDI_NT51926				false   /*61.FC147.005*/
#define P758_TDDI_NT51926				false   /*61.FC147.011*/

/*C120UAA01.1*/
#define BX726_TDDI_NT51926				true   /*61.FC147.004*/
#define P708_TDDI_NT51926				false   /*61.FC147.010*/

/*C120UAA01.2*/
#define P702_TDDI_NT51926				false   /*61.FC147.012*/

#if (CX430_TDDI_NT51926 || P758_TDDI_NT51926)
#define FORD_SPSS_CRC_ROLL_EN			false
#define FORD_SPSSV1P0					false
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					true
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		true
#define BACKDOOR_ICDIAG_OPEN			true
#define BACKDOOR_DIAGNOSIS_SIMULATE		false
#define DEBUG_POWER_UP					false
#elif (U717_TDDI_NT51926)
#define FORD_SPSS_CRC_ROLL_EN			true
#define FORD_SPSSV1P0					true
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					false
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		true
#define BACKDOOR_ICDIAG_OPEN			true
#define BACKDOOR_DIAGNOSIS_SIMULATE		false
#define DEBUG_POWER_UP					false
#elif (BX726_TDDI_NT51926 || P708_TDDI_NT51926 || P702_TDDI_NT51926)
#define FORD_SPSS_CRC_ROLL_EN			false
#define FORD_SPSSV1P0					false
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					true
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		true
#define BACKDOOR_ICDIAG_OPEN			true
#define BACKDOOR_DIAGNOSIS_SIMULATE		false
#define DEBUG_POWER_UP					false
#define HIJACK_ADC						false
#endif

/* -- Includes -- */
#include "public.h"

#define LOOP_MAX 0xFFFFFFFFUL

/*------------------------------- global Var.-- -----------------------------*/
extern int16_t MainTestDeratingTemperature;

/*---------------------------- Declare Function -----------------------------*/
void Main_TimerAssign(void);
void Main_TimerCore(void);

#endif
