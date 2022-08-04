/* -- Avoid Re-Includes -- */
#ifndef M_TEMPERATUREDERATING_H
#define M_TEMPERATUREDERATING_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* -- Includes -- */
#include "main.h"
#include "M_Derating.h"

/* -- Marco Define -- */
#define TEMP_DERATINGAPP_TEST false /* a compile option for simulative testing. */

#define TEMP_DERATING_METHOD_PERIOD_TIME 10U /* a minimal period time to check backlight derating state functions. */
#define TEMP_DERATING_METHOD_CHECK_TIME 20U /* a minimal period time to call backlight derating state functions. */

#if(DERATINGAPP_FLOAT_OPERATION)
#define TEMP_DERATING_DEFAULT_OUTDATA 100.0f /* The default value of backlight PWM duty. */
#define TEMP_DERATING_FULL_OUTDATA 100.0f /* The full value of backlight PWM duty. */
#define TEMP_DERATING_DEFAULT_DERA_TEMP 70.0f /* The default value of derating threshold */
#define TEMP_DERATING_DEFAULT_REDUCEBL_TEMP 85.0f /* The default value of reduce threshold */
#define TEMP_DERATING_DEFAULT_SHUTDOWN_TEMP 96.0f /* The default value of shut down threshold */
#define TEMP_DERATING_DEFAULT_SHUTDOWN_REL_TEMP 94.0f /* The default value of shut down release threshold */
#else
#define TEMP_DERATING_TEMP_RESOLUTION 1 /* The minimal unit of derating threshold */
#define TEMP_DERATING_DEFAULT_OUTDATA 100  /* The default value of backlight PWM duty. */
#define TEMP_DERATING_FULL_OUTDATA 100  /* The full value of backlight PWM duty. */
#define TEMP_DERATING_DEFAULT_DERA_TEMP (70 * TEMP_DERATING_TEMP_RESOLUTION) /* The default value of derating threshold */
#define TEMP_DERATING_DEFAULT_REDUCEBL_TEMP (85 * TEMP_DERATING_TEMP_RESOLUTION) /* The default value of reduce threshold */
#define TEMP_DERATING_DEFAULT_SHUTDOWN_TEMP (96 * TEMP_DERATING_TEMP_RESOLUTION) /* The default value of shut down threshold */
#define TEMP_DERATING_DEFAULT_SHUTDOWN_REL_TEMP (94 * TEMP_DERATING_TEMP_RESOLUTION) /* The default value of shut down release threshold */
#endif

#define TEMP_DERATING_TABLE_SIZE 31 /* The data size of gDeratingTable. */

#define TEMP_DERATING_INIT_WAIT_TIME_OUT 10000U /* The threshold is maximal acceptable time. */

typedef enum
{
    LIMITED_DERATING_TEMP = 0,
    LIMITED_SHUTDOWN_TEMP,    
    LIMITED_SHUTDOWN_REL_TEMP,
    LIMITED_REDUCE_BL_TEMP,
}TEMP_DERATING_LIMITED_SELECTION;

#if(BACKDOOR_WRITE_DERATINGDATA)
#define TEMP_DERATING_RESOLUTION 1
#define TEMP_DERATING_OFFSET -40
#define TEMP_DERATING_LIMITED_TEMPERATURE_LENGTH 8
#define TEMP_DERATING_TABLE_TEMPERATURE_LENGTH 31

typedef enum
{
    DERATING_CALIBRATION_DATAINDEX_DERATING_TEMP = 1,
    DERATING_CALIBRATION_DATAINDEX_SHUTDOWN_TEMP,
    DERATING_CALIBRATION_DATAINDEX_SHUTDOWN_REL_TEMP,
    DERATING_CALIBRATION_DATAINDEX_SHOWN_TERR_TEMP,
    DERATING_CALIBRATION_DATAINDEX_TABLE_START,
}TEMP_DERATING_CALIBRATION_DATAINDEX;
#endif
/* -- Data Type Define -- */

/* -- Extern Global Variables -- */
#if(TEMP_DERATINGAPP_TEST)
#if(DERATINGAPP_FLOAT_OPERATION)
extern float gSimuBLTemperature; /* The simulative backlight temperature for testing. */
#else
extern int16_t gSimuBLTemperature; /* The simulative backlight temperature for testing. */
#endif
#endif

/* -- Extern Functions -- */
#if(DERATINGAPP_FLOAT_OPERATION)
extern void TemperatureDerating_Init(void);
extern float TemperatureDerating_GetLimitedTemperature(TEMP_DERATING_LIMITED_SELECTION TempDeratingLimitedSelection);
#else
extern void TemperatureDerating_Init(void);
extern int16_t TemperatureDerating_GetLimitedTemperature(TEMP_DERATING_LIMITED_SELECTION TempDeratingLimitedSelection);
#endif

#if(BACKDOOR_WRITE_DERATINGDATA)
extern bool TemperatureDerating_LimitedTemperature_Get(volatile uint8_t *CalibrationElement, uint8_t CalibrationElementLength);
extern bool TemperatureDerating_DeratingTable_Get(volatile uint8_t *CalibrationElement, uint8_t CalibrationElementLength);
extern bool TemperatureDerating_DeratingCalibrationData_Set(volatile uint8_t *CalibrationElement);
#endif

#endif
/* -- END -- */
