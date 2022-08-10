#include "M_GPIOSense.h"
#include "M_DisplayManage.h"
#if (BACKDOOR_DIAGNOSIS_SIMULATE)
#include "Memory_Pool.h"
#endif

/******************************************************************************
 ;       Function Name			:	void Main_I2cSlaveInit(void)
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
bool M_GPIOSense_LevelDeboucne(GPIO_PRT_Type *pPort, uint8_t u8Pin, tgpio_debounce_def *ptDebounce)
{
    uint8_t u8IOLevel;
	
    HAL_GPIO_Read(pPort, u8Pin, &u8IOLevel);
#if(BACKDOOR_DIAGNOSIS_SIMULATE)
	if((pPort == U301_LED_INT_PORT) && (u8Pin == U301_LED_INT_PIN))
	{
		u8IOLevel = Memory_Pool_DiagnosisSimulateInfo_Get().u8LedINTPinLevel;
	}
	else if((pPort == U301_DISP_FAULT_PORT) && (u8Pin == U301_DISP_FAULT_PIN))
	{
		u8IOLevel = Memory_Pool_DiagnosisSimulateInfo_Get().u8DispFaultPinLevel;
	}
	else if((pPort == U301_LOCK_PORT) && (u8Pin == U301_LOCK_PIN))
	{
		u8IOLevel = Memory_Pool_DiagnosisSimulateInfo_Get().u8LockPinLevel;
	}
	else if((pPort == U301_P1V2_PGOOD_PORT) && (u8Pin == U301_P1V2_PGOOD_PIN))
	{
		u8IOLevel = Memory_Pool_DiagnosisSimulateInfo_Get().u8LockPinLevel;
	}
	else if((pPort == U301_P3V3_PGOOD_PORT) && (u8Pin == U301_P3V3_PGOOD_PIN))
	{
		u8IOLevel = Memory_Pool_DiagnosisSimulateInfo_Get().u8LockPinLevel;
	}
	else
	{/* Nothing */}
#endif

    return Common_LevelDebounce(u8IOLevel, ptDebounce);
}
/******************************************************************************
 ;       Function Name			:	void Main_I2cSlaveInit(void)
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
uint64_t M_GPIOSense_LED_Driver_Diagnosis(void)
{
    int8_t i8Counter;
    uint8_t u8SendData;
    uint8_t mu8Temp[6U];
	uint64_t u64ReturnStatus=0UL;
    uint64_t u64Temp=0UL;

    /* Get the supply status of LED driver*/
    u8SendData = 0x0EU;
    HAL_I2C_Master_Write(LP8864_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
    HAL_I2C_Master_Read(LP8864_ADDRESS, &mu8Temp[0U], 2U, 100U);

    mu8Temp[0U] = mu8Temp[0U] & 0xAA;
    mu8Temp[1U] = mu8Temp[1U] & 0xAA;

    /* Get the supply status of LED driver*/
    u8SendData = 0x10U;
    HAL_I2C_Master_Write(LP8864_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
    HAL_I2C_Master_Read(LP8864_ADDRESS, &mu8Temp[2U], 2U, 100U);

    mu8Temp[2U] = mu8Temp[0U] & 0xAA;
    mu8Temp[3U] = mu8Temp[1U] & 0xAA;

    /* Get the supply status of LED driver*/
    u8SendData = 0x12U;
    HAL_I2C_Master_Write(LP8864_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
    HAL_I2C_Master_Read(LP8864_ADDRESS, &mu8Temp[4U], 2U, 100U);

    mu8Temp[4U] = mu8Temp[4U] & 0xCF;
    mu8Temp[5U] = mu8Temp[5U] & 0x55;

    for ( i8Counter = 5; i8Counter >= 0; i8Counter-- )
    {
        u64Temp = mu8Temp[i8Counter];
        u64Temp <<= (i8Counter * 8);
        u64ReturnStatus|=u64Temp;
    }
#if (BACKDOOR_DIAGNOSIS_SIMULATE)
	u64ReturnStatus = Memory_Pool_DiagnosisSimulateInfo_Get().u64LedFaultStatus;
#endif
	
	return u64ReturnStatus;
}
/******************************************************************************
 ;       Function Name			:	
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
#if 0
void M_GPIOSense_LED_Driver_DiagClear(void)
{
	uint8_t u8Temp[7];
	/* Clear All error of LED driver. */
	u8Temp[0]=0x0EU;
	u8Temp[1]=0xFFU;
	u8Temp[2]=0xFFU;
	u8Temp[3]=0xFFU;
	u8Temp[4]=0xFFU;
	u8Temp[5]=0x00U;
	u8Temp[6]=0x7EU;
	HAL_I2C_Master_Write(LP8864_ADDRESS, &u8Temp[0], 7U, 100U);
}
#endif
/******************************************************************************
 ;       Function Name			:	uint64_t M_GPIOSense_DisplayFault_Read(void)
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
uint64_t M_GPIOSense_DisplayFault_Read(void)
{
    int8_t i8Counter;
	uint64_t u64ReturnStatus=0UL;
    uint64_t u64Temp=0UL;
	/*Switch Page to CMD2_P2.*/
	uint8_t u8PageSwitch[2]={0x1EU,0x2BU};
	uint8_t u8DataAddress=0x00U;
	uint8_t u8ReadData[8];

	/*Switch Page to CMD2_PB.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8PageSwitch, 2U, 100U);
	/*Write data address.*/
    HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 00h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[0], 1U, 100U);

	/*Write data address.*/
    u8DataAddress=0x01U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 01h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[1], 1U, 100U);

	/*Write data address.*/
	u8DataAddress=0x03U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 03h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[2], 1U, 100U);

	/*Write data address.*/
	u8DataAddress=0x04U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 04h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[3], 1U, 100U);

	/*Write data address.*/
	u8DataAddress=0x07U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 07h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[4], 1U, 100U);

  	/*Write data address.*/
    u8DataAddress=0x0AU;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 0Ah.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[5], 1U, 100U);

	/*Write data address.*/
    u8DataAddress=0x1FU;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 0Ah.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[6], 1U, 100U);

	/* Touch error status */	
	/*Switch Page to CMD4_P1.*/
    u8PageSwitch[1]=0x41U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8PageSwitch, 2U, 100U);
	/*Write data address.*/
	u8DataAddress=0x1CU;
    HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 00h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[7], 1U, 100U);
    
	for(i8Counter=7; i8Counter>=0; i8Counter--)
	{
		u64Temp=u8ReadData[i8Counter];
		u64Temp<<=(i8Counter*8);
		u64ReturnStatus|=u64Temp;
	}
#if (BACKDOOR_DIAGNOSIS_SIMULATE)
	u64ReturnStatus=Memory_Pool_DiagnosisSimulateInfo_Get().u64DispFaultStatus;
#endif
	return u64ReturnStatus;
}


