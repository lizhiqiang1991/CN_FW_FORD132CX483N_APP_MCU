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
static float gBLDeratingLimitedTemp = TEMP_DERATING_DEFAULT_DERA_TEMP;
static float gBLShutDownLimitedTemp = TEMP_DERATING_DEFAULT_SHUTDOWN_TEMP;
static float gBLShutDownRelLimitedTemp = TEMP_DERATING_DEFAULT_SHUTDOWN_REL_TEMP;
static float gBLReduceLimitedTemp = TEMP_DERATING_DEFAULT_REDUCEBL_TEMP;
#else
static int16_t gBLDeratingLimitedTemp = TEMP_DERATING_DEFAULT_DERA_TEMP;
static int16_t gBLShutDownLimitedTemp = TEMP_DERATING_DEFAULT_SHUTDOWN_TEMP;
static int16_t gBLShutDownRelLimitedTemp = TEMP_DERATING_DEFAULT_SHUTDOWN_REL_TEMP;
static int16_t gBLReduceLimitedTemp = TEMP_DERATING_DEFAULT_REDUCEBL_TEMP;
#endif
/**
 * @brief A look up table for catching the PWM duty according to backlight temperature. 
 * 
 * @details When the backlight temperature is over gBLDeratingLimitedTemp,the PWM duty is changed by the look up table.
 * 
 */
#if(DERATINGAPP_FLOAT_OPERATION)
static float gDeratingTable[TEMP_DERATING_TABLE_SIZE] = 
{
    100.0f, /**< 70 Degree. */
    100.0f, /**< 71 Degree. */
    100.0f, /**< 72 Degree. */
    100.0f, /**< 73 Degree. */
    100.0f, /**< 74 Degree. */
    100.0f, /**< 75 Degree. */
    100.0f, /**< 76 Degree. */
    100.0f, /**< 77 Degree. */
    100.0f, /**< 78 Degree. */
    100.0f, /**< 79 Degree. */
    100.0f, /**< 80 Degree. */
    100.0f, /**< 81 Degree. */
    100.0f, /**< 82 Degree. */
    100.0f, /**< 83 Degree. */
    100.0f, /**< 84 Degree. */
    100.0f, /**< 85 Degree. */
    85.0f, /**< 86 Degree. */
    71.0f, /**< 87 Degree. */
    59.0f, /**< 88 Degree. */
    48.0f, /**< 89 Degree. */
    38.0f, /**< 90 Degree. */
    29.0f, /**< 91 Degree. */
    22.0f, /**< 92 Degree. */
    16.0f, /**< 93 Degree. */
    10.0f, /**< 94 Degree. */
    10.0f, /**< 95 Degree. */
    10.0f, /**< 96 Degree. */
    10.0f, /**< 97 Degree. */
    10.0f, /**< 98 Degree. */
    10.0f, /**< 99 Degree. */
    10.0f, /**< 100 Degree. */
};
#else
static int16_t gDeratingTable[TEMP_DERATING_TABLE_SIZE] = 
{
    100, /**< 70 Degree. */
    100, /**< 71 Degree. */
    100, /**< 72 Degree. */
    100, /**< 73 Degree. */
    100, /**< 74 Degree. */
    100, /**< 75 Degree. */
    100, /**< 76 Degree. */
    100, /**< 77 Degree. */
    100, /**< 78 Degree. */
    100, /**< 79 Degree. */
    100, /**< 80 Degree. */
    100, /**< 81 Degree. */
    100, /**< 82 Degree. */
    100, /**< 83 Degree. */
    100, /**< 84 Degree. */
    100, /**< 85 Degree. */
    85, /**< 86 Degree. */
    71, /**< 87 Degree. */
    59, /**< 88 Degree. */
    48, /**< 89 Degree. */
    38, /**< 90 Degree. */
    29, /**< 91 Degree. */
    22, /**< 92 Degree. */
    16, /**< 93 Degree. */
    10, /**< 94 Degree. */
    10, /**< 95 Degree. */
    10, /**< 96 Degree. */
    10, /**< 97 Degree. */
    10, /**< 98 Degree. */
    10, /**< 99 Degree. */
    10, /**< 100 Degree. */
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

    if(BLTemperature >= gBLDeratingLimitedTemp)
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
    i16DeratingTableIndex = (BLTemperature - gBLDeratingLimitedTemp);
#else
    i16DeratingTableIndex = (BLTemperature - gBLDeratingLimitedTemp) / TEMP_DERATING_TEMP_RESOLUTION;
#endif
    /* Avoid to go out of array range. */
    i16DeratingTableIndex = (i16DeratingTableIndex > (TEMP_DERATING_TABLE_SIZE - 1))?(TEMP_DERATING_TABLE_SIZE - 1):i16DeratingTableIndex;

    DeratingTempToPWMDown.DeratingPWM = gDeratingTable[i16DeratingTableIndex];
#if(DERATINGAPP_FLOAT_OPERATION)
    DeratingTempToPWMDown.DeratingTemperature = gBLDeratingLimitedTemp + (float)(i16DeratingTableIndex);
#else
    DeratingTempToPWMDown.DeratingTemperature = gBLDeratingLimitedTemp + (i16DeratingTableIndex * TEMP_DERATING_TEMP_RESOLUTION);
#endif
    DeratingTempToPWMUp.DeratingPWM = gDeratingTable[i16DeratingTableIndex + 1U];
#if(DERATINGAPP_FLOAT_OPERATION)
    DeratingTempToPWMUp.DeratingTemperature = gBLDeratingLimitedTemp + (float)(i16DeratingTableIndex + 1U);
#else
    DeratingTempToPWMUp.DeratingTemperature = gBLDeratingLimitedTemp + ((i16DeratingTableIndex + 1) * TEMP_DERATING_TEMP_RESOLUTION);
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

    if(BLTemperature >= gBLDeratingLimitedTemp)
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
 */
void TemperatureDerating_Init(void)
{
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

#if(DERATINGAPP_FLOAT_OPERATION)
float TemperatureDerating_GetLimitedTemperature(TEMP_DERATING_LIMITED_SELECTION TempDeratingLimitedSelection)
#else
int16_t TemperatureDerating_GetLimitedTemperature(TEMP_DERATING_LIMITED_SELECTION TempDeratingLimitedSelection)
#endif
{
    switch (TempDeratingLimitedSelection)
    {    
        default:
            return 0;
            break;

        case LIMITED_DERATING_TEMP:
            return gBLDeratingLimitedTemp;
            break;

        case LIMITED_SHUTDOWN_TEMP:
            return gBLShutDownLimitedTemp;
            break;

        case LIMITED_SHUTDOWN_REL_TEMP:
            return gBLShutDownRelLimitedTemp;

        case LIMITED_REDUCE_BL_TEMP:
            return gBLReduceLimitedTemp;
            break;
    }
}

#if(BACKDOOR_WRITE_DERATINGDATA)
/**
 * @brief Setting DeratingLimitedTemp,ShutDownLimitedTemp and ShutDownRelLimitedTemp.
 * 
 * @param CalibrationElement 
 * 
 * @return true Success.
 *  
 * @return false Unsuccess.
 *  
 */
bool TemperatureDerating_LimitedTemperature_Get(volatile uint8_t *CalibrationElement, uint8_t CalibrationElementLength)
{
    float fConvertedTemp = 0.0f;
    uint16_t u16ReturnedTemp = 0U;

    if(CalibrationElementLength != TEMP_DERATING_LIMITED_TEMPERATURE_LENGTH)
    {
        return false;
    }
    else{/* NA */}

    /* Sets Derating Setting Temperature */
    fConvertedTemp = (((float)(gBLDeratingLimitedTemp) - (float)TEMP_DERATING_OFFSET) / (float)TEMP_DERATING_RESOLUTION);
    u16ReturnedTemp = (uint16_t)(fConvertedTemp);
    /* Fills Data */
    *CalibrationElement = (uint8_t)(u16ReturnedTemp >> 8);
    *(CalibrationElement + 1U) = (uint8_t)(u16ReturnedTemp);

    /* Sets Shutdown Setting Temperature */
    fConvertedTemp = (((float)(gBLShutDownLimitedTemp) - (float)TEMP_DERATING_OFFSET) / (float)TEMP_DERATING_RESOLUTION);
    u16ReturnedTemp = (uint16_t)(fConvertedTemp);
    /* Fills Data */
    *(CalibrationElement + 2U) = (uint8_t)(u16ReturnedTemp >> 8);
    *(CalibrationElement + 3U) = (uint8_t)(u16ReturnedTemp);

    /* Sets Shutdown Rel Setting Temperature */
    fConvertedTemp = (((float)(gBLShutDownRelLimitedTemp) - (float)TEMP_DERATING_OFFSET) / (float)TEMP_DERATING_RESOLUTION);
    u16ReturnedTemp = (uint16_t)(fConvertedTemp);
    /* Fills Data */
    *(CalibrationElement + 4U) = (uint8_t)(u16ReturnedTemp >> 8);
    *(CalibrationElement + 5U) = (uint8_t)(u16ReturnedTemp);

    /* Sets Shown TERR Temperature */
    fConvertedTemp = (((float)(gBLReduceLimitedTemp) - (float)TEMP_DERATING_OFFSET) / (float)TEMP_DERATING_RESOLUTION);
    u16ReturnedTemp = (uint16_t)(fConvertedTemp);
    /* Fills Data */
    *(CalibrationElement + 6U) = (uint8_t)(u16ReturnedTemp >> 8);
    *(CalibrationElement + 7U) = (uint8_t)(u16ReturnedTemp);

    return true;
}
/**
 * @brief 
 * 
 * @return volatile* 
 */
bool TemperatureDerating_DeratingTable_Get(volatile uint8_t *CalibrationElement, uint8_t CalibrationElementLength)
{
    uint8_t u8DeratingTableIndex = 0;

    if(CalibrationElementLength != TEMP_DERATING_TABLE_TEMPERATURE_LENGTH)
    {
        return false;
    }
    else{/* NA */}

    for(u8DeratingTableIndex = 0 ; u8DeratingTableIndex < CalibrationElementLength; u8DeratingTableIndex++)
    {
        *(CalibrationElement + u8DeratingTableIndex) = (uint8_t)(*(gDeratingTable + u8DeratingTableIndex));
    }

    return true;
}
/**
 * @brief 
 * 
 * @param DeratingCalibrationDataIndex 
 * 
 * @param SettingData 
 * 
 * @return true 
 * 
 * @return false
 *  
 */
bool TemperatureDerating_DeratingCalibrationData_Set(volatile uint8_t *CalibrationElement)
{
    float fConvertedTemp = 0.0f;

    switch((TEMP_DERATING_CALIBRATION_DATAINDEX)(CalibrationElement[0]))
    {
        case DERATING_CALIBRATION_DATAINDEX_DERATING_TEMP:
            /* Gets Derating Setting Temperature */
            fConvertedTemp = ((float)(CalibrationElement[1]) * 256) + (float)(CalibrationElement[2]);
            fConvertedTemp /= 10.0f;
            fConvertedTemp -= 40.0f;
#if(DERATINGAPP_FLOAT_OPERATION)
            gBLDeratingLimitedTemp = fConvertedTemp;
#else
            gBLDeratingLimitedTemp = (int16_t)(fConvertedTemp);
#endif
            break;
        case DERATING_CALIBRATION_DATAINDEX_SHUTDOWN_TEMP:
            /* Gets Shut Down Setting Temperature */
            fConvertedTemp = ((float)(CalibrationElement[1]) * 256) + (float)(CalibrationElement[2]);
            fConvertedTemp /= 10.0f;
            fConvertedTemp -= 40.0f;
#if(DERATINGAPP_FLOAT_OPERATION)
            gBLShutDownLimitedTemp = fConvertedTemp;
#else
            gBLShutDownLimitedTemp = (int16_t)(fConvertedTemp);
#endif
            break;
        case DERATING_CALIBRATION_DATAINDEX_SHUTDOWN_REL_TEMP:
            /* Gets Shut Down Rel Setting Temperature */
            fConvertedTemp = ((float)(CalibrationElement[1]) * 256) + (float)(CalibrationElement[2]);
            fConvertedTemp /= 10.0f;
            fConvertedTemp -= 40.0f;;
#if(DERATINGAPP_FLOAT_OPERATION)
            gBLShutDownRelLimitedTemp = fConvertedTemp;
#else
            gBLShutDownRelLimitedTemp = (int16_t)(fConvertedTemp);
#endif
            break;

        case DERATING_CALIBRATION_DATAINDEX_SHOWN_TERR_TEMP:
            /* Gets Shown TERR Setting Temperature */
            fConvertedTemp = ((float)(CalibrationElement[1]) * 256) + (float)(CalibrationElement[2]);
            fConvertedTemp /= 10.0f;
            fConvertedTemp -= 40.0f;;
#if(DERATINGAPP_FLOAT_OPERATION)
            gBLReduceLimitedTemp = fConvertedTemp;
#else
            gBLReduceLimitedTemp = (int16_t)(fConvertedTemp);
#endif
            break;

        default:
            if(CalibrationElement[0] >= (DERATING_CALIBRATION_DATAINDEX_TABLE_START + TEMP_DERATING_TABLE_TEMPERATURE_LENGTH))
            {
                return false;
            }
            else
            {
                fConvertedTemp = ((float)(CalibrationElement[1]) * 256) + (float)(CalibrationElement[2]);
#if(DERATINGAPP_FLOAT_OPERATION)
                gDeratingTable[(DeratingCalibrationDataIndex - DERATING_CALIBRATION_DATAINDEX_TABLE_START)] = fConvertedTemp;
#else
                gDeratingTable[(CalibrationElement[0] - DERATING_CALIBRATION_DATAINDEX_TABLE_START)] = (int16_t)(fConvertedTemp);
#endif
            }
            break;
    }

    return true;
}
#endif
/* -- END -- */
