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
#define U625_TDDI_TD7800				false
#define CX430_TDDI_NT51926				true
#define U717_TDDI_NT51926				false

#if(U625_TDDI_TD7800)
#define FORD_SPSS_CRC_ROLL_EN			true
#define FORD_SPSSV1P0					false
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					true
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		false
#define BACKDOOR_ICDIAG_OPEN			false
#define BACKDOOR_DIAGNOSIS_SIMULATE		false
#define DEBUG_POWER_UP					false
#elif (CX430_TDDI_NT51926)
#define FORD_SPSS_CRC_ROLL_EN			false
#define FORD_SPSSV1P0					false
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					true
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		true
#define BACKDOOR_ICDIAG_OPEN			false
#define BACKDOOR_DIAGNOSIS_SIMULATE		true
#define DEBUG_POWER_UP					false
#elif (U717_TDDI_NT51926)
#define FORD_SPSS_CRC_ROLL_EN			true
#define FORD_SPSSV1P0					true
#define FORD_SPSSV1P1					false
#define FORD_I2CCOMV1P9					false
#define INT_TCH_LATCH					false
#define BACKDOOR_WRITE_DERATINGDATA		true
#define BACKDOOR_ICDIAG_OPEN			false
#define BACKDOOR_DIAGNOSIS_SIMULATE		false
#define DEBUG_POWER_UP					false
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
