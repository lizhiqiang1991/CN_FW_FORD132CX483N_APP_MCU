
/* -- Includes -- */
#include "M_INTB.h"

/* -- Marco Define -- */
typedef enum
{
    INTB_SM_WAIT_REGISTER = 0U,
    INTB_SM_INIT,
    INTB_SM_WAIT_TRIGGER,
    INTB_SM_SETUP_TIME,
    INTB_SM_HOLD_TIME,
    INTB_SM_DEINIT,
}MINTB_StateMachine_E;

/* -- Type Define -- */
typedef struct
{
    MINTB_StateMachine_E eStateMachine;
    MINTB_InttruptType_E eInterruptType;
    MINTB_StrategyCtrl_E eStrategyCtrl;
    CALLBACK_SDM_INTB_DO_SET CallbackSDMIntbDoSet;
}MINTB_Control;

/* -- Global Variables -- */
static MINTB_Control mINTBControl = 
{
    .eStateMachine = INTB_SM_WAIT_REGISTER,
    .eInterruptType = INTB_INT_TYPE_DEFAULT,
    .eStrategyCtrl = STRATEGY_CTRL_CANCEL,
    .CallbackSDMIntbDoSet = NULL,
};

/* -- Local Functions -- */
/**
 * @brief Waits for registering callback func. which controls INTB GPIO and trigger type.
 * 
 * @details N/A 
 * 
 * @note Calls MINTB_Register() to register these.
 * 
 */
static void mINTB_StateWaitRegister(void)
{
    if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_INIT)
    {
        mINTBControl.eStateMachine = INTB_SM_INIT;
    }
}
/**
 * @brief Waits for that system sets up INTB init.
 * 
 * @details N/A 
 * 
 * @note Calls MINTB_StrategyControl(STRATEGY_CTRL_INIT).
 * 
 */
static void mINTB_StateInit(void)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(1U);
    }

    mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;
}

static void mINTB_StateWaitTrigger(void)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(1U);
    }

    if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_START)
    {
        mINTBControl.eStateMachine = INTB_SM_SETUP_TIME;
    }
    else if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_DEINIT)
    {
        mINTBControl.eStateMachine = INTB_SM_DEINIT;
    }
    else{/* Wait for trigger */}
}

static void mINTB_StateSetup(void)
{
    if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_START)
    {
        mINTBControl.eStateMachine = INTB_SM_HOLD_TIME;
    }
    else
    {
        mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;
    }
}

static void mINTB_StateHold(void)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(1U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }

    /* Clears eStrategyCtrl */
    mINTBControl.eStrategyCtrl = STRATEGY_CTRL_CANCEL;
    /* Changes State Machine */
    mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;
}

static void mINTB_StateDeInit(void)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }

    /* Deinit mINTBControl */
    mINTBControl.CallbackSDMIntbDoSet = NULL;
    mINTBControl.eInterruptType = INTB_INT_TYPE_DEFAULT;
    mINTBControl.eStateMachine = INTB_SM_WAIT_REGISTER;
}

/* -- Global Functions -- */
bool MINTB_Register(CALLBACK_SDM_INTB_DO_SET CallbackSDMIntbDoSet,MINTB_InttruptType_E eInterruptType)
{
    if((CallbackSDMIntbDoSet == NULL)\
        || ((eInterruptType != INTB_INT_TYPE_RISING)\
            && (eInterruptType != INTB_INT_TYPE_FALLING))
        || (mINTBControl.eStateMachine != INTB_SM_WAIT_REGISTER))
    {
        return false;
    }

    mINTBControl.CallbackSDMIntbDoSet = CallbackSDMIntbDoSet;
    mINTBControl.eInterruptType = eInterruptType;

    return true;
}

bool MINTB_StrategyControl(MINTB_StrategyCtrl_E eStrategyCtrl)
{
    if((mINTBControl.CallbackSDMIntbDoSet == NULL)\
        || (mINTBControl.eInterruptType == INTB_INT_TYPE_DEFAULT))
    {
        return false;
    }
    else
    {
        mINTBControl.eStrategyCtrl = eStrategyCtrl;
    }

    return true;
}
/**
 * @brief INTB strategy control routine in period of 2ms.
 * 
 * @details N/A 
 * 
 * @note Sends communication event to call MINTB_StrategyControl() and change state machine.
 * 
 * @dot
 * digraph INTBStateMachine{
 *  WINIT [label = "Wait Init:\nNA"];
 *  INIT [label = "Init:\nGPIO_INTB = H"];
 *  WT [label = "Wait Trigger:\nNA"];
 *  SU [label = "Set Up:\nGPIO_INTB = H"];
 *  HO [label = "Hold:\nGPIO_INTB = L"];
 *  DEINIT [label = "Deinit:\n1.GPIO_INTB = L\n2.Reset State machine."];
 * 
 *  WINIT -> INIT [label = "STRATEGY_CTRL_INIT"];
 *  INIT -> WT; 
 *  WT -> SU [label = "STRATEGY_CTRL_START"];
 *  SU -> HO [label = "Wait 2ms"];
 *  SU -> WT [label = "STRATEGY_CTRL_CANCEL"];
 *  HO -> WT [label = "Wait 2ms"];
 *  WT -> DEINIT [label = "STRATEGY_CTRL_DEINIT"];
 * }
 * @enddot
 * 
 */
void MINTB_Routine2ms(void)
{
    switch(mINTBControl.eStateMachine)
    {
        default:
            break;

        case INTB_SM_WAIT_REGISTER:
            mINTB_StateWaitRegister();
            break;

        case INTB_SM_INIT:
            mINTB_StateInit();
            break;

        case INTB_SM_WAIT_TRIGGER:
            mINTB_StateWaitTrigger();
            break;

        case INTB_SM_SETUP_TIME:
            mINTB_StateSetup();
            break;

        case INTB_SM_HOLD_TIME:
            mINTB_StateHold();
            break;

        case INTB_SM_DEINIT:
            mINTB_StateDeInit();
            break;
    }
}
/* -- END -- */
