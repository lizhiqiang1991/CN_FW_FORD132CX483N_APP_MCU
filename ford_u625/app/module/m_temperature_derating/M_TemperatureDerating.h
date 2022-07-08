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
#else
#define TEMP_DERATING_TEMP_RESOLUTION 1 /* The minimal unit of derating threshold */
#define TEMP_DERATING_DEFAULT_OUTDATA 100  /* The default value of backlight PWM duty. */
#define TEMP_DERATING_FULL_OUTDATA 100  /* The full value of backlight PWM duty. */
#define TEMP_DERATING_DEFAULT_DERA_TEMP (70 * TEMP_DERATING_TEMP_RESOLUTION) /* The default value of derating threshold */
#endif

#define TEMP_DERATING_TABLE_SIZE 31 /* The data size of gDeratingTable. */

#define TEMP_DERATING_INIT_WAIT_TIME_OUT 10000U /* The threshold is maximal acceptable time. */

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
extern void TemperatureDerating_Init(float DeratingSettingTemp);
#else
extern void TemperatureDerating_Init(int16_t DeratingSettingTemp);
#endif

#if(BACKDOOR_WRITE_DERATINGTABLE)
extern bool TemperatureDerating_DeratingTable_Set(volatile uint8_t *AllTableElement);
#endif

#if(DERATINGAPP_FLOAT_OPERATION)
extern void TemperatureDerating_DeratingTemp_Set(float DeratingSettingTemp);
#if(BACKDOOR_WRITE_DERATINGTABLE)
extern float* TemperatureDerating_DeratingTable_Get(void);
#endif
#else
extern void TemperatureDerating_DeratingTemp_Set(int16_t DeratingSettingTemp);
#if(BACKDOOR_WRITE_DERATINGTABLE)
extern int16_t* TemperatureDerating_DeratingTable_Get(void);
#endif
#endif
#endif
/* -- END -- */
