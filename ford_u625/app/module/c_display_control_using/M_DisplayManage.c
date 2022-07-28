#include "M_DisplayManage.h"

static bool bFlashReloadDisable = false; 
/******************************************************************************
 ;       Function Name			:	void Main_I2cMasterInit(void)
 ;       Function Description	:
 ;       Parameters				:	void
 ;       Return Values			:
 ;       Source ID				:
 ******************************************************************************/
void M_DM_I2cMasterInit(void)
{
    i2c_master_typedef tI2CMaster;

    tI2CMaster.pBase = I2C_MASTER_HW;
    tI2CMaster.pConfig = &I2C_MASTER_config;

    (void) HAL_I2C_Master_Init(tI2CMaster);
}
/******************************************************************************
 ;       Function Name			:	void M_DM_BacklightControl( bool bEnable, bool bLockLoss)
 ;       Function Description	:	Backlight function control
 ;       Parameters				:	bool bEnable, bool bLockLoss
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void M_DM_BacklightControl( bool bEnable, bool bLockLoss)
{
    if (bLockLoss == true)
    {
        HAL_GPIO_Low( U301_LED_EN_PORT, U301_LED_EN_PIN);
    }
    else
    {
        if (bEnable == true)
        {
            HAL_GPIO_High( U301_LED_EN_PORT, U301_LED_EN_PIN);
        }
        else
        {
            HAL_GPIO_Low( U301_LED_EN_PORT, U301_LED_EN_PIN);
        }
    }
}
/******************************************************************************
 ;       Function Name			:	void M_DM_TouchControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss, uint8_t u8LcdCurrentStatus)
 ;       Function Description	:	Touch function control
 ;       Parameters				:	uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss, uint8_t u8LcdCurrentStatus
 ;       Return Values			:	uint8_t u8ReturnStatus
 ;		Source ID				:
 ******************************************************************************/
uint8_t M_DM_TouchControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss, uint8_t u8LcdCurrentStatus)
{
    uint8_t u8ReturnStatus;

    if (bLockLoss == true)
    {
        HAL_GPIO_Low( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
        u8ReturnStatus = TOUCH_OFF;
    }
    else
    {
        switch (u8Command)
        {
            case DISPLAY_OFF_TOUCH_OFF:
				if (u8CurrentStatus != TOUCH_OFF)
				{
	                HAL_GPIO_Low( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
				}
                else
                { /* Nothing */ }
				u8ReturnStatus = TOUCH_OFF;
                break;
            case DISPLAY_ON_TOUCH_OFF:
				if(u8CurrentStatus != TOUCH_OFF)
				{
	                HAL_GPIO_Low( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
				}
                else
                { /* Nothing */ }
				u8ReturnStatus = TOUCH_OFF;
                break;
            case DISPLAY_OFF_TOUCH_ON:
				if (u8CurrentStatus != TOUCH_ON)
				{				
            		HAL_GPIO_High( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
				}
                else
                { /* Nothing */ }
                u8ReturnStatus = TOUCH_ON;
                break;
            case DISPLAY_ON_TOUCH_ON:
				if (u8CurrentStatus != TOUCH_ON)
				{
                	HAL_GPIO_High( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
				}
                else
                { /* Nothing */ }
                u8ReturnStatus = TOUCH_ON;
                break;
             default:
                u8ReturnStatus = u8CurrentStatus;
                break;               
        }
    }
    return u8ReturnStatus;
}
/******************************************************************************
 ;       Function Name			:	uint8_t M_DM_DisplayControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss)
 ;       Function Description	:	Display control function
 ;       Parameters				:	uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss
 ;       Return Values			:	uint8_t u8ReturnStatus
 ;		Source ID				:
 ******************************************************************************/
uint8_t M_DM_DisplayControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss)
{
#if (U625_TDDI_TD7800)
    uint8_t u8SendData;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
    uint8_t u8SendData1[2] = {0x1EU,0x20U};
    uint8_t u8SendData2[2] = {0x01U,0x06U};	
#else
#endif

    uint8_t u8ReturnStatus;

    if (bLockLoss == true)
    {
        if (u8CurrentStatus == DISPLAY_ON)
        {
#if (U625_TDDI_TD7800)        
            u8SendData = 0x28U;
            HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
            u8SendData = 0x10U;
            HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
			u8SendData2[1] = 0x06U;
            /*Switch Page to CMD2_P0.*/
            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData1, sizeof(u8SendData1), 100U);
            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData2, sizeof(u8SendData2), 100U);						
#else
#endif			
        }
        else
        { /* Nothing */ }
        u8ReturnStatus = DISPLAY_OFF;
    }
    else
    {
        switch (u8Command)
        {
            case DISPLAY_OFF_TOUCH_OFF:
                if (u8CurrentStatus != DISPLAY_OFF)
                {
#if (U625_TDDI_TD7800)        
                    u8SendData = 0x28U;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
                    u8SendData = 0x10U;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
					u8SendData2[1] = 0x06U;
                    /*Switch Page to CMD2_P0.*/
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData1, sizeof(u8SendData1), 100U);
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData2, sizeof(u8SendData2), 100U);						
#else
#endif					
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = DISPLAY_OFF;
                break;
            case DISPLAY_ON_TOUCH_OFF:
                if (u8CurrentStatus != DISPLAY_ON)
                {
#if (U625_TDDI_TD7800)               
                    u8SendData = 0x29;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
                    u8SendData = 0x11;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
					u8SendData2[1] = 0x07U;
                    /*Switch Page to CMD2_P0.*/
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData1, sizeof(u8SendData1), 100U);
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData2, sizeof(u8SendData2), 100U);						
#else
#endif					
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = DISPLAY_ON;
                break;
            case DISPLAY_OFF_TOUCH_ON:
                if (u8CurrentStatus != DISPLAY_OFF)
                {                
#if (U625_TDDI_TD7800)            
                    u8SendData = 0x28;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
                    u8SendData = 0x10;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
					u8SendData2[1] = 0x06U;
                    /*Switch Page to CMD2_P0.*/
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData1, sizeof(u8SendData1), 100U);
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData2, sizeof(u8SendData2), 100U);						
#else
#endif					
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = DISPLAY_OFF;
                break;
            case DISPLAY_ON_TOUCH_ON:
                if (u8CurrentStatus != DISPLAY_ON)
                {
#if (U625_TDDI_TD7800)                
                    u8SendData = 0x29;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
                    u8SendData = 0x11;
                    HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, &u8SendData, sizeof(u8SendData), 100U);
                    u8ReturnStatus = DISPLAY_ON;
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
					u8SendData2[1] = 0x07U;
                    /*Switch Page to CMD2_P0.*/
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData1, sizeof(u8SendData1), 100U);
		            HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8SendData2, sizeof(u8SendData2), 100U);						
#else
#endif			
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = DISPLAY_ON;
                break;
                
             default:
                u8ReturnStatus = u8CurrentStatus;
                break;               
        }
    }

    return u8ReturnStatus;
}

/******************************************************************************
 ;       Function Name			:	uint8_t M_DM_LcdControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss)
 ;       Function Description	:	Lcd  control function
 ;       Parameters				:	uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss
 ;       Return Values			:	uint8_t u8ReturnStatus
 ;		Source ID				:
 ******************************************************************************/
uint8_t M_DM_LcdControl(uint8_t u8CurrentStatus, uint8_t u8Command, bool bLockLoss)
{
    uint8_t u8ReturnStatus;

    if (bLockLoss == true)
    {
        if (u8CurrentStatus == LCD_RESET_HIGH)
        {
			HAL_GPIO_Low( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);
			u8ReturnStatus = LCD_RESET_LOW;
        }
        else
        { /* Nothing */ }
        bFlashReloadDisable = false;
        u8ReturnStatus = LCD_RESET_LOW;
    }
    else
    {
        switch (u8Command)
        {
            default:
                u8ReturnStatus = u8CurrentStatus;
                break;
            case DISPLAY_OFF_TOUCH_OFF:
                if (u8CurrentStatus != LCD_RESET_LOW)
                {
					HAL_GPIO_Low( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);
                }
                else
                { /* Nothing */ }
                bFlashReloadDisable = false;
                u8ReturnStatus = LCD_RESET_LOW;
                break;
            case DISPLAY_ON_TOUCH_OFF:
                if (u8CurrentStatus != LCD_RESET_HIGH)
                {
					HAL_GPIO_High( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = LCD_RESET_HIGH;

                break;
            case DISPLAY_OFF_TOUCH_ON:
                if (u8CurrentStatus != LCD_RESET_LOW)
                {
					HAL_GPIO_Low( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);
                }
                else
                { /* Nothing */ }
                bFlashReloadDisable = false;
                u8ReturnStatus = LCD_RESET_LOW;

                break;
            case DISPLAY_ON_TOUCH_ON:
                if (u8CurrentStatus != LCD_RESET_HIGH)
                {
					HAL_GPIO_High( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);
                }
                else
                { /* Nothing */ }
                u8ReturnStatus = LCD_RESET_HIGH;

                break;
        }
    }

    return u8ReturnStatus;
}

/******************************************************************************
 ;       Function Name			:	void M_TC_Control(uint8_t u8Status)
 ;       Function Description	:	This state for error condition
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
uint8_t M_DM_ScanningControl(uint8_t u8CurrentStatus, uint8_t u8Command)
{
    uint8_t mu8SendData[2];	
    uint8_t u8ReturnStatus;
#if (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
	uint8_t u8ScanSel = NUMBER_ZERO;
#endif

    if (u8CurrentStatus != u8Command)
    {
#if (U625_TDDI_TD7800)    
        mu8SendData[0] = 0x36U;
        mu8SendData[1] = (u8Command & (BIT_HSD_MASK | BIT_VSD_MASK)) << 6U;

        HAL_I2C_Master_Write(TD7800_MASTER_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
    if(bFlashReloadDisable == false)
    {
        mu8SendData[0] = 0x1EU;
        mu8SendData[1] = 0x21U;
        /*Switch Page to CMD2_P1.*/
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);

        /*-RELOAD_OFF_KEY .*/
        mu8SendData[0] = 0x0AU;
        mu8SendData[1] = 0xA5U;        
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);
        mu8SendData[0] = 0x09U;
        mu8SendData[1] = 0xA5U;        
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);

        mu8SendData[0] = 0x1EU;
        mu8SendData[1] = 0x10U;
        /*Switch Page to CMD1_P0.*/
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);

        mu8SendData[0] = 0x1DU;
        mu8SendData[1] = 0x03U;
        /* Do not reload.*/
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);
        
        bFlashReloadDisable = true;
    } 
        mu8SendData[0] = 0x1EU;
        mu8SendData[1] = 0x1BU;
        /*Switch Page to CMD1_PB.*/
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);
		
        mu8SendData[0] = 0x05U;
        u8ScanSel = (u8Command & (BIT_HSD_MASK | BIT_VSD_MASK));

		switch (u8ScanSel)
		{
			case SCAN_VT_HL :  
				/* Vertical scan Top to Bottom and Horizontal Left to Right */
				mu8SendData[1] = 0x30U;
				break;
			case SCAN_VT_HR :  
				/* Vertical scan Top to Bottom and Horizontal Right to Left */
				mu8SendData[1] = 0x10U;
				break;
			case SCAN_VB_HL :  
				/* Vertical scan Bottom to Top and Horizontal Left to Right */
				mu8SendData[1] = 0x20U;
				break;
			case SCAN_VB_HR :  
				/* Vertical scan Bottom to Top and Horizontal Right to Left */
				mu8SendData[1] = 0x00U;
				break;					
			default:
				/* Vertical scan Top to Bottom and Horizontal Left to Right */
				mu8SendData[1] = 0x30U; 
				break;
		}			
        HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, sizeof(mu8SendData), 100U);
#else
#endif		
        u8ReturnStatus = u8Command;
    }
    else
    {
        u8ReturnStatus = u8CurrentStatus;
    }
    return u8ReturnStatus;
}
/******************************************************************************
 ;       Function Name			:	void M_TC_Control(uint8_t u8Status)
 ;       Function Description	:	This state for error condition
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
uint32_t M_DM_VCOM_Get(void)
{
	uint8_t u8PageSwitch[2]={0x1EU,0x10U};
	uint8_t u8DataAddress=0x01U;
	uint8_t u8ReadData[3];
	uint32_t u32ReadData;

    /* Switch Page to CMD1_P0.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, u8PageSwitch, sizeof(u8PageSwitch), 100U);
#if 0
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, u8ReadData, 1U, 100U);
#endif
	/*Write data address.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 01h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[0], 1U, 100U);
	/*Write data address.*/
	u8DataAddress=0x02U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 02h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[1], 1U, 100U);
	/*Write data address.*/
	u8DataAddress=0x03U;
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &u8DataAddress, 1U, 100U);
	/*Read data from 03h.*/
	HAL_I2C_Master_Read(NT51926_SLAVE_ADDRESS, &u8ReadData[2], 1U, 100U);

	u32ReadData = ((((uint32_t)u8ReadData[2])<<16U) | (((uint32_t)u8ReadData[1])<<8U) | ((uint32_t)u8ReadData[0]));
	return u32ReadData;
}
/******************************************************************************
 ;       Function Name			:	void M_TC_Control(uint8_t u8Status)
 ;       Function Description	:	This state for error condition
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
void M_DM_VCOM_Set(uint8_t *pSetData)
{
#if (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
	uint8_t mu8SendData[4];
			
	mu8SendData[0] = 0x1EU;
	mu8SendData[1] = 0x10U;
    /*Switch Page to CMD1_P0.*/
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, mu8SendData, (sizeof(mu8SendData)-1U), 100U);
				
	mu8SendData[2] = 0x03U;
	mu8SendData[3] = pSetData[1];	
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &mu8SendData[2], (sizeof(mu8SendData[2])+ 1U), 100U);
			
	mu8SendData[2] = 0x02U;
	mu8SendData[3] = pSetData[0];
	HAL_I2C_Master_Write(NT51926_SLAVE_ADDRESS, &mu8SendData[2], (sizeof(mu8SendData[2]) + 1U), 100U);
#endif


}

/******************************************************************************
;       Function Name			:	void M_DM_INTB_Ctrl(uint8_t u8DigitalLevel)
;       Function Description	:
;       Parameters				:	uint8_t u8DigitalLevel
;       Return Values			:   void
;       Source ID				:
******************************************************************************/
void M_DM_INTB_Ctrl(uint8_t u8DigitalLevel)
{
    if(u8DigitalLevel == 0U)
    {
        HAL_GPIO_Low( U301_INTB_IN_PORT,  U301_INTB_IN_PIN);
    }
    else
    {
        HAL_GPIO_High( U301_INTB_IN_PORT,  U301_INTB_IN_PIN);
    }
}
#if (U625_TDDI_TD7800)
/******************************************************************************
;       Function Name			:	void M_DM_TD7800_ATTN_Read(void)
;       Function Description	:
;       Parameters				:	void
;       Return Values			:
;       Source ID				:
******************************************************************************/
uint8_t M_DM_TD7800_ATTN_Read(void)
{
	uint8_t u8Level;
	HAL_GPIO_Read( U301_TSC_ATTN_PORT,  U301_TSC_ATTN_PIN, &u8Level);

	return u8Level;
}
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
/******************************************************************************
;       Function Name			:	uint8_t M_DM_NT51926_ATTN_Read(void)
;       Function Description	:
;       Parameters				:	void
;       Return Values			:
;       Source ID				:
******************************************************************************/
uint8_t M_DM_NT51926_ATTN_Read(void)
{
	uint8_t u8Level;
	HAL_GPIO_Read( U301_TSC_ATTN_PORT,  U301_TSC_ATTN_PIN, &u8Level);

	return u8Level;
}
#else
#endif

