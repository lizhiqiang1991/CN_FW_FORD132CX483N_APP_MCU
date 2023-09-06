
/******************************************************************************
;       Program		: MC_Power_Management.h
;       Function	: Declare main function & variable
;       Chip		: Cypress CY8C4149AZI-S598
;       Clock		: IMO Internal 48MHz
;       Date		:
;       Author		:
******************************************************************************/
#pragma once

#ifndef M_GPIOSENSE_H
#define M_GPIOSENSE_H

#include "public.h"

#define NUMBER_8BIT(x) ((uint8_t)x)

/* -- Macro define -- */
#define aNVT_REG_CRC		aTRUE

/* Error Code */
/** TDDI action success */
#define aTDDI_OK		1	

/** TDDI action fault */
#define aTDDI_FAIL		-1	

#define aTDDI_VAR_00	NUMBER_8BIT(0)
#define aNT51926ADDR	0x6F

/** Master TDDI i2c slave address */
#define aNTSlaveADDR		(uint8_t)(0x6FU)	

/** Slave TDDI i2c slave address */
#define aNTSlave2ADDR		(uint8_t)(0x6DU)	

/** NT51926 switch page address */
#define	aNTADDR						0x1EU		

/* CMD2_PB, 0x1Fh (IC_Status) ref. -> NT51926 ASIL AN V0.03_20220622.pdf */
/** (Default) TDDI in standby mode */
#define NT51926_STATUS_STANDY		0x00U		

/** TDDI in normal mode */
#define NT51926_STATUS_NORMAL		0x01U		

/** TDDI in BIST mode */
#define NT51926_STATUS_BIST			0x02U

/** TDDI in MUTE mode */
#define NT51926_STATUS_MUTE			0x03U	

/** TDDI in black insertion mode */
#define NT51926_BLACK_INSERTION		0x04U		

/** TDDI in abnormal power off mode */
#define NT51926_ABNORMAL_PWR_OFF	0x05U		

/** TDDI in power on frame 3 mode */
#define NT51926_PWN_ON_FRAME3		0x06U		

/** 0x07 ~ 0x0F are Reserved mode */
#define NT51926_READ_STATUS_FAIL	0x07U


#define ARRY_DEPTH				3U
#define NORMAL_CONTROL_STEPS	23U
#define SHLR_CONTROL_STEPS		11U
#define UPDN_CONTROL_STEPS		17U
#define SERIAL_FLOW_ON_STPE		18U
#define SERIAL_FLOW_OFF_STPE	6U
#define aI2C_TIMEOUT			10U
	
/* Command List */
#define aCMD1		0x10U
#define aCMD2		0x20U
#define aCMD3		0x30U
#define aCMD4		0x40U
#define aCMD6		0x60U
#define aCMDD		0xD0U

#define aPAGE0		0x00U
#define aPAGE1		0x01U
#define aPAGE2		0x02U
#define aPAGE3		0x03U
#define aPAGE4		0x04U
#define aPAGE5		0x05U
#define aPAGE6		0x06U
#define aPAGE7		0x07U
#define aPAGE8		0x08U
#define aPAGE9		0x09U
#define aPAGEA		0x0AU
#define aPAGEB		0x0BU
#define aPAGEC		0x0CU
#define aPAGEF		0x0FU

#define aCMD1_P0		NUMBER_8BIT(aCMD1 | aPAGE0)
#define aCMD1_P1		NUMBER_8BIT(aCMD1 | aPAGE1)
#define aCMD1_P7		NUMBER_8BIT(aCMD1 | aPAGE7)
#define aCMD1_PB		NUMBER_8BIT(aCMD1 | aPAGEB)
#define aCMD2_P0		NUMBER_8BIT(aCMD2 | aPAGE0)
#define aCMD2_P1		NUMBER_8BIT(aCMD2 | aPAGE1)
#define aCMD2_P2		NUMBER_8BIT(aCMD2 | aPAGE2)
#define aCMD2_P8		NUMBER_8BIT(aCMD2 | aPAGE8)
#define aCMD2_PB		NUMBER_8BIT(aCMD2 | aPAGEB)
#define aCMD3_P0		NUMBER_8BIT(aCMD3 | aPAGE0)
#define aCMD4_P0		NUMBER_8BIT(aCMD4 | aPAGE0)
#define aCMDD_P9		NUMBER_8BIT(aCMDD | aPAGE9)

#define aTDDI_ADDR_04			0x04U
#define aTDDI_ADDR_05			0x05U
#define aTDDI_ADDR_08			0x08U
#define aTDDI_ADDR_09			0x09U
#define aTDDI_ADDR_0A			0x0AU
#define aTDDI_ADDR_1D			0x1DU
#define aTDDI_ADDR_1E			0x1EU

#define aTDDI_DATA_00 			0x00U
#define aTDDI_DATA_02 			0x02U
#define aTDDI_DATA_03 			0x03U
#define aTDDI_DATA_10 			0x10U
#define aTDDI_DATA_1B 			0x1BU
#define aTDDI_DATA_20 			0x20U
#define aTDDI_DATA_21 			0x21U
#define aTDDI_DATA_22 			0x22U
#define aTDDI_DATA_30 			0x30U
#define aTDDI_DATA_9F 			0x9FU
#define aTDDI_DATA_A5 			0xA5U
#define aTDDI_DATA_DF 			0xDFU
#define aTDDI_DATA_E2 			0xE2U

#define I2CM_WRITE_RDY	0x00000001UL
#define I2CM_READ_RDY	0x00000001UL

#define aTSC_RST	1U
#define aDISP_GLOBAL_RST	2U

bool M_GPIOSense_LevelDeboucne(GPIO_PRT_Type *pPort, uint8_t u8Pin, tgpio_debounce_def *ptDebounce);
uint64_t M_GPIOSense_LED_Driver_Diagnosis(void);
bool M_GPIOSense_LED_Driver_Clear(void);
uint64_t M_GPIOSense_DisplayFault_Read(void);
uint8_t M_GPIOSense_NT51926_Status_Get(void);

#endif

