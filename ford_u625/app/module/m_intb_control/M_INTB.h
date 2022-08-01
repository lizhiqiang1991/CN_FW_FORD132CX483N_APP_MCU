#ifndef M_INTB_H
#define M_INTB_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- Includes -- */
#include "main.h"

/* -- Marco Define -- */
#define MINTB_SATISFIED_DEASSERTED_TIME (uint16_t)2u
#define MINTB_SATISFIED_ASSERTED_TIME (uint16_t)2u

typedef enum
{
    STRATEGY_CTRL_CANCEL = 0U,
    STRATEGY_CTRL_START,
    STRATEGY_CTRL_INIT,
    STRATEGY_CTRL_DEINIT,
}MINTB_StrategyCtrl_E;

typedef enum
{
    INTB_INT_TYPE_RISING = 0U,
    INTB_INT_TYPE_FALLING,
    INTB_INT_TYPE_DEFAULT,
}MINTB_InttruptType_E;

/* -- Data Type Define -- */
typedef void (*CALLBACK_SDM_INTB_DO_SET)(uint8_t u8DigitalLevel); 

/* -- Extern Global Variables -- */

/* -- Extern Functions -- */
extern bool MINTB_Register(CALLBACK_SDM_INTB_DO_SET CallbackSDMIntbDoSet, MINTB_InttruptType_E eInterruptType);
extern bool MINTB_StrategyControl(MINTB_StrategyCtrl_E eStrategyCtrl);
extern void MINTB_Routine(uint16_t u16RoutineTimeMs);

#endif
/* -- END -- */
