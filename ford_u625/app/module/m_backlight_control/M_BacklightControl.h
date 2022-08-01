#ifndef M_BACKLIGHT_CONTROL_H
#define M_BACKLIGHT_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- Includes -- */
#include "main.h"

/* -- Marco Define -- */
/**
 * @brief The define const values for controling backlight state machine.
 * 
 */
typedef enum
{
    E_MBL_STATEMACHINE_INIT = 0U, /**< 0U is Init. */
    E_MBL_STATEMACHINE_NORMAL, /**< 1U is Normal. */
    E_MBL_STATEMACHINE_DERATING, /**< 2U is Derating. */
    E_MBL_STATEMACHINE_SHUTDOWM, /**< 3U is Shutdown. */
    E_MBL_STATEMACHINE_WAITENABLE, /**< 4U is Waits backlight enable at boot. */
}MBacklightControlStateMachine_E;
/**
 * @brief 
 * 
 */
typedef enum
{
    E_MBL_EXTERNAL_ENABLE_DIMMING = 0xAAU,
    E_MBL_EXTERNAL_DISABLE_DIMMING = 0xABU,
    E_MBL_EXTERNAL_ENABLE_NODIMMNG = 0xACU,
    E_MBL_EXTERNAL_DISABLE_NODIMMNG = 0xADU,
}MBacklightControlExternalEnable_E;
/**
 * @brief 
 * 
 */
typedef enum
{
    E_MBL_ENABLE_DERATING = 0xAAU,
    E_MBL_DISABLE_DERATING = 0xABU,
}MBacklightControlDeratingEnable_E;
/* -- Data Type Define -- */
typedef void (*CALLBACK_EX_ALERT_HANDLER_COMPLETE)(void);

/* -- Extern Global Variables -- */

/* -- Extern Functions -- */
extern void MBacklightControl_TurnOnOffDerating(MBacklightControlDeratingEnable_E eMBLCtrlDeratingEn);
extern void MBacklightControl_ExternalTurnOnOffBL(MBacklightControlExternalEnable_E eMBLCtrlExternalEn);
extern bool MBacklightControl_RegisterExternalEnableCallback(
CALLBACK_EX_ALERT_HANDLER_COMPLETE CallbackExAlertEnComplete,\
CALLBACK_EX_ALERT_HANDLER_COMPLETE CallbackExAlertDisComplete);
extern MBacklightControlStateMachine_E MBacklightControl_TriggerStateMachine(void);
extern void MBacklightControl_DeratingModuleRoutine10ms(void);
extern void MBacklightControl_DimmingModuleRoutine6ms(void);
#endif
/* -- END -- */