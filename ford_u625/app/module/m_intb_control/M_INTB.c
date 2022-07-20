
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
    uint16_t u16AssertedTimer;
    uint16_t u16DeAssertedTimer;
}MINTB_Control;

/* -- Global Variables -- */
static MINTB_Control mINTBControl = 
{
    .eStateMachine = INTB_SM_WAIT_REGISTER,
    .eInterruptType = INTB_INT_TYPE_DEFAULT,
    .eStrategyCtrl = STRATEGY_CTRL_CANCEL,
    .CallbackSDMIntbDoSet = NULL,
    .u16AssertedTimer = 0x0000U,
    .u16DeAssertedTimer = 0x0000U,
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
static void mINTB_StateWaitRegister(uint16_t u16RoutineTime)
{
    if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_INIT)
    {
        mINTBControl.eStateMachine = INTB_SM_INIT;
    }

    (void)u16RoutineTime;
}
/**
 * @brief Waits for that system sets up INTB init.
 * 
 * @details N/A 
 * 
 * @note Calls MINTB_StrategyControl(STRATEGY_CTRL_INIT).
 * 
 */
static void mINTB_StateInit(uint16_t u16RoutineTime)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(1U);
    }

    /* Increates DeAsserted Timer. */
    mINTBControl.u16DeAssertedTimer = (mINTBControl.u16DeAssertedTimer < MINTB_SATISFIED_DEASSERTED_TIME)?\
    (mINTBControl.u16DeAssertedTimer + u16RoutineTime):\
    (MINTB_SATISFIED_DEASSERTED_TIME);

    mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;
}

static void mINTB_StateWaitTrigger(uint16_t u16RoutineTime)
{
    if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
    {
        mINTBControl.CallbackSDMIntbDoSet(0U);
    }
    else
    {
        mINTBControl.CallbackSDMIntbDoSet(1U);
    }

    /* Increates DeAsserted Timer. */
    mINTBControl.u16DeAssertedTimer = (mINTBControl.u16DeAssertedTimer < MINTB_SATISFIED_DEASSERTED_TIME)?\
    (mINTBControl.u16DeAssertedTimer + u16RoutineTime):\
    (MINTB_SATISFIED_DEASSERTED_TIME);

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

static void mINTB_StateSetup(uint16_t u16RoutineTime)
{
    if(mINTBControl.eStrategyCtrl == STRATEGY_CTRL_CANCEL)
    {
        mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;
        mINTBControl.u16DeAssertedTimer = 0;
    }
    else if(mINTBControl.u16DeAssertedTimer < MINTB_SATISFIED_DEASSERTED_TIME)
    {
        /* Wait Time */
        /* Increates DeAsserted Timer. */
        mINTBControl.u16DeAssertedTimer += u16RoutineTime;
    }
    else
    {
        /* Sets up INTB */
        if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
        {
            mINTBControl.CallbackSDMIntbDoSet(1U);
        }
        else
        {
            mINTBControl.CallbackSDMIntbDoSet(0U);
        }

        /* Clears DeAsserted Time */
        mINTBControl.u16DeAssertedTimer = 0;
        
        /* Increates Asserted Timer. */
        mINTBControl.u16AssertedTimer = (mINTBControl.u16AssertedTimer < MINTB_SATISFIED_ASSERTED_TIME)?\
        (mINTBControl.u16AssertedTimer + u16RoutineTime):\
        (MINTB_SATISFIED_ASSERTED_TIME);
        
        mINTBControl.eStateMachine = INTB_SM_HOLD_TIME;
    }
}

static void mINTB_StateHold(uint16_t u16RoutineTime)
{
    if(mINTBControl.u16AssertedTimer < MINTB_SATISFIED_ASSERTED_TIME)
    {
        /* Wait Time */
        mINTBControl.u16AssertedTimer += u16RoutineTime;
    }
    else
    {
        if(mINTBControl.eInterruptType == INTB_INT_TYPE_RISING)
        {
            mINTBControl.CallbackSDMIntbDoSet(0U);
        }
        else
        {
            mINTBControl.CallbackSDMIntbDoSet(1U);
        }

        /* Clears eStrategyCtrl */
        mINTBControl.eStrategyCtrl = STRATEGY_CTRL_CANCEL;
        
        /* Changes State Machine */
        mINTBControl.eStateMachine = INTB_SM_WAIT_TRIGGER;

        /* Clears Asserted Time */
        mINTBControl.u16AssertedTimer = 0;

        /* Increates DeAsserted Timer. */
        mINTBControl.u16DeAssertedTimer = (mINTBControl.u16DeAssertedTimer < MINTB_SATISFIED_DEASSERTED_TIME)?\
        (mINTBControl.u16DeAssertedTimer + u16RoutineTime):\
        (MINTB_SATISFIED_DEASSERTED_TIME);

    }
}

static void mINTB_StateDeInit(uint16_t u16RoutineTime)
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
    mINTBControl.u16AssertedTimer = 0x0000U;
    mINTBControl.u16DeAssertedTimer = 0x0000U;

    (void)u16RoutineTime;
}

/* -- Global Functions -- */
bool MINTB_Register(CALLBACK_SDM_INTB_DO_SET CallbackSDMIntbDoSet\
,MINTB_InttruptType_E eInterruptType)
{
    if((CallbackSDMIntbDoSet == NULL)\
        || ((eInterruptType != INTB_INT_TYPE_RISING) && (eInterruptType != INTB_INT_TYPE_FALLING))\
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
 *  init [label = "Init: \n Increates set-time."];
 *  wait_trigger [label = "Wait Trigger: \n 1.GPIO_INTB = H \n 2.Increates set-time. \n 3.Clears hold-time."];
 *  set_up [label = "Set Up: \n 1.Increates set-time. \n 2.GPIO_INTB = L when set-time >= 2ms."];
 *  hold [label = "Hold: \n 1.Clears set-time. \n 2.Increates hold-time. \n 3.GPIO_INTB = H when hold-time >= 2ms."];
 *  de_init [label = "Deinit: \n Rleases INTB Module."];
 * 
 *  init -> wait_trigger; 
 *  wait_trigger -> set_up [label = "Receives set up interrupt."];
 *  set_up -> hold [label = "GPIO_INTB is from H to L."];
 *  set_up -> wait_trigger [label = "Receives cancelled interrupt."];
 *  hold -> wait_trigger [label = "GPIO_INTB is from L to H."];
 *  wait_trigger -> de_init [label = "Receives power-off interrupt."];
 * }
 * @enddot
 * 
 */
void MINTB_Routine2ms(uint16_t u16RoutineTime)
{
    switch(mINTBControl.eStateMachine)
    {
        default:
            break;

        case INTB_SM_WAIT_REGISTER:
            mINTB_StateWaitRegister(u16RoutineTime);
            break;

        case INTB_SM_INIT:
            mINTB_StateInit(u16RoutineTime);
            break;

        case INTB_SM_WAIT_TRIGGER:
            mINTB_StateWaitTrigger(u16RoutineTime);
            break;

        case INTB_SM_SETUP_TIME:
            mINTB_StateSetup(u16RoutineTime);
            break;

        case INTB_SM_HOLD_TIME:
            mINTB_StateHold(u16RoutineTime);
            break;

        case INTB_SM_DEINIT:
            mINTB_StateDeInit(u16RoutineTime);
            break;
    }
}
/* -- END -- */
