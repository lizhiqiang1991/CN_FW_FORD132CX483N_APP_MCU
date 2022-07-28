/**
 * @file M_TemperatureDerating.c
 * 
 * @brief The program could execute a specific derating method according to backlight temperature.
 * 
 * @details 1.To build a derating table which saves each backlight percentage according to backlight temperature.\n
 * 2.To build a smooth dimming control table which changes 10% per 200 milli second.\n
 * 3.To build a state machine mechanism according to backlight temperature.
 * 
 * @note After building a derating method and dimming control,
 * you must register these by DeratingApp_RegisterElement() and DeratingApp_RegisterDimmingControl(). 
 * 
 * @author Orlando Huang (Orlando.Huang@auo.com)
 * 
 * @version 0.1
 * 
 * @date 2022-05-04
 * 
 */

/* -- Includes -- */
#include "M_TemperatureDerating.h"
#include "Memory_Pool.h"

/* -- Type Define -- */
/**
 * @brief The data type is used for calculating a PWM duty between two elements in a derating table.
 * 
 */
typedef struct
{
#if(DERATINGAPP_FLOAT_OPERATION)
    float DeratingTemperature; /**< To save near temperature according to system's backlight temperature. */
    float DeratingPWM; /**< To save the PWM value according to DeratingTemperature value. */
#else
    int16_t DeratingTemperature; /**< To save near temperature according to system's backlight temperature. */
    int16_t DeratingPWM; /**< To save the PWM value according to DeratingTemperature value. */
#endif
}BacklightDeratingApp_DeratingTempToPWM_Typedef;

/* -- Global Variables -- */

#if(TEMP_DERATINGAPP_TEST)
/**
 * @brief A simulate backlight temperature is used for testing function.
 * 
 * @details The simulate backlight temperature could affect state machine mechanism's result.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
float gSimuBLTemperature = (TEMP_DERATING_DEFAULT_DERA_TEMP - 1.0);
#else
int16_t gSimuBLTemperature = (TEMP_DERATING_DEFAULT_DERA_TEMP - (1*TEMP_DERATING_TEMP_RESOLUTION));
#endif
#endif
/**
 * @brief To Save previous state.
 * 
 * @details To reserve for extended function in the future.  
 * 
 */
DeratingApp_Status PreDeratingStatus = INIT_MODE;
/**
 * @brief The state is from normal to derating if the backlight temperature is over it.
 * 
 * @details The value is equal to 70 celisius,it could start getting the PWM by looking for derating table.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
static float gBLDeratingLimitTemp = TEMP_DERATING_DEFAULT_DERA_TEMP;
#else
static int16_t gBLDeratingLimitTemp = TEMP_DERATING_DEFAULT_DERA_TEMP;
#endif
/**
 * @brief Avoiding that the system does not catch backlight temperature.
 * 
 * @details 1.When the function start running,the time will count down.\n
 * 2.If the time is equal to zero,The state is changed to derating.
 * 
 */

/**
 * @brief A look up table for catching the PWM duty according to backlight temperature. 
 * 
 * @details When the backlight temperature is over gBLDeratingLimitTemp,the PWM duty is changed by the look up table.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
static float gDeratingTable[TEMP_DERATING_TABLE_SIZE] = 
{
    100.0f, 
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    100.0f,
    85.0f,
    71.0f,
    59.0f,
    48.0f,
    38.0f,
    29.0f,
    22.0f,
    16.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
    10.0f,
};
#else
static int16_t gDeratingTable[TEMP_DERATING_TABLE_SIZE] = 
{
    100, 
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    85,
    71,
    59,
    48,
    38,
    29,
    22,
    16,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
};
#endif

/* -- Local Functions -- */

/* -- Global Functions -- */
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Init(void);
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Boot(void);
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Derating(void);
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Normal(void);
/**
 * @brief When the system start,the function will be called.
 * 
 * @details If system does not get backlight temperature,the state machine is from init to normal avoid affecting backlingt control.
 * 
 * @note It is global function and be called by reference.
 * 
 * @return DeratingApp_ReturnMethod_Typedef To return the state function of derating method and the PWM dyuty.
 * 
 * @dot
 * digraph BLDerating_State_Machine_Mechanism {
 *  "Init" -> "Boot" [label = "BLTemperature >= 70 celies."];
 *  "Init" -> "Normal" [label = "BLTemperature is not vaild \n until gu16InitWaitTimer counts down to zero."];
 *  "Init" -> "Normal" [label = "BLTemperature is vaild \n and BLTemperature <70 celies."];
 * }
 * @enddot 
 * 
 */
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Init(void)
{
    DeratingApp_Status DeratingStatus = PreDeratingStatus;
    DeratingApp_ReturnMethod_Typedef ReturnMethod;
#if(DERATINGAPP_FLOAT_OPERATION)
    float BLTemperature = (float)(Memory_Pool_PCBATemp_Get());
#else

#endif 
    
#if(TEMP_DERATINGAPP_TEST)
    BLTemperature = gSimuBLTemperature;
#endif

    DeratingStatus = NORMAL;

    PreDeratingStatus = DeratingStatus; 
    ReturnMethod.MethodStatus = DeratingStatus;
    ReturnMethod.DeratingOutData = (int16_t)TEMP_DERATING_DEFAULT_OUTDATA;

    return ReturnMethod;
}
/**
 * @brief Ready to execute derating function.
 * 
 * @details When system catchs backlight temperatrue which is over derating threshold.
 * 
 * @note It is global function and be called by reference.
 * 
 * @return DeratingApp_ReturnMethod_Typedef To return the state function of derating method and the PWM dyuty.
 * 
 * @dot
 * digraph BLDerating_State_Machine_Mechanism {
 *  "Boot" -> "Derating" [label = "No condition."];
 * }
 * @enddot 
 * 
 */
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Boot(void)
{
    DeratingApp_ReturnMethod_Typedef ReturnMethod;
    DeratingApp_Status DeratingStatus = DERATING;

    PreDeratingStatus = DeratingStatus;
    ReturnMethod.MethodStatus = DeratingStatus;
    ReturnMethod.DeratingOutData = (int16_t)TEMP_DERATING_DEFAULT_OUTDATA;

    return ReturnMethod;
}
/**
 * @brief To execute derating function and decide whether the next state is called.
 * 
 * @details 1.When backlight temperatrue is less derating threshold,the next state is normal.\n
 * 2.Because the state is derating,the PWM duty should be calculated according to gDeratingTable.
 * 
 * @note It is global function and be called by reference.
 * 
 * @return DeratingApp_ReturnMethod_Typedef To return the state function of derating method and the PWM dyuty.
 * 
 * @dot
 * digraph BLDerating_State_Machine_Mechanism {
 *  "Derating" -> "Derating" [label = "BLTemperature >= 70 celies."];
 *  "Derating" -> "Normal" [label = "BLTemperature < 70 celies."];
 * }
 * @enddot 
 * 
 */
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Derating(void)
{
    DeratingApp_ReturnMethod_Typedef ReturnMethod;
    DeratingApp_Status DeratingStatus = PreDeratingStatus;
    BacklightDeratingApp_DeratingTempToPWM_Typedef DeratingTempToPWMUp;
    BacklightDeratingApp_DeratingTempToPWM_Typedef DeratingTempToPWMDown;
    int16_t i16DeratingTableIndex = 0U;

#if(DERATINGAPP_FLOAT_OPERATION)
    float BLTemperature = (float)(Memory_Pool_PCBATemp_Get());
#else
    int16_t BLTemperature = Memory_Pool_PCBATemp_Get()*TEMP_DERATING_TEMP_RESOLUTION;
#endif

#if(TEMP_DERATINGAPP_TEST)
    BLTemperature = gSimuBLTemperature;
#endif

    if(BLTemperature >= gBLDeratingLimitTemp)
    {
        DeratingStatus = DERATING;
    }
    else
    {
        DeratingStatus = NORMAL;
    }

    PreDeratingStatus = DeratingStatus;
    ReturnMethod.MethodStatus = DeratingStatus;

    /* PWM is calculate from derating table. 
    ; The relationship of two items is linear.
    */
#if(DERATINGAPP_FLOAT_OPERATION)
    i16DeratingTableIndex = (BLTemperature - gBLDeratingLimitTemp);
#else
    i16DeratingTableIndex = (BLTemperature - gBLDeratingLimitTemp) / TEMP_DERATING_TEMP_RESOLUTION;
#endif
    /* Avoid to go out of array range. */
    i16DeratingTableIndex = (i16DeratingTableIndex > (TEMP_DERATING_TABLE_SIZE - 1))?(TEMP_DERATING_TABLE_SIZE - 1):i16DeratingTableIndex;

    DeratingTempToPWMDown.DeratingPWM = gDeratingTable[i16DeratingTableIndex];
#if(DERATINGAPP_FLOAT_OPERATION)
    DeratingTempToPWMDown.DeratingTemperature = gBLDeratingLimitTemp + (float)(i16DeratingTableIndex);
#else
    DeratingTempToPWMDown.DeratingTemperature = gBLDeratingLimitTemp + (i16DeratingTableIndex * TEMP_DERATING_TEMP_RESOLUTION);
#endif
    DeratingTempToPWMUp.DeratingPWM = gDeratingTable[i16DeratingTableIndex + 1U];
#if(DERATINGAPP_FLOAT_OPERATION)
    DeratingTempToPWMUp.DeratingTemperature = gBLDeratingLimitTemp + (float)(i16DeratingTableIndex + 1U);
#else
    DeratingTempToPWMUp.DeratingTemperature = gBLDeratingLimitTemp + ((i16DeratingTableIndex + 1) * TEMP_DERATING_TEMP_RESOLUTION);
#endif
    ReturnMethod.DeratingOutData = DeratingTempToPWMDown.DeratingPWM + \
        (((BLTemperature - DeratingTempToPWMDown.DeratingTemperature)\
        *(DeratingTempToPWMUp.DeratingPWM - DeratingTempToPWMDown.DeratingPWM))\
        /(DeratingTempToPWMUp.DeratingTemperature - DeratingTempToPWMDown.DeratingTemperature));

    return ReturnMethod;
}
/**
 * @brief To execute normal function and decide whether the next state is called.
 * 
 * @details 1.When backlight temperatrue is over derating threshold,the next state is derating.\n
 * 2.Because the state is normal,the PWM duty should be controled according to host's command.
 * 
 * @note It is global function and be called by reference.
 * 
 * @return DeratingApp_ReturnMethod_Typedef To return the state function of derating method and the PWM dyuty.
 * 
 * @dot
 * digraph BLDerating_State_Machine_Mechanism {
 *  "Normal" -> "Derating" [label = "BLTemperature >= 70 celies."];
 *  "Normal" -> "Normal" [label = "BLTemperature < 70 celies."];
 * }
 * @enddot 
 * 
 */
DeratingApp_ReturnMethod_Typedef TemperatureDerating_State_Normal(void)
{
    DeratingApp_ReturnMethod_Typedef ReturnMethod;
    DeratingApp_Status DeratingStatus = PreDeratingStatus;

#if(DERATINGAPP_FLOAT_OPERATION)
    float BLTemperature = (float)(Memory_Pool_PCBATemp_Get());
#else
    int16_t BLTemperature = Memory_Pool_PCBATemp_Get()*TEMP_DERATING_TEMP_RESOLUTION;
#endif

#if(TEMP_DERATINGAPP_TEST)
    BLTemperature = gSimuBLTemperature;
#endif

    if(BLTemperature >= gBLDeratingLimitTemp)
    {
        DeratingStatus = DERATING;
    }
    else
    {
        DeratingStatus = NORMAL;
    }

    PreDeratingStatus = DeratingStatus;
    ReturnMethod.MethodStatus = DeratingStatus;
    ReturnMethod.DeratingOutData = (int16_t)TEMP_DERATING_DEFAULT_OUTDATA;

    return ReturnMethod;
}
/**
 * @brief To apply to DeratingApp.c for registering the backlight derating method.
 * 
 * @details To register dimming control settings in order to control backlight smoothly.
 * 
 * @note It is a global function.
 * 
 * @param DeratingSettingTemp To set up the derating temperature threshold.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
void TemperatureDerating_Init(float DeratingSettingTemp)
#else
void TemperatureDerating_Init(int16_t DeratingSettingTemp)
#endif
{
    TemperatureDerating_DeratingTemp_Set(DeratingSettingTemp);
    /* Register BL Derating Dimming */
    DeratingApp_RegisterElement(AMBIENT_TEMPERATURE,\
        TemperatureDerating_State_Init,\
        TemperatureDerating_State_Boot,\
        TemperatureDerating_State_Derating,\
        TemperatureDerating_State_Normal,\
        TEMP_DERATING_METHOD_PERIOD_TIME,\
        TEMP_DERATING_METHOD_CHECK_TIME
    ); 
}
#if(BACKDOOR_WRITE_DERATINGTABLE)
/**
 * @brief To change all contents of gDeratingTable.
 * 
 * @details The setting data over 100 is not allowed.
 * 
 * @note It is a global function.
 * 
 * @param uint8_t The input data is used to save into gDeratingTable.
 * 
 * @return bool Is the setting data vaild?
 * 
 */
bool TemperatureDerating_DeratingTable_Set(volatile uint8_t *AllTableElement)
{
    uint8_t u8DeratingTableIndex = 0U;
#if(DERATINGAPP_FLOAT_OPERATION)
    float MiniTableElementData = TEMP_DERATING_DEFAULT_OUTDATA;
#else
    int16_t MiniTableElementData = TEMP_DERATING_DEFAULT_OUTDATA;
#endif
    /* Checking data is sorted from largest to smallest */
    for(u8DeratingTableIndex = 0U ; u8DeratingTableIndex < TEMP_DERATING_TABLE_SIZE ; u8DeratingTableIndex++)
    {
#if(DERATINGAPP_FLOAT_OPERATION)
        if((float)(*(AllTableElement + u8DeratingTableIndex)) > MiniTableElementData)
#else
        if((int16_t)(*(AllTableElement + u8DeratingTableIndex)) > MiniTableElementData)
#endif
        {
            return FALSE;
        }
        else
        {
#if(DERATINGAPP_FLOAT_OPERATION)
            MiniTableElementData = (float)(*(AllTableElement + u8DeratingTableIndex));
#else
            MiniTableElementData = (int16_t)(*(AllTableElement + u8DeratingTableIndex));
#endif
        }
    }
    /* Updating derating table elements */
    for(u8DeratingTableIndex = 0U ; u8DeratingTableIndex < TEMP_DERATING_TABLE_SIZE ; u8DeratingTableIndex++)
    {
#if(DERATINGAPP_FLOAT_OPERATION)
       *(gDeratingTable + u8DeratingTableIndex) = (float)(*(AllTableElement + u8DeratingTableIndex));
#else
       *(gDeratingTable + u8DeratingTableIndex) = (int16_t)(*(AllTableElement + u8DeratingTableIndex)) * (TEMP_DERATING_TEMP_RESOLUTION);
#endif
    }

    return TRUE;
}
#endif
/**
 * @brief To set up the derating temperature threshold.
 * 
 * @details None.
 * 
 * @note It is a global function.
 * 
 * @param DeratingSettingTemp The input data is used to change the derating temperature threshold.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
void TemperatureDerating_DeratingTemp_Set(float DeratingSettingTemp)
#else
void TemperatureDerating_DeratingTemp_Set(int16_t DeratingSettingTemp)
#endif
{
    gBLDeratingLimitTemp = DeratingSettingTemp;
}
/**
 * @brief To catch the derating temperature threshold.
 * 
 * @details None.
 * 
 * @note It is a global function.
 * 
 * @return float The output data is used to catch the derating temperature threshold.
 * 
 */
#if(BACKDOOR_WRITE_DERATINGTABLE)
#if(DERATINGAPP_FLOAT_OPERATION)
float* TemperatureDerating_DeratingTable_Get(void)
#else
int16_t* TemperatureDerating_DeratingTable_Get(void)
#endif
{
    return gDeratingTable;
}
#endif
/* -- END -- */
