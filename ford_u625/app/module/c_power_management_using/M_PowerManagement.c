#include "M_PowerManagement.h"
#include "main.h"
#include "M_INTB.h"


static uint8_t M_PM_CheckPowerPG(uint8_t u8Status, uint8_t u8Times)
{
    uint8_t u8PGLevel;
	uint8_t u8CheckTime;
	uint8_t u8Return = LOW_LEVEL;
	
    
   	if (u8Times == NUMBER_ZERO)
   	{   /* times can not be zero */
       u8Return = LOW_LEVEL;
   	}
	else
   	{
		for(u8CheckTime = u8Times; u8CheckTime > NUMBER_ZERO; u8CheckTime--)
	    {
	        switch(u8Status)
	        {
		        case P3V3_PGOOD:
		            HAL_GPIO_Read(U301_P3V3_PGOOD_PORT, U301_P3V3_PGOOD_PIN, &u8PGLevel);
		            break;
		        case P1V2_PGOOD:
		            HAL_GPIO_Read(U301_P1V2_PGOOD_PORT, U301_P1V2_PGOOD_PIN, &u8PGLevel);
		            break;
		        default:
		            break;
	        }
	        
	        if (u8PGLevel == HIGH_LEVEL)
	        {   /* pin good! */
	        	u8Return = HIGH_LEVEL;
	            return u8Return;
	        }
	        else
	        {
	            Cy_SysLib_DelayUs(500U); /* 500us */
	        }
	        
	    }
	}
    
    /* error flag */
    return u8Return;
}

/******************************************************************************
 ;       Function Name			:	void M_PM_Sequnce_Execute(void)
 ;       Function Description	:	This state for error condition
 ;       Parameters				:	void
 ;       Return Values			:	void
 ;		Source ID				:
 ******************************************************************************/
uint8_t M_PM_Sequnce_Execute(uint8_t u8Action)
{
    uint8_t u8Return;

    switch (u8Action)
    {
        default:
            u8Return = POWER_CMD_FAIL;
            break;
        case POWER_OFF_READY:
        case POWER_ON_READY:
            u8Return = POWER_PASS;
            break;
        case POWER_ON:
#if (U625_TDDI_TD7800)
            HAL_GPIO_Low( U301_FCS_PORT, U301_FCS_PIN);
            HAL_GPIO_Low( U301_DBIST_PORT, U301_DBIST_PIN);
            HAL_GPIO_Low( U301_DISP_SLEEPIN_PORT, U301_DISP_SLEEPIN_PIN);
            HAL_GPIO_Low( U301_DISP_MUTE_PORT, U301_DISP_MUTE_PIN);
            HAL_GPIO_Low( U301_UD_PORT, U301_UD_PIN);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
#else
#endif
            HAL_GPIO_High( U301_HV_LDO_EN_PORT, U301_HV_LDO_EN_PIN);
            HAL_GPIO_High( U301_P3V3_EN_PORT, U301_P3V3_EN_PIN);
            Cy_SysLib_Delay(10U);  /* 10ms */

            if (HIGH_LEVEL == M_PM_CheckPowerPG(P3V3_PGOOD, 12U)) /* about 1ms */
            {
				Cy_SysLib_Delay(2U);  /* 2ms */
				Cy_SysLib_DelayUs(200U); /* 200us */
                HAL_GPIO_High( U301_P1V2_EN_PORT, U301_P1V2_EN_PIN);
				
                Cy_SysLib_Delay(10U);	/* 10ms */
                if (HIGH_LEVEL == M_PM_CheckPowerPG(P1V2_PGOOD, 10U))
                {
                 	/* HAL_UART_Printf("=> P1V2 successful ...\n"); */

					Cy_SysLib_Delay(1U);	/* 1ms */
                	HAL_GPIO_High( U301_TSC_RESET_PORT, U301_TSC_RESET_PIN);
#if (U625_TDDI_TD7800)          
                    HAL_GPIO_High( U301_SS_PORT, U301_SS_PIN);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
#else
#endif
					Cy_SysLib_Delay(10U); /* 10ms */
					HAL_GPIO_High( U301_DISP_GLOBAL_RESET_PORT, U301_DISP_GLOBAL_RESET_PIN);

					Cy_SysLib_Delay(100U); /* 100ms */
                    HAL_GPIO_High( U301_PDB_PORT, U301_PDB_PIN);
                    HAL_GPIO_High( U301_INTB_IN_PORT,  U301_INTB_IN_PIN);
                    /* INTB Init. */
                    MINTB_StrategyControl(STRATEGY_CTRL_INIT);
			
                    u8Return = POWER_PASS;
                }
                else
                {
                    /* HAL_UART_Printf("=> P1V2 fail ...\n"); */
                    u8Return = P1V2_FAIL;
                }
            }
            else
            {
                /* HAL_UART_Printf("=> P3V3 fail ...\n"); */
                u8Return = P3V3_FAIL;
            }
            break;
        case POWER_OFF:

			Cy_SysLib_Delay(5U); /* 5ms */
            /* INTB Deinit */
			MINTB_StrategyControl(STRATEGY_CTRL_DEINIT);
            HAL_GPIO_Low( U301_INTB_IN_PORT,  U301_INTB_IN_PIN);
            HAL_GPIO_Low( U301_PDB_PORT, U301_PDB_PIN);
			
			Cy_SysLib_Delay(5U); /* 5ms */
            HAL_GPIO_Low( U301_P1V2_EN_PORT, U301_P1V2_EN_PIN);
			
			Cy_SysLib_Delay(5U); /* 5ms */
#if (U625_TDDI_TD7800)
            HAL_GPIO_Low( U301_FCS_PORT, U301_FCS_PIN);
#elif (CX430_TDDI_NT51926 || U717_TDDI_NT51926)
#endif
            HAL_GPIO_Low( U301_P3V3_EN_PORT, U301_P3V3_EN_PIN);

			Cy_SysLib_Delay(5U); /* 5ms */
            HAL_GPIO_Low( U301_HV_LDO_EN_PORT, U301_HV_LDO_EN_PIN);

            u8Return = POWER_PASS;
            break;
    }

    return u8Return;
}

