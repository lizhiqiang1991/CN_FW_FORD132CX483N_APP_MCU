/* -- Includes -- */
#include "C_Communication.h"
#include "C_Power_Management.h"
#include "C_Display_Management.h"
#include "C_Diagnosis.h"
#include "Memory_Pool.h"
#include "M_INTB.h"
#include "M_BacklightControl.h"
#include "M_FPNCtrl.h"
#include "M_FixedFlashAccess.h"
#include "main.h"
#include "public.h"
#include "ICDiagApp.h"

/* -- Marco Define -- */
#define CCOMMUNICATION_EEPROM_FACTORYDATA_USED_SIZE 25U
#define CCOMMUNICATION_EEPROM_FACTORYDATA_RESERVED_SIZE 7U
#define CCOMMUNICATION_EEPROM_FACTORYDATA_SIZE (CCOMMUNICATION_EEPROM_FACTORYDATA_USED_SIZE + CCOMMUNICATION_EEPROM_FACTORYDATA_RESERVED_SIZE)
#define CCOMMUNICATION_EEPROM_FACTORYDATA_NUMBER 4U
#define CCOMMUNICATION_EEPROM_DATA_SIZE (CCOMMUNICATION_EEPROM_FACTORYDATA_SIZE * CCOMMUNICATION_EEPROM_FACTORYDATA_NUMBER)
   
/* -- Type Define -- */
typedef struct
{
	uint8_t u8UsedData[CCOMMUNICATION_EEPROM_FACTORYDATA_USED_SIZE];
	uint8_t u8Reserved[CCOMMUNICATION_EEPROM_FACTORYDATA_RESERVED_SIZE];
}CCommunication_FactoryData;

/* -- Type Define -- */
typedef struct
{
	uint8_t u8UsedData[CCOMMUNICATION_EEPROM_FACTORYDATA_USED_SIZE];
	uint8_t u8Reserved[CCOMMUNICATION_EEPROM_FACTORYDATA_RESERVED_SIZE];
}CCommunication_FactoryData2;

typedef union
{
	uint8_t u8Byte[CCOMMUNICATION_EEPROM_DATA_SIZE];
	struct
	{
		CCommunication_FactoryData DeliveryData;
		CCommunication_FactoryData SNData;
		CCommunication_FactoryData DisplayIDData;
		CCommunication_FactoryData ProductionPhaseData;
	}FactoryData;
}CCommunication_AllFactoryData;

/* -- Global Variables -- */
static tcommunication_task_def tCommunicationTask;
static uint8_t mu8RxBuff[BUFFER_SIZE];
static uint8_t mu8TxBuff[BUFFER_SIZE];
static uint8_t mu8BackupTxBuff[BUFFER_SIZE];
volatile uint16_t *pUpdateKey = ((volatile uint16_t*) (0x20003F00));

#if (U717_TDDI_NT51926)
	const uint8_t mu8Core[] = { "RU5T-14F180-SA" };
	const uint8_t mu8Soft[] = { "RU5T-14D358-SA" };
	const uint8_t mu8Main[] = { "RU5T-14D359-SA" };    
#elif(CX430_TDDI_NT51926)
	const uint8_t mu8Core[] = { "SU5T-14F180-PA" };
	const uint8_t mu8Soft[] = { "SU5T-14D358-PA" };
	const uint8_t mu8Main[] = { "SU5T-14D359-PA" };
#else
#endif
/* -- Local Functions -- */
/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Communication_EEPROM_Read(void)
{
	uint8_t u8FactoryDataIndex = 0;
	uint32_t u32FactoryDataOffset = 0;
	CCommunication_AllFactoryData cCommunicaiotnAllFactoryData;

	memset(cCommunicaiotnAllFactoryData.u8Byte , 0x00U , sizeof(CCommunication_AllFactoryData));
	/* Read All Factory Data */
	for(u8FactoryDataIndex = 0 ; u8FactoryDataIndex < CCOMMUNICATION_EEPROM_FACTORYDATA_NUMBER ; u8FactoryDataIndex++)
	{
		u32FactoryDataOffset = (uint32_t)(u8FactoryDataIndex) * (uint32_t)(CCOMMUNICATION_EEPROM_FACTORYDATA_SIZE);
		MFixedFlashAccess_ReadPage(u32FactoryDataOffset, (cCommunicaiotnAllFactoryData.u8Byte + u32FactoryDataOffset), CCOMMUNICATION_EEPROM_FACTORYDATA_SIZE);
	}

	/* Update Data to Memory Pool */
	Memory_Pool_FPNCore_Set(mu8Core, sizeof(mu8Core));
	Memory_Pool_FPNDelivery_Set(cCommunicaiotnAllFactoryData.FactoryData.DeliveryData.u8UsedData, sizeof(cCommunicaiotnAllFactoryData.FactoryData.DeliveryData.u8UsedData));
	Memory_Pool_FPNSoft_Set(mu8Soft , sizeof(mu8Soft));
	Memory_Pool_FPNSerial_Set(cCommunicaiotnAllFactoryData.FactoryData.SNData.u8UsedData , sizeof(cCommunicaiotnAllFactoryData.FactoryData.SNData.u8UsedData));
	Memory_Pool_FPNMain_Set(mu8Main , sizeof(mu8Main));
	Memory_Pool_FPNID_Set(cCommunicaiotnAllFactoryData.FactoryData.DisplayIDData.u8UsedData, sizeof(cCommunicaiotnAllFactoryData.FactoryData.DisplayIDData.u8UsedData));
	Memory_Pool_FPNProductionPhase_Set(cCommunicaiotnAllFactoryData.FactoryData.ProductionPhaseData.u8UsedData[0]);
}
/******************************************************************************
 ;       Function Name			:	void C_TD7800_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Communication_EEPROM_Write(uint8_t u8Case)
{
	tFPN_ctrl_def tFPNCtrlDelivery = {NUMBER_ZERO};
	tFPN_ctrl_def tFPNCtrlSerNum = {NUMBER_ZERO};
	tFPN_production_byte_def tFPNProductionByte = {NUMBER_ZERO};

	uint8_t u8FPN_Control_Status = NUMBER_ZERO;

	switch (u8Case)
	{
		default:
			/* Nothing */
		break;

		case DELIVERY_ASSEMBLY:
			/* Set object */
			tFPNCtrlDelivery.FPN_Flag_t.FPNCtrlFlag = (uint8_t)Memory_Pool_WriteFPNDeliveryStatusReg_Get();
			tFPNCtrlDelivery.FPN_Flag_t.FPNDataFlag = (uint8_t)Memory_Pool_WriteFPNDelivery_Get();
			Memory_Pool_FPNDeliverystatusInfo_Get(&tFPNCtrlDelivery.FPN_Status_t);
			tFPNCtrlDelivery.FPNDataLength = FPN_SIZE;
			tFPNCtrlDelivery.pFPNDataBuff = Memory_Pool_pdelivery_assembly_Get();

			/* excute FPN process */
			u8FPN_Control_Status = M_FPNCtrl_DeliveryFPNControl_Process(&tFPNCtrlDelivery);
			if(u8FPN_Control_Status == FPN_WRITE_SUCCESS)
			{   
				Memory_Pool_WriteFPNDeliveryStatusReg_Set(false);
				Memory_Pool_WriteFPNDelivery_Set(false);
				tFPNCtrlDelivery.FPN_Status_t.INT_WRT = FPN_ENABLE;
				Memory_Pool_FPNDeliverystatusInfo_Set(&tFPNCtrlDelivery.FPN_Status_t);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else if(u8FPN_Control_Status == FPN_CHECKSUM_ERR)
			{
				Memory_Pool_WriteFPNDeliveryStatusReg_Set(false);
				Memory_Pool_WriteFPNDelivery_Set(false);
				tFPNCtrlDelivery.FPN_Status_t.CKSUM_ERR = FPN_ENABLE;
				Memory_Pool_FPNDeliverystatusInfo_Set(&tFPNCtrlDelivery.FPN_Status_t);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else if(u8FPN_Control_Status == FPN_WRITE_NO_DATA)
			{   
				Memory_Pool_WriteFPNDeliveryStatusReg_Set(false);
				Memory_Pool_WriteFPNDelivery_Set(false);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else
			{ /* Nothing */ }

		break;
			
		case SERIAL_NUMBER:
			/* Set object */
			tFPNCtrlSerNum.FPN_Flag_t.FPNCtrlFlag = (uint8_t)Memory_Pool_WriteSerNumPNStatusReg_Get();
			tFPNCtrlSerNum.FPN_Flag_t.FPNDataFlag = (uint8_t)Memory_Pool_WriteSerNumFPN_Get();
			Memory_Pool_FPNSerialstatusInfo_Get(&tFPNCtrlSerNum.FPN_Status_t);
			tFPNCtrlSerNum.FPNDataLength = FPN_SIZE;
			tFPNCtrlSerNum.pFPNDataBuff = Memory_Pool_pSerNum_assembly_Get();

			/* excute FPN process */
			u8FPN_Control_Status = M_FPNCtrl_SerNumFPNControl_Process(&tFPNCtrlSerNum);
			if(u8FPN_Control_Status == FPN_WRITE_SUCCESS)
			{   
				Memory_Pool_WriteSerNumPNStatusReg_Set(false);
				Memory_Pool_WriteSerNumFPN_Set(false);
				tFPNCtrlSerNum.FPN_Status_t.INT_WRT = FPN_ENABLE;
				Memory_Pool_FPNSerialstatusInfo_Set(&tFPNCtrlSerNum.FPN_Status_t);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else if(u8FPN_Control_Status == FPN_CHECKSUM_ERR)
			{   
				Memory_Pool_WriteSerNumPNStatusReg_Set(false);
				Memory_Pool_WriteSerNumFPN_Set(false);
				tFPNCtrlSerNum.FPN_Status_t.CKSUM_ERR = FPN_ENABLE;
				Memory_Pool_FPNSerialstatusInfo_Set(&tFPNCtrlSerNum.FPN_Status_t);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else if(u8FPN_Control_Status == FPN_WRITE_NO_DATA)
			{   
				Memory_Pool_WriteSerNumPNStatusReg_Set(false);
				Memory_Pool_WriteSerNumFPN_Set(false);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}			
			else
			{ /* Nothing */ } 
		break;
			
		case PRODUCTION_PHASE_BYTE:
			/* Set object */
			tFPNProductionByte.FPN_Flag_t.FPNCtrlFlag = (uint8_t)Memory_Pool_WriteProductPhasePNStatusReg_Get();
			tFPNProductionByte.FPN_Flag_t.FPNDataFlag = (uint8_t)Memory_Pool_WriteProductPhaseFPN_Get();
			Memory_Pool_FPNSProductPhaseStatusInfo_Get(&tFPNProductionByte.FPN_Status_t);
			tFPNProductionByte.FPNDataLength = FPN_BYTE_SIZE;
			tFPNProductionByte.pFPNDataBuff = Memory_Pool_pProductionPhase_Get();

			/* excute FPN process */
			u8FPN_Control_Status = M_FPNCtrl_ProductionByteFPNControl_Process(&tFPNProductionByte);
			if(u8FPN_Control_Status == FPN_WRITE_SUCCESS)
			{   
				Memory_Pool_WriteProductPhasePNStatusReg_Set(false);
				Memory_Pool_WriteProductPhaseFPN_Set(false);
				tFPNProductionByte.FPN_Status_t.INT_WRT = FPN_ENABLE;
				Memory_Pool_FPNSProductPhaseStatusInfo_Set(&tFPNProductionByte.FPN_Status_t);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}
			else if(u8FPN_Control_Status == FPN_WRITE_NO_DATA)
			{   
				Memory_Pool_WriteProductPhasePNStatusReg_Set(false);
				Memory_Pool_WriteProductPhaseFPN_Set(false);
				(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_START_INTB_STRATEGY);
			}			
			else
			{ /* Nothing */ } 
		break;
	}
}
/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Communication_Event_Assign(uint8_t u8Message)
{
	uint8_t u8Temp;

	switch (u8Message)
	{
		case CMD_BACKLIGHT_PWM:
			(void)Task_ChangeEvent(TYPE_BACKLIGHT_MANAGE, LEVEL4, EVENT_MESSAGE);
		break;
		
		case CMD_DISPLAY_SCANNING:
			(void)Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_SCANNING);
		break;
		
		case CMD_DISPLAY_ENABLE:
			u8Temp = Memory_Pool_DisplayEnable_Reg_Get();
            if(u8Temp != DISPLAY_OFF_TOUCH_ON)
            {
			if ((u8Temp & BIT_DISP_EN_POS) == DISPLAY_ENABLE)
			{                
				Memory_Pool_BacklightEnable_Set(true);
				Memory_Pool_DiagnosisEnable_Set(true);
			}
			else
			{
				Memory_Pool_BacklightEnable_Set(false);
				Memory_Pool_DiagnosisEnable_Set(false);
			}
			Memory_Pool_DisplayEnable_Set(u8Temp);

			(void)Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
			(void)Task_ChangeEvent(TYPE_DIAGNOSIS, LEVEL4, EVENT_MESSAGE_DISANOSIS_ENABLE);
            }
            else
			{/*Nothing*/}
		
            break;
		case CMD_DISPLAY_SHUTDOWN:
			u8Temp = Memory_Pool_Shutdown_Get();
			if ((u8Temp & BIT_SHDWN_POS) == SHUTDOWN_ENABLE)
			{
				Memory_Pool_PowerState_Set(SHUTDOWN1OR2_STATE);
				Memory_Pool_BacklightEnable_Set(false);
				(void)Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
			}
			else
			{/* Nothing */ }
		break;		
		case CMD_FACTORY_MODE:
			if (Memory_Pool_FactoryMode_Get() == NORMAL_MODE)
			{
				Memory_Pool_PowerState_Set(NORMAL_RUN_STATE);
				Memory_Pool_PowerStatus_Set(POWER_ON_READY);
				(void)Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
			}
			else if (Memory_Pool_FactoryMode_Get() == OTP_MODE)
			{
				/*In standby mode, Switch to OTP mode. OTP mode need to enable back-light function . */
				Memory_Pool_BacklightEnable_Set(true);
				(void)Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
				/* Disable diagnosis functions*/
				Memory_Pool_DiagnosisEnable_Set(false);
				(void)Task_ChangeEvent(TYPE_DIAGNOSIS, LEVEL5, EVENT_MESSAGE_DISANOSIS_ENABLE);
			}
			else if (Memory_Pool_FactoryMode_Get() == TPT_MODE)
			{
				/*Switch to TPT mode. TPT mode need to enable back-light function and wait display enable. */
				Memory_Pool_BacklightEnable_Set(true);
				(void)Task_ChangeEvent(TYPE_DISPLAY_MANAGE, LEVEL4, EVENT_MESSAGE_DISPLAY_ENABLE);
				/* Disable diagnosis functions*/
				Memory_Pool_DiagnosisEnable_Set(false);
				(void)Task_ChangeEvent(TYPE_DIAGNOSIS, LEVEL5, EVENT_MESSAGE_DISANOSIS_ENABLE);
			}
			else if (Memory_Pool_FactoryMode_Get() == NORMAL_DISALBE_MODE)
			{
				Memory_Pool_PowerState_Set(NORMAL_DISABLE_STATE);
				Memory_Pool_PowerStatus_Set(POWER_ON_READY);
				(void)Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
			}
			else
			{ /* Nothing */ }
		break;
			
		case CMD_DERATING_ENABLE:
			(void)Task_ChangeEvent(TYPE_BACKLIGHT_MANAGE, LEVEL4, EVENT_MESSAGE1);
		break;
			
		case CMD_UPDATE_REQUEST:
			*pUpdateKey = 0x55AAU;
			Memory_Pool_SoftwareReset_Set(true);
		break;
		
		case CMD_LOCK_DELIVERY_ASSEMBLY:    
		case CMD_DELIVERY_ASSEMBLY_DATA:
		case CMD_LOCK_SERIAL_NUMBER:
		case CMD_SERIAL_NUMBER_DATA:
		case CMD_LOCK_PRODUCTION_PHASE_BYTE:
		case PRODUCTION_PHASE_BYTE:
			(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL4, EVENT_MESSAGE_EEPROM_CONTROL);
		break;
		
		default:
			/* Nothing */
		break;
	}
}
/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Communication_Callback(uint32_t u32I2cEvent)
{
	uint8_t u8DataLength = 0U;
	uint8_t u8MSGFormatCheckResult=FORMAT_TO_BE_CHECK;

	switch (u32I2cEvent)
	{
		case CY_SCB_I2C_SLAVE_READ_EVENT:
			/* Nothing */
		break;

		case CY_SCB_I2C_SLAVE_WRITE_EVENT:
			/* Nothing */
		break;

		case CY_SCB_I2C_SLAVE_RD_IN_FIFO_EVENT:
			/* Nothing */
		break;

		case CY_SCB_I2C_SLAVE_RD_BUF_EMPTY_EVENT:
			/* Nothing */
		break;

		case CY_SCB_I2C_SLAVE_ERR_EVENT:
			/* Nothing */
		break;

		/* Receive data complete */
		case CY_SCB_I2C_SLAVE_WR_CMPLT_EVENT:
			/* Check communication enable. */
			if(Memory_Pool_CommunicationMask_Get() == false)
			{
				/* Check message format. */
				u8DataLength = M_COM_RxLength_Get();
				u8MSGFormatCheckResult = M_COM_RxMSGFormat_Check(mu8RxBuff[CMD_SUBADDRESS_POS], u8DataLength);

				if(u8MSGFormatCheckResult == FORMAT_UNSUPPORT_SUBADDRESS)
				{		
					/* Clear all data. */
					memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
					/* Restore subaddress for echo purpose. */
					mu8TxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];
					/* Calculated the rolling counter and CRC8. */
					M_COM_TxBufferData_Set(mu8TxBuff, LEN_SUBADDRESS);
					/* MCU returns data. The first data is sub-address echo and the other data are 0xFF. No rolling counter and CRC8 */
					M_COM_TxBuffer_Config(mu8TxBuff, BUFFER_SIZE);					
				}
				else if(u8MSGFormatCheckResult == FORMAT_LEN_FAIL)
				{
					/* Clear all data. */
					memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
				}
				else if(u8MSGFormatCheckResult == FORMAT_READ_CHECK_SAFETY)
				{
					if(mu8RxBuff[CMD_SUBADDRESS_POS] == CMD_JUMP_TO_BOOTLOADER)
					{
						u8MSGFormatCheckResult=FORMAT_READ_FORMAT_CORRECT;
						/* Clear all data. */
						memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
						/* Restore subaddress for echo purpose. */
						mu8TxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];
						/* Fetch the information from memory pool. */
						Memory_Pool_Command_Info_Fetch(mu8TxBuff, &u8DataLength);
						/* Calculated the rolling counter and CRC8 or checksum. */
						M_COM_TxBufferData_Set(mu8TxBuff, u8DataLength);
						/* MCU returns data.*/
						M_COM_TxBuffer_Config(mu8TxBuff, BUFFER_SIZE);
						Memory_Pool_PowerState_Set(SHUTDOWN1OR2_STATE);
						(void)Task_ChangeEvent(TYPE_POWER_MANAGE, LEVEL4, EVENT_MESSAGE);
					}
					else if(M_COM_RxMSGCRC8Value_Check(mu8RxBuff, u8DataLength) == false)
					{
						u8MSGFormatCheckResult=FORMAT_READ_CRC_FAIL;
						/* Clear all data. */
						memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
					}
					else if(M_COM_RxRollingCounter_Check(mu8RxBuff[u8DataLength-(LEN_CRC8+LEN_ROLLING_COUNTER)]) == false)
					{
						u8MSGFormatCheckResult=FORMAT_READ_RC_FAIL;
						/* Fetch the information from memory pool. */
						Memory_Pool_Command_Info_Fetch(mu8BackupTxBuff, &u8DataLength);
						/* Calculated the rolling counter and CRC8. */
						M_COM_TxBufferData_Set(mu8BackupTxBuff, u8DataLength);
						/* MCU returns data. The data are previously by MCU returning successfully. */
						M_COM_TxBuffer_Config(mu8BackupTxBuff, BUFFER_SIZE);
					}
					else
					{
						u8MSGFormatCheckResult=FORMAT_READ_FORMAT_CORRECT;
						/* Backup command for error handling use if next command format rolling counter error */
						mu8BackupTxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];

						/* Clear all data. */
						memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
						/* Copy sub-address */
						mu8TxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];
						/* Fetch the information from memory pool. */
						Memory_Pool_Command_Info_Fetch(mu8TxBuff, &u8DataLength);
						/* Calculated the rolling counter and CRC8 or checksum. */
						M_COM_TxBufferData_Set(mu8TxBuff, u8DataLength);
						/* MCU returns data.*/
						M_COM_TxBuffer_Config(mu8TxBuff, BUFFER_SIZE);
					}
				}
				else if(u8MSGFormatCheckResult == FORMAT_WRITE_CHECK_SAFETY)
				{
					if(mu8RxBuff[CMD_SUBADDRESS_POS] == CMD_UPDATE_REQUEST)
					{
						if((Common_Checksum_Calculation(mu8RxBuff, (u8DataLength-LEN_CHECKSUM)) + 0x01U) == mu8RxBuff[u8DataLength - 1])
						{
							u8MSGFormatCheckResult=FORMAT_WRITE_FORMAT_CORRECT;
							/* MCU will execute the command from host */
							Memory_Pool_Command_Info_Assign(mu8RxBuff);
							C_Communication_Event_Assign(mu8RxBuff[CMD_SUBADDRESS_POS]);
						}
						else
						{
							u8MSGFormatCheckResult=FORMAT_WRITE_CRC_FAIL;
						}
					}
					else if(M_COM_RxMSGCRC8Value_Check(mu8RxBuff, u8DataLength) == false)
					{
						u8MSGFormatCheckResult=FORMAT_WRITE_CRC_FAIL;
					}
					else if(M_COM_RxRollingCounter_Check(mu8RxBuff[u8DataLength-(LEN_CRC8+LEN_ROLLING_COUNTER)]) == false)
					{
						u8MSGFormatCheckResult=FORMAT_WRITE_RC_FAIL;
					}
					else
					{
						u8MSGFormatCheckResult=FORMAT_WRITE_FORMAT_CORRECT;
						/* Backup command for error handling use if next command format rolling counter error */
						mu8BackupTxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];

						/* MCU will execute the command from host */
						Memory_Pool_Command_Info_Assign(mu8RxBuff);
						C_Communication_Event_Assign(mu8RxBuff[CMD_SUBADDRESS_POS]);

						/* Set TX buffer if repeat start read*/
						/* Clear all data. */
						memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
						/* Copy sub-address */
						mu8TxBuff[CMD_SUBADDRESS_POS] = mu8RxBuff[CMD_SUBADDRESS_POS];
						/* Fetch the information from memory pool. */
						Memory_Pool_Command_Info_Fetch(mu8TxBuff, &u8DataLength);
						/* Calculated the rolling counter and CRC8 or checksum. */
						M_COM_TxBufferData_Set(mu8TxBuff, u8DataLength);
						/* MCU returns data.*/
						M_COM_TxBuffer_Config(mu8TxBuff, BUFFER_SIZE);
					}
				}
				else
				{/* Nothing */ }
			}
			else
			{/* Nothing */ }
			memset(mu8RxBuff, 0xFFU, BUFFER_SIZE);
			M_COM_RxBuffer_Config(mu8RxBuff, BUFFER_SIZE);
		break;

		/* Transmit data complete */
		case CY_SCB_I2C_SLAVE_RD_CMPLT_EVENT:
			/* Nothing */
		break;

		case CY_SCB_I2C_MASTER_WR_IN_FIFO_EVENT:
			/* Nothing */
		break;

		default:
			/* Nothing */
		break;
	}
}

/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Communication_ParaInit(void)
{
	memset(mu8TxBuff, 0xFFU, BUFFER_SIZE);
	memset(mu8RxBuff, 0xFFU, BUFFER_SIZE);
	memset(mu8BackupTxBuff, 0xFFU, BUFFER_SIZE);

	Memory_Pool_CommunicationMask_Set(false);
	Memory_Pool_I2CDesInit_Set(false);
	Memory_Pool_VBattProtectState_Set(false);
	Memory_Pool_WriteFPNDeliveryStatusReg_Set(false);
	Memory_Pool_WriteFPNDelivery_Set(false);
	Memory_Pool_WriteSerNumPNStatusReg_Set(false);
	Memory_Pool_WriteSerNumFPN_Set(false);
	Memory_Pool_WriteProductPhasePNStatusReg_Set(false);
	Memory_Pool_WriteProductPhaseFPN_Set(false);

	*pUpdateKey = NUMBER_ZERO;

	tCommunicationTask.u16Timer1 = TIME_DISABLE;
	tCommunicationTask.u16Timer2 = TIME_DISABLE;
	tCommunicationTask.u32Timeout = TIME_DISABLE;

	/* Registers INTB Module */
	MINTB_Register(M_DM_INTB_Ctrl,INTB_INT_TYPE_FALLING);

#if(BACKDOOR_ICDIAG_OPEN)
	ICDIAG_Initialize();
#endif
}
/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Init(void)
 ;       Function Description	:	This state will do power management initialize
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Communiction_Init(void)
{
	switch (Task_Current_Event_Get())
	{
		case EVENT_FIRST :
			C_Communication_ParaInit();
			tCommunicationTask.u16Timer1 = TIME_2ms;
		break;

		case EVENT_TIMER_I2C_DES_INIT_DELAY :
			/* Check power ready and MCU is in the normal run mode */
			if (Memory_Pool_PowerState_Get() == NORMAL_RUN_STATE)
			{
				/* Power is ready and start I2C Initialize. */
				M_COM_I2cInit(mu8TxBuff, BUFFER_SIZE, mu8RxBuff, BUFFER_SIZE, (cy_cb_scb_i2c_handle_events_t) C_Communication_Callback);
				Memory_Pool_I2CDesInit_Set(true);
#if(DEBUG_POWER_UP)
				HAL_GPIO_Toggle( U301_INTB_IN_PORT,  U301_INTB_IN_PIN); 
#endif
				__disable_irq();
				C_Communication_EEPROM_Read();  /* Do this function use 91us */
				__enable_irq();
				tCommunicationTask.u16Timer1 = TIME_DISABLE;
#if(DEBUG_POWER_UP)				
				HAL_GPIO_Toggle( U301_INTB_IN_PORT,  U301_INTB_IN_PIN); 
#endif  				
				/* Changes state from STATE_COMMUNICATION_INIT to STATE_COMMUNICATION_PROCESS  */
				(void)Task_ChangeState(TYPE_COMMUNICATION, LEVEL5, STATE_COMMUNICATION_PROCESS, Communication_State_Machine[STATE_COMMUNICATION_PROCESS]);
			}
			else
			{
				/* Delay 2ms and retry again. */
				tCommunicationTask.u16Timer1 = TIME_2ms;
			}
		break;

		default:
		/* Nothing */
		break;
	}

	Task_TaskDone();
}
/******************************************************************************
 ;       Function Name			:	void C_Power_Manager_Control(void)
 ;       Function Description	:	This state will do power on/off sequence
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
static void C_Communiction_Process(void)
{
	switch (Task_Current_Event_Get())
	{
		case EVENT_FIRST :
			/* Enable EVENT_TIMER_INTB_ROUNTINE */
			tCommunicationTask.u16Timer2 = TIME_2ms;
			tCommunicationTask.u16Timer1 = TIME_DISABLE;
		break;
		
		case EVENT_MESSAGE_EEPROM_CONTROL :
			C_Communication_EEPROM_Write(Memory_Pool_EppromIndex_Get());
		break;

		case EVENT_MESSAGE_START_INTB_STRATEGY:
			MINTB_StrategyControl(STRATEGY_CTRL_START);
		break;

		case EVENT_MESSAGE_CANCEL_INTB_STRATEGY:
			MINTB_StrategyControl(STRATEGY_CTRL_CANCEL);
		break;

		case EVENT_TIMER_INTB_ROUNTINE:
			tCommunicationTask.u16Timer2 = TIME_2ms;
			MINTB_Routine((uint16_t)(TIME_2ms - 1));
		break;

		default:
			(void)Task_ChangeState(TYPE_COMMUNICATION, LEVEL5, STATE_COMMUNICATION_ERROR, Communication_State_Machine[STATE_COMMUNICATION_ERROR]);
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
static void C_Communiction_Error(void)
{
	Task_TaskDone();
}

/* -- Global Functions -- */
/******************************************************************************
 ;       Function Name			:	void C_Power_Manage_Timer1(void)
 ;       Function Description	:	This function for timing using
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void C_Communication_Timer1(void)
{
	if (tCommunicationTask.u16Timer1 > TIME_UP)
	{
		tCommunicationTask.u16Timer1--;
		if (tCommunicationTask.u16Timer1 == TIME_UP)
		{
			tCommunicationTask.u16Timer1 = TIME_DISABLE;
			(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL3, EVENT_TIMER1);
		}
		else
		{ /* Nothing */ }
	}
	else
	{ /* Nothing */ }
}
/******************************************************************************
 ;       Function Name          :   void C_Power_Manage_Timer1(void)
 ;       Function Description   :   This function for timing using
 ;       Parameters             :   void
 ;       Return Values          :   void
 ;      Source ID               :
 ******************************************************************************/
void C_Communication_Timer2(void)
{
	if (tCommunicationTask.u16Timer2 > TIME_UP)
	{
		tCommunicationTask.u16Timer2--;
		if (tCommunicationTask.u16Timer2 == TIME_UP)
		{
			tCommunicationTask.u16Timer2 = TIME_DISABLE;
			(void)Task_ChangeEvent(TYPE_COMMUNICATION, LEVEL3, EVENT_TIMER2);
		}
		else
		{ /* Nothing */ }
	}
	else
	{ /* Nothing */ }
}

void (*const Communication_State_Machine[MAX_COM_STATE_NO])(void) =
{   C_Communiction_Init, C_Communiction_Process, C_Communiction_Error };
/* -- End -- */
