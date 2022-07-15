#include "M_GPIOSense.h"
#include "M_DisplayManage.h"

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
    return Common_LevelDebounce(u8IOLevel, ptDebounce);
}
/******************************************************************************
 ;       Function Name			:	void Main_I2cSlaveInit(void)
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
void M_GPIOSense_LED_Driver_Diagnosis(uint64_t *pReturnData)
{
    int8_t i8Counter;
    uint8_t u8SendData;
    uint8_t mu8Temp[6U];
    uint64_t u64Temp;
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

    *pReturnData = 0UL;

    for ( i8Counter = 5; i8Counter >= 0; i8Counter-- )
    {
        u64Temp = mu8Temp[i8Counter];
        u64Temp <<= (i8Counter * 8);
        *pReturnData |= u64Temp;
    }
}
/******************************************************************************
 ;       Function Name			:	
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
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
/******************************************************************************
 ;       Function Name			:	void Main_I2cSlaveInit(void)
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
	uint8_t u8PageSwitch[2]={0x1EU,0x22U};
	uint8_t u8DDataAddress=0x01U;
	uint8_t u8ReadData[5];

	/*Switch Page to CMD2_P2.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8PageSwitch, 2U, 100U);
	/*Write data address.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DDataAddress, 1U, 100U);
	/*Read data from 01h-05h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, u8ReadData, 5U, 100U);

	uint8_t u8ReadData2[9];
	/*Switch Page to CMD2_PB.*/
    u8PageSwitch[1]=0x2BU;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8PageSwitch, 2U, 100U);
	/*Write data address.*/
    u8DDataAddress=0x00U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DDataAddress, 1U, 100U);
	/*Read data from 00h-07h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData2[0], 8U, 100U);

  	/*Write data address.*/
    u8DDataAddress=0x0AU;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DDataAddress, 1U, 100U);
	/*Read data from 0Ah.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData2[8], 1U, 100U);

    
	for(i8Counter=4; i8Counter>=0; i8Counter--)
	{
		u64Temp=u8ReadData[i8Counter];
		u64Temp<<=(i8Counter*8);
		u64ReturnStatus|=u64Temp;
	}

	return u64ReturnStatus;
}

