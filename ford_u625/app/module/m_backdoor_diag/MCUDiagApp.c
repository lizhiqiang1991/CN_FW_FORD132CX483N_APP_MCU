
#include "MCUDiagApp.h"
#include "M_FixedFlashAccess.h"

#if(BACKDOOR_ICDIAG_OPEN)

#define MCU_FLASH_ACCESS_FLASH_ADDRESS 0x0001E000U
#define MCU_FLASH_ACCESS_ROW_BYTE_SIZE CY_FLASH_SIZEOF_ROW /** < 256 Bytes. */
#define MCU_FLASH_ACCESS_ROW_SIZE 1U
#define MCU_FLASH_ACCESS_TOTLA_BYTE_SIZE (MCU_FLASH_ACCESS_ROW_BYTE_SIZE * MCU_FLASH_ACCESS_ROW_SIZE)

/* -- Global Variables -- */
static uint8_t MDiagFlashAccess_Buffer[MCU_FLASH_ACCESS_TOTLA_BYTE_SIZE] = {0x00};

/******************************************************************************
;	Function Name			:	MCUDIAG_MemRW
;	Function Description	:	
;	Parameters				:	
;	Return Values			:	
******************************************************************************/
void MCUDIAG_MemRW(uint8_t *u8DataAddr, uint8_t *u8TxData, uint8_t u8TxLen, uint8_t *u8RxData, uint8_t u8RxLen)
{
	uint8_t u8Index=0U;
	if(u8RxLen > 0U)
	{
		for(u8Index=0U; u8Index<u8RxLen; u8Index++)
		{
			*(u8RxData+u8Index)=*(u8DataAddr+u8Index);
		}
	}
	else
	{/*Nothing*/}

	if(u8TxLen > 0U)
	{
		for(u8Index=0U; u8Index<u8TxLen; u8Index++)
		{
			*(u8DataAddr+u8Index)=*(u8TxData+u8Index);
		}
	}
	else
	{/*Nothing*/}

	ICDIAG_SetCmdResault(ICDIAG_RESULT_SUCCESS);

	(void)u8Index;
}

/******************************************************************************
;	Function Name			:	MCUDIAG_NVMRW
;	Function Description	:	
;	Parameters				:	
;	Return Values			:	
******************************************************************************/
void MCUDIAG_NVMRW(uint32_t u32DataAddr, uint8_t *u8TxData, uint8_t u8TxLen, uint8_t *u8RxData, uint8_t u8RxLen)
{
	uint32_t u32FlashAddr=0U;
	
	if(u8RxLen > 0U)
	{		
		u32FlashAddr = MCU_FLASH_ACCESS_FLASH_ADDRESS + u32DataAddr;
		if((u32FlashAddr   > (MCU_FLASH_ACCESS_FLASH_ADDRESS + MCU_FLASH_ACCESS_ROW_BYTE_SIZE))\
        	|| ((MCU_FLASH_ACCESS_FLASH_ADDRESS + MCU_FLASH_ACCESS_ROW_BYTE_SIZE) < (u32FlashAddr +  (uint32_t)u8RxLen - 1U))\
        	|| (u8RxData == NULL))
    	{
			(void)memset(&u8RxData, 0xFFU, 64U);
			ICDIAG_SetCmdResault(ICDIAG_RESULT_FAIL);
        }
		else
		{
			__disable_irq();
			memcpy((void *)u8RxData, (const void *)(MCU_FLASH_ACCESS_FLASH_ADDRESS + u32DataAddr), u8RxLen);
			__enable_irq();
		}
	}
	else
	{/*Nothing*/}

	if(u8TxLen > 0U)
	{
		u32FlashAddr = MCU_FLASH_ACCESS_FLASH_ADDRESS + u32DataAddr;
		if((u32FlashAddr   > (MCU_FLASH_ACCESS_FLASH_ADDRESS + MCU_FLASH_ACCESS_ROW_BYTE_SIZE))\
        	|| ((MCU_FLASH_ACCESS_FLASH_ADDRESS + MCU_FLASH_ACCESS_ROW_BYTE_SIZE) < (u32FlashAddr +  (uint32_t)u8TxLen - 1U))\
        	|| (u8TxData == NULL))
    	{
			ICDIAG_SetCmdResault(ICDIAG_RESULT_FAIL);
        }
		else
		{
			(void)memset(&MDiagFlashAccess_Buffer, 0xFFU, sizeof(MDiagFlashAccess_Buffer));
			__disable_irq();
			/* Read Total Flash */
			memcpy(MDiagFlashAccess_Buffer, (const void *)MCU_FLASH_ACCESS_FLASH_ADDRESS, MCU_FLASH_ACCESS_TOTLA_BYTE_SIZE);
			/* Recover Page Data */
			memcpy((MDiagFlashAccess_Buffer + u32DataAddr), u8TxData, u8TxLen);
			/* Write Flash */
			if(CY_FLASH_DRV_SUCCESS != Cy_Flash_WriteRow(MCU_FLASH_ACCESS_FLASH_ADDRESS , (uint32_t *)MDiagFlashAccess_Buffer))
			{
				ICDIAG_SetCmdResault(ICDIAG_RESULT_FAIL);
			}
			else
			{
				ICDIAG_SetCmdResault(ICDIAG_RESULT_SUCCESS);
			}
			__enable_irq();
		}
	}
	else
	{/*Nothing*/}		

}

/******************************************************************************
;	Function Name			:	I2cMasterApp_ICDiag
;	Function Description	:	
;	Parameters				:	
;	Return Values			:	true:cmd success, false:cmd fail
******************************************************************************/
void MCUDIAG_I2cMaster_ICDiag(uint8_t u8DeviceAddr, uint8_t u8Channel, uint8_t *u8TxData, uint8_t u8TxLen, uint8_t *u8RxData, uint8_t u8RxLen)
{
	bool blStatus=true;
	uint8_t u8ChannelCheckPass=1U;
	uint16_t u16TxLen=(uint16_t)u8TxLen;
	uint16_t u16RxLen=(uint16_t)u8RxLen;

    i2c_master_typedef tI2CMaster;
    tI2CMaster.pBase = I2C_MASTER_HW;
    tI2CMaster.pConfig = &I2C_MASTER_config;
	
	if(u8Channel == 1U)
	{/*Nothing*/}
	else
	{
		u8ChannelCheckPass=0U;
	}

	if(u8ChannelCheckPass == 1U)
	{
		/*Read after write*/
		if(u16RxLen > 0U)
		{
			if(HAL_I2C_Master_Write(u8DeviceAddr, u8TxData, u16TxLen, 100U) == DRIVER_TRUE)
			{
				if(HAL_I2C_Master_Read(u8DeviceAddr, u8RxData, u16RxLen, 100U) == DRIVER_TRUE)
				{/*Nothing*/}
				else
				{
					(void)HAL_I2C_Master_Init(tI2CMaster);
					blStatus=false;					
				}
			}
			else
			{
				(void)HAL_I2C_Master_Init(tI2CMaster);
				blStatus=false;					
			}
		}
		/*Write only*/
		else
		{
			if(HAL_I2C_Master_Write(u8DeviceAddr, u8TxData, u16TxLen, 100U) == DRIVER_TRUE)
			{/*Nothing*/}
			else
			{
				(void)HAL_I2C_Master_Init(tI2CMaster);
				blStatus=false;
			}
		}

		if(blStatus == true)
		{
			ICDIAG_SetCmdResault(ICDIAG_RESULT_SUCCESS);
		}
		else
		{
			ICDIAG_SetCmdResault(ICDIAG_RESULT_FAIL);
		}
	}
	else
	{
		ICDIAG_SetCmdResault(ICDIAG_RESULT_FAIL);
	}
}
#endif



