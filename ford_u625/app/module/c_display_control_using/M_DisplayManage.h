/******************************************************************************
 ;       Program		: MC_Power_Management.h
 ;       Function	: Declare main function & variable
 ;       Chip		: Cypress CY8C4149AZI-S598
 ;       Clock		: IMO Internal 48MHz
 ;       Date		:
 ;       Author		:
 ******************************************************************************/
#pragma once

#ifndef M_DISPLAYMANAGE_H
#define M_DISPLAYMANAGE_H

#include "main.h"

/*---------------------------- Declare Type Define  --------------------------*/
/*For TD7800*/
#define TOUCH_ADDRESS				0x2CU
#define TD7800_SLAVE_ADDRESS		0x01U
#define TD7800_MASTER_ADDRESS		0x03U
/*For NT51926*/
#define TOUCH_SLAVE_ADDRESS			0x01U
#define TOUCH_SLAVE_ADDRESS_UP_FW	0x62U	/* touch firmware update */
#define NT51926_SLAVE_ADDRESS		0x6FU
#define NT51925_STATUS_STANDY		0x00U
#define NT51925_STATUS_NORMAL		0x01U
#define NT51925_STATUS_BIST			0x02U
#define NT51925_STATUS_MUTE			0x03U

#define LP8864_ADDRESS				0x3AU		// for LED driver

extern void M_DM_I2cMasterInit(void);
extern void M_DM_BacklightControl(bool bEnable, bool bLockLoss);
extern uint8_t M_DM_TouchControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss, uint8_t u8LcdCurrentStatus);
extern uint8_t M_DM_DisplayControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss);
extern uint8_t M_DM_ScanningControl(uint8_t u8CurrentStatus, uint8_t u8Command);
extern uint8_t M_DM_LcdControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss);
extern uint32_t M_DM_VCOM_Get(void);
extern void M_DM_VCOM_Set(uint8_t *pReturnData);
extern void M_DM_DisplayStatus_Get(uint8_t *pReturnData);
extern void M_DM_FW_DisplayStatus_Get(uint8_t *pReturnData);
extern void M_DM_INTB_Ctrl(uint8_t u8DigitalLevel);
extern uint8_t M_DM_NT51926_Status_Get(void);
#if(U625_TDDI_TD7800)
extern uint8_t M_DM_TD7800_ATTN_Read (void);
#elif(CX430_TDDI_NT51926 || U717_TDDI_NT51926)
extern uint8_t M_DM_NT51926_ATTN_Read (void);
#else
#endif
#endif
