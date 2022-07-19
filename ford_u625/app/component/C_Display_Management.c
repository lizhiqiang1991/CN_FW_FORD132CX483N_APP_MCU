/* -- Includes -- */
#include "C_Display_Management.h"
#include "C_Power_Management.h"
#include "C_Communication.h"
#include "Memory_Pool.h"
#include "C_Diagnosis.h"
#include "M_DetectTCHAttn.h"
#include "M_INTB.h"
#include "M_BatteryProtect.h"
#include "M_BacklightControl.h"
#include "C_Diagnosis.h"

/* -- Marco Define -- */

/* -- Type Define -- */

/* -- Global Variables -- */
static tdisplay_manage_task_def tDisplayManageTask;
static tdisplay_ctrl_def tDisplayCtrl;

/* -- Local Functions -- */
/**
 * @brief When Detected ATTN Module will Get TP Status,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA
 * 
 * @return MDetectTchAttn_TouchController_E Is Touch Controller Ready?
 * 
 */
static MDetectTchAttn_TouchController_E C_Display_Management_CallbackTCHState(void)
{
    if(Memory_Pool_TouchStatus_Get() != TOUCH_ON)
    {
        return TCH_CONTROLLER_NOTREADY;
    }

    return TCH_CONTROLLER_READY;
}
/**
 * @brief When SDM Detects The behavior of Clicking for TP,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackTCHClickHandler(void)
{
    Memory_Pool_IntStatus_Set(Memory_Pool_IntStatus_Get() | BIT_INT_TCH_POS);
    /* Send INTB Strategy Control Msg. */
    Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
}
/**
 * @brief When SDM Detects The behavior of Leaving Click for TP,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackTCHClickRelHandler(void)
{
    Memory_Pool_IntStatus_Set(Memory_Pool_IntStatus_Get() & ~BIT_INT_TCH_POS);
}
/**
 * @brief When SDM's Battery Voltage is at invaild reange,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackEnteringBatteryProtected(uint8_t u8BpStatus)
{
    /* Turns off backlight */
    Memory_Pool_BacklightEnable_Set(false);
	MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_DISABLE_NODIMMNG);

	/* Action => do shutdown2 sequence*/
	Memory_Pool_VBattProtectState_Set(BATT_PROTECT_ON);
	Memory_Pool_PowerState_Set(SHUTDOWN1OR2_STATE);
	Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
	(void)u8BpStatus;
}
/**
 * @brief When SDM's Battery Voltage is from invaild to vaild,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackLeavingBatteryProtected(void)
{
	Memory_Pool_VBattProtectState_Set(BATT_PROTECT_OFF);
}

/**
 * @brief When SDM's Diagnosis detect error,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackEnteringDiagnosisProtected(void)
{
    /* Turns off backlight */
    Memory_Pool_BacklightEnable_Set(false);
	MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_DISABLE_NODIMMNG);
	/* Action => turn off the display, touch , back-light*/
	tDisplayCtrl.bDiagnosisProtect = true;
	Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
}
/**
 * @brief When SDM's Diagnosis not detect error,
 * The Function will Be Called.
 * 
 * @details NA
 * 
 * @note NA 
 * 
 */
static void C_Display_Management_CallbackLeavingDiagnosisProtected(void)
{
	tDisplayCtrl.bDiagnosisProtect = false;
	tDisplayCtrl.bDiagnosisProtectLeve = true;	
	Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
}

/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Display_Management_RegContrl(uint8_t *pCommand)
{
    uint8_t u8Command = *(pCommand + 1U);
    uint8_t u8Action = *pCommand;
    uint8_t mu8Data[16U] = { 0 };
    switch (u8Command)
    {
        case SCMD_DISPLAY_STATUS_GET:
            mu8Data[0U] = LEN_DISPLAY_STATUS_GET;
            mu8Data[1U] = SCMD_DISPLAY_STATUS_GET;
            M_DM_DisplayStatus_Get(&mu8Data[2U]);
#if (U625_TDDI_TD7800)		
            Memory_Pool_TD7800_Set(mu8Data, LEN_DISPLAY_STATUS_GET + LEN_LEN + LEN_SCMD);
            Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
		
#else
#endif			
            break;
        case SCMD_FW_DISPLAY_STATUS_GET:
            mu8Data[0U] = LEN_FW_DISPLAY_STATUS_GET;
            mu8Data[1U] = SCMD_FW_DISPLAY_STATUS_GET;
            M_DM_FW_DisplayStatus_Get(&mu8Data[2U]);
#if (U625_TDDI_TD7800)			
            Memory_Pool_TD7800_Set(mu8Data, LEN_FW_DISPLAY_STATUS_GET + LEN_LEN + LEN_SCMD);
            Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
		
#else
#endif			
            break;
#if (U625_TDDI_TD7800)
        case SCMD_DISPLAY_ID1_GET:
            mu8Data[0U] = LEN_DISPLAY_ID1_GET;
            mu8Data[1U] = SCMD_DISPLAY_ID1_GET;
            M_DM_DisplayID1_Get(&mu8Data[2U]);
            Memory_Pool_TD7800_Set(mu8Data, LEN_DISPLAY_ID1_GET + LEN_LEN + LEN_SCMD);
            Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
            break;
        case SCMD_DISPLAY_ID2_GET:
            mu8Data[0U] = LEN_DISPLAY_ID2_GET;
            mu8Data[1U] = SCMD_DISPLAY_ID2_GET;
            M_DM_DisplayID2_Get(&mu8Data[2U]);
            Memory_Pool_TD7800_Set(mu8Data, LEN_DISPLAY_ID2_GET + LEN_LEN + LEN_SCMD);
            Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
            break;
        case SCMD_DISPLAY_ID3_GET:
            mu8Data[0U] = LEN_DISPLAY_ID3_GET;
            mu8Data[1U] = SCMD_DISPLAY_ID3_GET;
            M_DM_DisplayID3_Get(&mu8Data[2U]);
            Memory_Pool_TD7800_Set(mu8Data, LEN_DISPLAY_ID3_GET + LEN_LEN + LEN_SCMD);
            Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
            break;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
				
#else
#endif

        case SCMD_FAULT_DETECT_ENABLE:
            if (u8Action == I2C_WRITE)
            {
#if (U625_TDDI_TD7800)         
                Memory_Pool_TD7800_Get(mu8Data, (LEN_FAULT_DETECT_ENABLE + LEN_LEN + LEN_SCMD));
                M_DM_Fault_Enable_Set(&mu8Data[1U]);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
				
#else
#endif				
            }
            else if (u8Action == I2C_READ)
            {
#if (U625_TDDI_TD7800)             
                mu8Data[0U] = LEN_FAULT_DETECT_ENABLE;
                mu8Data[1U] = SCMD_FAULT_DETECT_ENABLE;
                M_DM_Fault_Enable_Get(&mu8Data[2U]);
                Memory_Pool_TD7800_Set(mu8Data, LEN_FAULT_DETECT_ENABLE + LEN_LEN + LEN_SCMD);
                Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
				
#else
#endif				
            }
            else
            { /* Nothing */ }
            break;
        case SCMD_VCOM_SET:
            if (u8Action == I2C_WRITE)
            {
#if (U625_TDDI_TD7800)             
                Memory_Pool_TD7800_Get(mu8Data, (LEN_VCOM_SET + LEN_LEN + LEN_SCMD));
                M_DM_TD7800_Unlock();
                M_DM_VCOM_Set(&mu8Data[1U]);
                M_DM_TD7800_Lock();
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
		
#else
#endif				
            }
            else if (u8Action == I2C_READ)
            {
#if (U625_TDDI_TD7800)             
                mu8Data[0U] = LEN_VCOM_SET;
                mu8Data[1U] = SCMD_VCOM_SET;
                M_DM_VCOM_Get(&mu8Data[2U]);
                Memory_Pool_TD7800_Set(mu8Data, LEN_VCOM_SET + LEN_LEN + LEN_SCMD);
                Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_TD7800_REG_READY);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)

#else
#endif

            }
            else
            { /* Nothing */ }
            break;
        default:
        	/* Nothing */
            break;
    }
}
/******************************************************************************
 ;       Function Name			:	uint8_t C_Display_Sequence_Control(uint8_t u8CtrlStatus, uint8_t u8SetValue)
 ;       Function Description	:	This state will do display 
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static uint8_t C_Display_Sequence_Control(uint8_t u8CtrlStatus, uint8_t u8SetValue)
{	
	uint8_t u8ReturnStatus = DS_ACTION_NONE;
	uint32_t u32CommDisplayStatus = 0U;
	uint32_t u32Temp = 0U;

	if(tDisplayCtrl.bPowerStartupEvent == true)
	{
		switch (u8SetValue)
		{
			case DISPLAY_OFF_TOUCH_OFF :
					if(u8CtrlStatus == DS_ACTION_NONE)
					{
						u8CtrlStatus = DS_ACTION_BACKLIGHT;
					}
					else
					{ /* Nothing */ }

					switch (u8CtrlStatus)
					{
						case DS_ACTION_BACKLIGHT:
							/* Disable backlight function */
							Memory_Pool_BacklightEnable_Set(false);				          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)											
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();

							/* Set 0x00 BL_ST bit */				             
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS);			            
#endif						
							u8ReturnStatus  = DS_ACTION_DISPLAY_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_1ms;							

							break;
						case DS_ACTION_DISPLAY_CTRL:
							/* Disable LCD */
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));	
				            
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 DISP_ST bit */	
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);							
							u8ReturnStatus  = DS_ACTION_DISPLAY_RESET;							
#if (U625_TDDI_TD7800)             
							tDisplayManageTask.u16Timer1 = TIME_101ms;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
							tDisplayManageTask.u16Timer1 = TIME_151ms;		
#else
#endif							
							break;							
						case DS_ACTION_DISPLAY_RESET:
							/* Reset Display  */
							Memory_Pool_LcdResetStatus_Set(M_DM_LcdControl(Memory_Pool_LcdResetStatus_Get(),u8SetValue, Memory_Pool_LockLoss_Get()));
							u8ReturnStatus  = DS_ACTION_TOUCH_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_11ms;
							
							break;
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();					
							
							/* Set 0x00 TSC_ST bit */            				
                			Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS);
							if(Memory_Pool_PowerState_Get() == SHUTDOWN1OR2_STATE)
							{
								Memory_Pool_PowerState_Set(OFF_POWER_STATE);
								Memory_Pool_PowerStatus_Set(POWER_OFF);
								Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
							}
							else
							{ /* Nothing */ }
													
							tDisplayManageTask.u16Timer1 = TIME_2ms; /* TIME_1000ms */
							u8ReturnStatus  = DS_ACTION_CTRL_PROTECT;
							break;
						case DS_ACTION_CTRL_PROTECT:
							/* t_PON time 1sec */					
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;

							break;							
						default:
							/* Nothing */
							break;
					}
					break;
			case DISPLAY_ON_TOUCH_OFF :
					if(u8CtrlStatus == DS_ACTION_NONE)
					{
						u8CtrlStatus = DS_ACTION_DISPLAY_CTRL;
					}
					else
					{ /* Nothing */ }

					switch (u8CtrlStatus)
					{
						case DS_ACTION_DISPLAY_CTRL:
							/* Enable LCD */
							Memory_Pool_LcdResetStatus_Set(LCD_RESET_HIGH);
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));				
							u8ReturnStatus  = DS_ACTION_DISPLAY_STATUS;          
							tDisplayManageTask.u16Timer1 = TIME_190ms;

							break;
						case DS_ACTION_DISPLAY_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();						
							
							/* Set 0x00 DISP_ST bit */							
							if (Memory_Pool_LcdStatus_Get() == DISPLAY_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_DISP_ST_POS);
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);
							} 							
							u8ReturnStatus  = DS_ACTION_BACKLIGHT;	         
							tDisplayManageTask.u16Timer1 = TIME_11ms;
						
							break;
						case DS_ACTION_BACKLIGHT:
							/* Enable backlight function */		          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 BL_ST bit */							
							if (tDisplayCtrl.bBacklightSet == true)
							{
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)								
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_BL_ST_POS);
#endif								
								/* Nothing */
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS);
							}				            
						
							u8ReturnStatus  = DS_ACTION_BACKLIGHT_PWM;
							tDisplayManageTask.u16Timer1 = TIME_13ms;													

							break;
						case DS_ACTION_BACKLIGHT_PWM:
							/* Enable backlight PWM function */
							if (tDisplayCtrl.bBacklightSet == true)
							{								
								MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_ENABLE_DIMMING);
							}
							else
							{ /* Nothing */ }				            
						
							u8ReturnStatus  = DS_ACTION_TOUCH_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_32ms;													

							break;							
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */							
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS); 
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS); 
							}
						
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
							break;
							
						default:
							/* Nothing */
							break;
					}
					break;
			case DISPLAY_OFF_TOUCH_ON :  /* TBD */
#if 0
					if(u8CtrlStatus == DS_ACTION_NONE)
					{
						u8CtrlStatus = DS_ACTION_DISPLAY_STATUS;
					}
					else
					{ /* Nothing */ }

					switch (u8CtrlStatus)
					{
						case DS_ACTION_DISPLAY_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();						
							
							/* Set 0x00 DISP_ST bit */							
							if (Memory_Pool_LcdStatus_Get() == DISPLAY_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_DISP_ST_POS);
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);
							} 							
							u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;	         
							tDisplayManageTask.u16Timer1 = TIME_250ms;
						
							break;
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */							
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS); 
								tDisplayManageTask.u16Timer2 = TIME_2ms;
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS);
							}
						
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
							break;
							
						default:
							/* Nothing */
							break;
					}
#endif
					tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
					u8ReturnStatus  = DS_ACTION_NONE;
					tDisplayManageTask.u16Timer1 = TIME_DISABLE;				
					break;
			case DISPLAY_ON_TOUCH_ON :
					if(u8CtrlStatus == DS_ACTION_NONE)
					{
						u8CtrlStatus = DS_ACTION_DISPLAY_CTRL;
					}
					else
					{ /* Nothing */ }
								
					switch (u8CtrlStatus)
					{
						case DS_ACTION_DISPLAY_CTRL:
							/* Enable LCD */
							Memory_Pool_LcdResetStatus_Set(LCD_RESET_HIGH);
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));				
							u8ReturnStatus  = DS_ACTION_DISPLAY_STATUS;          
							tDisplayManageTask.u16Timer1 = TIME_190ms;

							break;
						case DS_ACTION_DISPLAY_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();

							/* Set 0x00 DISP_ST bit */							
							if (Memory_Pool_LcdStatus_Get() == DISPLAY_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_DISP_ST_POS);
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);
							} 							
							u8ReturnStatus  = DS_ACTION_BACKLIGHT;	         
							tDisplayManageTask.u16Timer1 = TIME_11ms;
						
							break;
						case DS_ACTION_BACKLIGHT:
							/* Enable backlight function */		          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());

							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 BL_ST bit */							
							if (tDisplayCtrl.bBacklightSet == true)
							{
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)								
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_BL_ST_POS);
#endif
								/* Nothing */
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS);
							}
							tDisplayManageTask.u16Timer1 = TIME_13ms;
							u8ReturnStatus  = DS_ACTION_BACKLIGHT_PWM;												

							break;
						case DS_ACTION_BACKLIGHT_PWM:
							/* Enable backlight PWM function */
							if (tDisplayCtrl.bBacklightSet == true)
							{								
								MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_ENABLE_DIMMING);
							}
							else
							{ /* Nothing */ }				            
						
							u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;
							tDisplayManageTask.u16Timer1 = TIME_32ms;													

							break;							
						case DS_ACTION_TOUCH_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();
							
							/* Set 0x00 TSC_ST bit */							
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS);
								tDisplayManageTask.u16Timer2 = TIME_2ms;
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS);
							}						
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;												

							break;							
						default:
							/* Nothing */
							break;
					}
					break;					
			default:
				/* Nothing */
				break;
		}
	}
	else
	{
		switch (u8SetValue)
		{
			case DISPLAY_OFF_TOUCH_OFF :
					if(tDisplayCtrl.u8LastDisplayStatus != DISPLAY_OFF_TOUCH_OFF)
					{
						if(u8CtrlStatus == DS_ACTION_NONE)
						{
							u8CtrlStatus = DS_ACTION_BACKLIGHT;
						}
						else
						{ /* Nothing */ }
					}
					else
					{
						u8CtrlStatus = DS_ACTION_NONE;
						tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
						u8ReturnStatus  = DS_ACTION_NONE;
						tDisplayCtrl.bPowerStartupEvent =false;
						tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
						tDisplayManageTask.u16Timer1 = TIME_DISABLE;					
					}

					switch (u8CtrlStatus)
					{
						case DS_ACTION_BACKLIGHT:
							/* Disable backlight function */
							Memory_Pool_BacklightEnable_Set(false);				          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();

							/* Set 0x00 BL_ST bit */					
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS);				            
#endif						
							u8ReturnStatus  = DS_ACTION_DISPLAY_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_1ms;							

							break;
						case DS_ACTION_DISPLAY_CTRL:
							/* Disable LCD */
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));	
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();
							
							/* Set 0x00 DISP_ST bit */	
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS); 
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);

							u8ReturnStatus  = DS_ACTION_DISPLAY_RESET;							
#if (U625_TDDI_TD7800)             
							tDisplayManageTask.u16Timer1 = TIME_101ms;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
							tDisplayManageTask.u16Timer1 = TIME_151ms;		
#else
#endif					
							break;							
						case DS_ACTION_DISPLAY_RESET:
							/* Reset Display  */
							Memory_Pool_LcdResetStatus_Set(M_DM_LcdControl(Memory_Pool_LcdResetStatus_Get(),u8SetValue, Memory_Pool_LockLoss_Get()));
							u8ReturnStatus  = DS_ACTION_TOUCH_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_11ms;
							
							break;
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */            				
                			Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS); 
							if(Memory_Pool_PowerState_Get() == SHUTDOWN1OR2_STATE)
							{
								Memory_Pool_PowerState_Set(OFF_POWER_STATE);
								Memory_Pool_PowerStatus_Set(POWER_OFF);
								Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
							}
							else
							{ /* Nothing */ }

							tDisplayManageTask.u16Timer1 = TIME_2ms; /* TIME_1000ms */
							u8ReturnStatus  = DS_ACTION_CTRL_PROTECT;;
							break;
						case DS_ACTION_CTRL_PROTECT:
							/* t_PON time 1sec */					
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;

							break;							
						default:
							/* Nothing */
							break;
					}
					break;
			case DISPLAY_ON_TOUCH_OFF :
					if(tDisplayCtrl.u8LastDisplayStatus != DISPLAY_ON_TOUCH_OFF)
					{	
						if((tDisplayCtrl.u8LastDisplayStatus == DISPLAY_OFF_TOUCH_OFF) || (tDisplayCtrl.u8LastDisplayStatus == DISPLAY_OFF_TOUCH_ON))
						{
							if(u8CtrlStatus == DS_ACTION_NONE)
							{
								u8CtrlStatus = DS_ACTION_TOUCH_CTRL;
							}
							else
							{ /* Nothing */}
						}
						else if(tDisplayCtrl.u8LastDisplayStatus == DISPLAY_ON_TOUCH_ON)
						{
							if(u8CtrlStatus == DS_ACTION_NONE)
							{
								u8CtrlStatus = DS_ACTION_TOUCH_STATUS;
							}
							else
							{ /* Nothing */}
						}
						else
						{ /* Nothing */}						
					}
					else
					{
						u8CtrlStatus = DS_ACTION_NONE;
						tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
						tDisplayCtrl.bPowerStartupEvent =false;
						tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
						tDisplayManageTask.u16Timer1 = TIME_DISABLE;
						u8ReturnStatus  = DS_ACTION_NONE;						
					}
					
					switch (u8CtrlStatus)
					{
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), DISPLAY_ON_TOUCH_ON, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							u8ReturnStatus  = DS_ACTION_DISPLAY_RESET;          
							tDisplayManageTask.u16Timer1 = TIME_11ms;
							break;
						case DS_ACTION_DISPLAY_RESET:
							/* Reset DISPLAY */
							Memory_Pool_LcdResetStatus_Set(M_DM_LcdControl(Memory_Pool_LcdResetStatus_Get(),u8SetValue, Memory_Pool_LockLoss_Get()));
							u8ReturnStatus  = DS_ACTION_DISPLAY_CTRL;          
							tDisplayManageTask.u16Timer1 = TIME_201ms;
							break;							
						case DS_ACTION_DISPLAY_CTRL:
							/* Enable LCD */
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));				
							u8ReturnStatus  = DS_ACTION_DISPLAY_STATUS;          
							tDisplayManageTask.u16Timer1 = TIME_190ms;

							break;
						case DS_ACTION_DISPLAY_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();

							/* Set 0x00 DISP_ST bit */							
							if (Memory_Pool_LcdStatus_Get() == DISPLAY_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_DISP_ST_POS); 
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);
							} 							
							u8ReturnStatus  = DS_ACTION_BACKLIGHT;	         
							tDisplayManageTask.u16Timer1 = TIME_11ms;
						
							break;
						case DS_ACTION_BACKLIGHT:
							/* Enable backlight function */		          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();
														
							/* Set 0x00 BL_ST bit */							
							if (tDisplayCtrl.bBacklightSet == true)
							{
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)								
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_BL_ST_POS); 
#endif								
								/* Nothing */
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS); 
							}						
							u8ReturnStatus  = DS_ACTION_BACKLIGHT_PWM;
							tDisplayManageTask.u16Timer1 = TIME_11ms;													

							break;
						case DS_ACTION_BACKLIGHT_PWM:
							/* Enable backlight PWM function */
							if (tDisplayCtrl.bBacklightSet == true)
							{								
								MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_ENABLE_DIMMING);
							}
							else
							{ /* Nothing */ }				            
						
							u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;
							tDisplayManageTask.u16Timer1 = TIME_34ms;

							break;
						case DS_ACTION_TOUCH_STATUS:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */							
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS); 
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS); 
							}
						
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
							u8ReturnStatus  = DS_ACTION_NONE;
							break;
							
						default:
							/* Nothing */
							break;
					}

					break;
			case DISPLAY_OFF_TOUCH_ON :  /* TBD */
#if 0			
					if(tDisplayCtrl.u8LastDisplayStatus != DISPLAY_OFF_TOUCH_ON)
					{
						if((tDisplayCtrl.u8LastDisplayStatus == DISPLAY_ON_TOUCH_OFF) || (tDisplayCtrl.u8LastDisplayStatus == DISPLAY_ON_TOUCH_ON))
						{
							if(u8CtrlStatus == DS_ACTION_NONE)
							{
								u8CtrlStatus = DS_ACTION_BACKLIGHT;
							}
							else
							{ /* Nothing */}
						}
						else if(tDisplayCtrl.u8LastDisplayStatus == DISPLAY_OFF_TOUCH_OFF)
						{
							if(u8CtrlStatus == DS_ACTION_NONE)
							{
								u8CtrlStatus = DS_ACTION_TOUCH_CTRL;
							}
							else
							{ /* Nothing */}
						}
						else
						{ /* Nothing */}
					}		
					else
					{
						u8CtrlStatus = DS_ACTION_NONE;
					}						
					switch (u8CtrlStatus)
					{
						case DS_ACTION_BACKLIGHT:
							/* Disable backlight function */
							Memory_Pool_BacklightEnable_Set(false);				          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();

							/* Set 0x00 BL_ST bit */					
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS);				            
#endif						
							u8ReturnStatus  = DS_ACTION_DISPLAY_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_1ms;							

							break;
						case DS_ACTION_DISPLAY_CTRL:
							/* Disable LCD */
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));	
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();
							
							/* Set 0x00 DISP_ST bit */	
				            Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS); 
							Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS);

							u8ReturnStatus  = DS_ACTION_DISPLAY_RESET;							
#if (U625_TDDI_TD7800)             
							tDisplayManageTask.u16Timer1 = TIME_101ms;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
							tDisplayManageTask.u16Timer1 = TIME_151ms;		
#else
#endif					
							break;							
						case DS_ACTION_DISPLAY_RESET:
							/* Reset Display  */
							Memory_Pool_LcdResetStatus_Set(M_DM_LcdControl(Memory_Pool_LcdResetStatus_Get(),u8SetValue, Memory_Pool_LockLoss_Get()));
							u8ReturnStatus  = DS_ACTION_TOUCH_CTRL;
							tDisplayManageTask.u16Timer1 = TIME_11ms;
							
							break;
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							tDisplayManageTask.u16Timer1 = TIME_2ms; 
							u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;;
							break;
						case DS_ACTION_TOUCH_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS); 
								tDisplayManageTask.u16Timer2 = TIME_2ms;
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS); 
							}
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
														
							break;							
						default:
							/* Nothing */
							break;
					}
#endif				
					u8ReturnStatus  = DS_ACTION_NONE;
					tDisplayManageTask.u16Timer1 = TIME_DISABLE;
					tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;	
					break;
			case DISPLAY_ON_TOUCH_ON :
					if(tDisplayCtrl.u8LastDisplayStatus != DISPLAY_ON_TOUCH_ON)
					{	
						if((tDisplayCtrl.u8LastDisplayStatus == DISPLAY_OFF_TOUCH_OFF) || (tDisplayCtrl.u8LastDisplayStatus == DISPLAY_OFF_TOUCH_ON) || (tDisplayCtrl.u8LastDisplayStatus == DISPLAY_ON_TOUCH_OFF))
						{
							if(u8CtrlStatus == DS_ACTION_NONE)
							{
								u8CtrlStatus = DS_ACTION_TOUCH_CTRL;
							}
							else
							{ /* Nothing */}
						}
						else
						{ /* Nothing */}						
					}
					else
					{
						u8CtrlStatus = DS_ACTION_NONE;
						tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
						tDisplayCtrl.bPowerStartupEvent =false;
						tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
						u8ReturnStatus  = DS_ACTION_NONE;
						tDisplayManageTask.u16Timer1 = TIME_DISABLE;						
					}

					switch (u8CtrlStatus)
					{
						case DS_ACTION_TOUCH_CTRL:
							/* Reset Touch */
							Memory_Pool_TouchStatus_Set(M_DM_TouchControl(Memory_Pool_TouchStatus_Get(), DISPLAY_ON_TOUCH_ON, Memory_Pool_LockLoss_Get(), Memory_Pool_LcdStatus_Get()));
							if(tDisplayCtrl.u8LastDisplayStatus == DISPLAY_ON_TOUCH_OFF)
							{
								u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;          
								tDisplayManageTask.u16Timer1 = TIME_246ms;
							}
							else
							{
								u8ReturnStatus  = DS_ACTION_DISPLAY_RESET;          
								tDisplayManageTask.u16Timer1 = TIME_11ms;
							}
							break;
						case DS_ACTION_DISPLAY_RESET:
							/* Reset Touch */
							Memory_Pool_LcdResetStatus_Set(M_DM_LcdControl(Memory_Pool_LcdResetStatus_Get(),u8SetValue, Memory_Pool_LockLoss_Get()));
							u8ReturnStatus  = DS_ACTION_DISPLAY_CTRL;          
							tDisplayManageTask.u16Timer1 = TIME_201ms;

							break;
						case DS_ACTION_DISPLAY_CTRL:
							/* Enable LCD */
							Memory_Pool_LcdStatus_Set(M_DM_DisplayControl(Memory_Pool_LcdStatus_Get(), u8SetValue, Memory_Pool_LockLoss_Get()));				
							u8ReturnStatus  = DS_ACTION_DISPLAY_STATUS;          
							tDisplayManageTask.u16Timer1 = TIME_190ms;

							break;
						case DS_ACTION_DISPLAY_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 DISP_ST bit */
							if (Memory_Pool_LcdStatus_Get() == DISPLAY_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_DISP_ST_POS); 
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_DISP_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_DISP_ST_POS); 
							} 							
							u8ReturnStatus  = DS_ACTION_BACKLIGHT;	         
							tDisplayManageTask.u16Timer1 = TIME_11ms;
						
							break;
						case DS_ACTION_BACKLIGHT:
							/* Enable backlight function */		          
							M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, Memory_Pool_LockLoss_Get());
							
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 BL_ST bit */
							if (tDisplayCtrl.bBacklightSet == true)
							{
#if(U717_TDDI_NT51926 || FORD_SPSS_CRC_ROLL_EN)								
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_BL_ST_POS); 
#endif								
								/* Nothing */
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_BL_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_BL_ST_POS); 
							}						
							u8ReturnStatus  = DS_ACTION_BACKLIGHT_PWM;	         
							tDisplayManageTask.u16Timer1 = TIME_11ms;

							break;
						case DS_ACTION_BACKLIGHT_PWM:
							/* Enable backlight PWM function */
							if (tDisplayCtrl.bBacklightSet == true)
							{								
								MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_ENABLE_DIMMING);
							}
							else
							{ /* Nothing */ }				            
						
							u8ReturnStatus  = DS_ACTION_TOUCH_STATUS;
							tDisplayManageTask.u16Timer1 = TIME_34ms;													

							break;							
						case DS_ACTION_TOUCH_STATUS:
							/* Read 0x00 status. */
							u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
							u32Temp = Memory_Pool_ActualDisplayStatus_Get();							
							
							/* Set 0x00 TSC_ST bit */
							if (Memory_Pool_TouchStatus_Get() == TOUCH_ON)
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_TSC_ST_POS); 
								tDisplayManageTask.u16Timer2 = TIME_2ms;
							}
							else
							{
								Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus & ~BIT_TSC_ST_POS);
								Memory_Pool_ActualDisplayStatus_Set(u32Temp & ~BIT_TSC_ST_POS); 
							}
							tDisplayCtrl.u8LastDisplayStatus = u8SetValue;
							tDisplayCtrl.bPowerStartupEvent =false;
							tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
							u8ReturnStatus  = DS_ACTION_NONE;
							tDisplayManageTask.u16Timer1 = TIME_DISABLE;
														
							break;							
						default:
							/* Nothing */
							break;
					}
					break;					
			default:
				/* Nothing */
				break;
		}
	}
	return u8ReturnStatus;
}

/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Display_Management_ParaInit(void)
{
    Memory_Pool_Shutdown_Set(NUMBER_ZERO);
    Memory_Pool_DisplayScan_Set(SCAN_VT_HL);
    Memory_Pool_DisplayEnable_Set(DISPLAY_OFF_TOUCH_OFF);
	Memory_Pool_I2CMcuInit_Set(false);
	Memory_Pool_VBattProtectState_Set(BATT_PROTECT_OFF);

    tDisplayManageTask.u16Timer1 = TIME_DISABLE;
    tDisplayManageTask.u16Timer2 = TIME_DISABLE;
	tDisplayManageTask.u16Timer3 = TIME_DISABLE;
    tDisplayManageTask.u32Timeout = TIME_DISABLE;

	tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_OFF;
	tDisplayCtrl.bPowerStartupEvent = false;
	tDisplayCtrl.u8LastDisplayStatus = DISPLAY_OFF_TOUCH_OFF;
	tDisplayCtrl.u8CurrentDisplaySet = DISPLAY_OFF_TOUCH_OFF;
	tDisplayCtrl.bBacklightSet = false;
	tDisplayCtrl.bDiagnosisProtectLeve = false;
	tDisplayCtrl.bDiagnosisProtect = false;
}
/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Display_Manage_Init(void)
{
    switch (Task_Current_Event_Get())
    {
        case EVENT_FIRST :
            C_Display_Management_ParaInit();

			tDisplayManageTask.u16Timer1 = TIME_2ms;

            break;
        case EVENT_I2C_MCU_INITIAL_DELAY :
            /* Check power ready. */
            if (Memory_Pool_PowerState_Get() == NORMAL_RUN_STATE)
            {
            	/* Power is ready and start I2C Master bus Initialize */
                M_DM_I2cMasterInit();
				Memory_Pool_TouchStatus_Set(TOUCH_ON);
				Memory_Pool_LcdResetStatus_Set(LCD_RESET_HIGH);
				tDisplayCtrl.bPowerStartupEvent = true;
#if (U625_TDDI_TD7800) 
                /* Registers Detect TCH ATTN Module. */          
				MDetectTchAttn_Register(C_Display_Management_CallbackTCHState\
                ,M_DM_TD7800_ATTN_Read\
				,C_Display_Management_CallbackTCHClickHandler\
                ,C_Display_Management_CallbackTCHClickRelHandler\
                ,ATTN_TRI_FALLING);

#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
                /* Registers Detect TCH ATTN Module. */
                MDetectTchAttn_Register(C_Display_Management_CallbackTCHState\
                ,M_DM_NT51926_ATTN_Read\
                ,C_Display_Management_CallbackTCHClickHandler\
                ,C_Display_Management_CallbackTCHClickRelHandler\
                ,ATTN_TRI_FALLING);
		
#else
#endif
                /* Registers Two Callback Functions for Entering Battery Protected and Leaving Battery Protected. */
                M_BP_Callback_Register(C_Display_Management_CallbackEnteringBatteryProtected\
                ,C_Display_Management_CallbackLeavingBatteryProtected);

                /* Registers Two Callback Functions for Entering Diagnostic Protected and Leaving Diagnostic  Protected. */
                C_Diagnosis_Callback_Register(C_Display_Management_CallbackEnteringDiagnosisProtected\
				, C_Display_Management_CallbackLeavingDiagnosisProtected);

                tDisplayManageTask.u16Timer1 = TIME_DISABLE;

				/* Delay 100ms =100ms + eeprom read time 91us + VBAT check 10ms */
				tDisplayManageTask.u16Timer2 = TIME_90ms; 			
            }
            else
            {
				/* Delay 2ms and retry again. */
				tDisplayManageTask.u16Timer1 = TIME_2ms;
            }
            break;
        case EVENT_DISPLAY_INITIAL_DELAY :
            	
	        /* Delay display control. */
	        Memory_Pool_I2CMcuInit_Set(true);
#if(DEBUG_POWER_UP)			
			HAL_GPIO_Toggle( U301_INTB_IN_PORT,  U301_INTB_IN_PIN); 
#endif
	        tDisplayManageTask.u16Timer2 = TIME_DISABLE;
	        Task_ChangeState(TYPE_DISPLAY_MANAGE, LEVEL5, STATE_DISPLAY_MANAGE_CTRL, Display_Manage_State_Machine[STATE_DISPLAY_MANAGE_CTRL]);

            break;
        default:
        	/* Nothing */
            break;
    }
    Task_TaskDone();
}
/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Control(void)
 ;       Function Description	:	This state will do power on/off sequence
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Display_Manage_Control(void)
{
	uint8_t u8Status = tDisplayCtrl.u8DispSeqStatus;
    uint32_t u32Temp=0U;
	uint32_t u32CommDisplayStatus=0U;
	
#if (U625_TDDI_TD7800)
    uint8_t mu8Temp[2U];
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
    uint8_t mu8Temp[8U];
#else
#endif

    switch (Task_Current_Event_Get())
    {
        case EVENT_FIRST :
			tDisplayManageTask.u16Timer3 = TIME_2ms;
            break;
        case EVENT_MESSAGE_DISPLAY_ENABLE :
			if((tDisplayCtrl.u8DisplayEnLock == DISP_SEQ_LOCK_OFF) && (Memory_Pool_PowerStatus_Get() != POWER_OFF))
			{	
				tDisplayCtrl.u8DisplayEnLock = DISP_SEQ_LOCK_ON;
				
				tDisplayCtrl.u8CurrentDisplaySet = Memory_Pool_DisplayEnable_Get();
				tDisplayCtrl.bBacklightSet = Memory_Pool_BacklightEnable_Get();
								
				if(Memory_Pool_PowerState_Get() == SHUTDOWN1OR2_STATE)
				{
					/* Action => do shutdown sequence*/
					MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_DISABLE_NODIMMNG);
					tDisplayCtrl.u8CurrentDisplaySet = DISPLAY_OFF_TOUCH_OFF;
					Memory_Pool_DisplayEnable_Set(DISPLAY_OFF_TOUCH_OFF);
					u8Status = DS_ACTION_NONE;
					tDisplayManageTask.u16Timer1 = TIME_2ms;
				}
				else if(tDisplayCtrl.bDiagnosisProtect == true )
				{	
					/* Action => turn off the display, touch , back-light*/
		            if ((Memory_Pool_LcdStatus_Get() == DISPLAY_ON) || (Memory_Pool_TouchStatus_Get() == TOUCH_ON))
		            {
		                /* Backup the DisplayEnable status*/
		            	Memory_Pool_DisplayEnableBp_Set(Memory_Pool_DisplayEnable_Get());
		            }
					else
					{ /* Nothing */ }	
					Memory_Pool_DisplayEnable_Set(DISPLAY_OFF_TOUCH_OFF);
					tDisplayCtrl.u8CurrentDisplaySet = DISPLAY_OFF_TOUCH_OFF;

					u8Status = DS_ACTION_NONE;
					u8Status = C_Display_Sequence_Control(u8Status, tDisplayCtrl.u8CurrentDisplaySet);
					tDisplayCtrl.u8DispSeqStatus = u8Status;
				}
				else if (Memory_Pool_FactoryMode_Get() == OTP_MODE)
				{
					/* In OTP test mode, enable back-light */
					M_DM_BacklightControl(tDisplayCtrl.bBacklightSet, false);
					MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_ENABLE_DIMMING);
				}
				else if (Memory_Pool_FactoryMode_Get() == TPT_MODE)
				{
					/* In OTP test mode, enable back-light */
					Memory_Pool_DisplayEnable_Set(DISPLAY_ON_TOUCH_ON);
					tDisplayCtrl.u8CurrentDisplaySet = DISPLAY_ON_TOUCH_ON;

					u8Status = DS_ACTION_NONE;
					u8Status = C_Display_Sequence_Control(u8Status, tDisplayCtrl.u8CurrentDisplaySet);
					tDisplayCtrl.u8DispSeqStatus = u8Status;
				}
				else if(tDisplayCtrl.bDiagnosisProtectLeve == true)
				{
					/* Action =>if not detect error, turn on the display, touch , back-light*/
					Memory_Pool_DisplayEnable_Set(Memory_Pool_DisplayEnableBp_Get());
					tDisplayCtrl.u8CurrentDisplaySet = Memory_Pool_DisplayEnable_Get();
		            if ((Memory_Pool_DisplayEnable_Get() & BIT_DISP_EN_POS) == DISPLAY_DISABLE)
		            {
		                /* Delay time and disable back-light function */
						MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_DISABLE_NODIMMNG);
		            }
		            else
		            { /* Nothing */}
					u8Status = DS_ACTION_NONE;

					u8Status = C_Display_Sequence_Control(u8Status, tDisplayCtrl.u8CurrentDisplaySet);
					tDisplayCtrl.u8DispSeqStatus = u8Status;					
					tDisplayCtrl.bDiagnosisProtectLeve = false; /* Recover disable */
				}
				else
				{
		            if ((tDisplayCtrl.u8CurrentDisplaySet & BIT_DISP_EN_POS) == DISPLAY_DISABLE)
		            {
		                /* Delay time and disable back-light function */
						MBacklightControl_ExternalTurnOnOffBL(E_MBL_EXTERNAL_DISABLE_NODIMMNG);
		            }
					else
					{ /* Nothing */ }

					u8Status = DS_ACTION_NONE;
					u8Status = C_Display_Sequence_Control(u8Status, tDisplayCtrl.u8CurrentDisplaySet);
					tDisplayCtrl.u8DispSeqStatus = u8Status;
				}
				
			}
			else
			{
				Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
            }
			
            break;
        case EVENT_DISPLAY_CONTORL_DELAY :
			u8Status = C_Display_Sequence_Control(u8Status, tDisplayCtrl.u8CurrentDisplaySet);
			tDisplayCtrl.u8DispSeqStatus = u8Status;

            break;
        case EVENT_MESSAGE_SCANNING :
			if ((Memory_Pool_LcdStatus_Get() == DISPLAY_ON) && (Memory_Pool_LcdResetStatus_Get() == LCD_RESET_HIGH))
			{
            Memory_Pool_ScanStatus_Set(M_DM_ScanningControl(Memory_Pool_ScanStatus_Get(), Memory_Pool_DisplayScan_Get()));
			}			
			else
			{ /* Nothing */}

            break;
        case EVENT_MESSAGE_DIPLAY_REGISTER :
#if (U625_TDDI_TD7800)   			
            Memory_Pool_TD7800_Get(mu8Temp, sizeof(mu8Temp));
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
			Memory_Pool_NT51926_Get(mu8Temp, sizeof(mu8Temp));
#else
#endif
            C_Display_Management_RegContrl(mu8Temp);
            break;

        case EVENT_TIME_ATTN_POLLING :
#if (!M_DETECT_TCH_ATTN_EX_INT)
            tDisplayManageTask.u16Timer2 = TIME_2ms;
            MDetectTchAttn_Routine2ms();
#endif
        	break;
        case EVENT_TIME_INIT_POLLING :
			if(Memory_Pool_PowerStatus_Get() != POWER_OFF)
			{
				if((Memory_Pool_DisplayEnable_Get() == DISPLAY_ON_TOUCH_ON) || (Memory_Pool_DisplayEnable_Get()==DISPLAY_ON_TOUCH_OFF))
				{
	                u32CommDisplayStatus = Memory_Pool_DisplayStatus_Get();
					u32Temp = Memory_Pool_ActualDisplayStatus_Get();
                	Memory_Pool_DisplayStatus_Set(u32CommDisplayStatus | BIT_INIT_POS);
					Memory_Pool_ActualDisplayStatus_Set(u32Temp | BIT_INIT_POS);			
				}
				else
				{ /* Nothing */}
				tDisplayManageTask.u16Timer3 = TIME_2ms;
			}
			else
			{ /* Nothing */ }

			break;
        default:
        	/* Nothing */
            break;
    }
    Task_TaskDone();
}
/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Error(void)
 ;       Function Description	:	This state for error condition
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Display_Manage_Error(void)
{
    Task_TaskDone();
}

/* -- Global Functions -- */
/******************************************************************************
 ;       Function Name			:	void C_Display_Manage_Timer1(void)
 ;       Function Description	:	This function for timing using
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Display_Manage_Timer1(void)
{
    if (tDisplayManageTask.u16Timer1 > TIME_UP)
    {
        tDisplayManageTask.u16Timer1--;
        if (tDisplayManageTask.u16Timer1 == TIME_UP)
        {
            tDisplayManageTask.u16Timer1 = TIME_DISABLE;
            Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL3, EVENT_TIMER1);
        }
		else
		{ /* Nothing */ }
    }
	else
	{ /* Nothing */ }
}
/******************************************************************************
 ;       Function Name			:	void C_Display_Manage_Timer2(void)
 ;       Function Description	:	This function for timing using
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Display_Manage_Timer2(void)
{
    if (tDisplayManageTask.u16Timer2 > TIME_UP)
    {
        tDisplayManageTask.u16Timer2--;
        if (tDisplayManageTask.u16Timer2 == TIME_UP)
        {
            tDisplayManageTask.u16Timer2 = TIME_DISABLE;
            Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL3, EVENT_TIMER2);
        }
		else
		{ /* Nothing */ }
    }
	else
	{ /* Nothing */ }
}
/******************************************************************************
 ;       Function Name			:	void C_Display_Manage_Timer3(void)
 ;       Function Description	:	This function for timing using
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Display_Manage_Timer3(void)
{
    if (tDisplayManageTask.u16Timer3 > TIME_UP)
    {
        tDisplayManageTask.u16Timer3--;
        if (tDisplayManageTask.u16Timer3 == TIME_UP)
        {
            tDisplayManageTask.u16Timer3 = TIME_DISABLE;
            Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL3, EVENT_TIMER3);
        }
		else
		{ /* Nothing */ }
    }
	else
	{ /* Nothing */ }
}

void (*const Display_Manage_State_Machine[MAX_DM_STATE_NO])(void) =
{   C_Display_Manage_Init, C_Display_Manage_Control, C_Display_Manage_Error };
/* -- END -- */

