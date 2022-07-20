/**
 * @file M_DetectTCHAttn.c
 * 
 * @author Orlando Huang (orlando.huang@auo.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * 
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
/* -- Includes -- */
#include "M_DetectTCHAttn.h"
#include "hal_uart.h"

/* -- Marco Define -- */
#if (M_DETECT_TCH_ATTN_EX_INT)
#define M_DETECT_TCH_ATTN_ISR_SETTING_RISING CY_GPIO_INTR_RISING 
#define M_DETECT_TCH_ATTN_ISR_SETTING_FALLING CY_GPIO_INTR_FALLING 
#endif


/**
 * @brief Read ATH_ATTN to Update State Machine.
 * 
 */
typedef enum
{
    TCH_ALERT_INIT = 0U,
    TCH_ALERT_WAIT_ALERT,
    TCH_ALERT_WAIT_RELEASE,
}MDetectTchAttn_StateMachine_E;

/* -- Type Define -- */
/**
 * @brief 
 * 
 */
typedef struct 
{
    MDetectTchAttn_StateMachine_E eStateMachine;
    MDetectTchAttn_ATTNTriggerType_E eAttnTriType;
#if (M_DETECT_TCH_ATTN_EX_INT)
    bool bRegisterPass;
#endif
    CALLBACK_TCH_CONTROLLER_ST_GET CallbackTchControllerGet;
    CALLBACK_TCH_ATTN_DI_GET CallbackTchAttnDiGet;
    CALLBACK_TCH_CLICK CallbackTchClick;
    CALLBACK_TCH_CLICK_RELEASE CallbackTchClickRel;
}MDetectTchAttn_Control;

/* -- Global Variables -- */
/**
 * @brief 
 * 
 */
static MDetectTchAttn_Control mDetectTchAttnControl = 
{
    .eAttnTriType = ATTN_TRI_FALLING,
    .eStateMachine = TCH_ALERT_INIT,
#if (M_DETECT_TCH_ATTN_EX_INT)
    .bRegisterPass = false,
#endif
    .CallbackTchControllerGet = NULL,
    .CallbackTchAttnDiGet = NULL,
    .CallbackTchClick = NULL,
    .CallbackTchClickRel = NULL,
};

#if (M_DETECT_TCH_ATTN_EX_INT)
static cy_stc_sysint_t tExternalInterruptConfig =
{
    .intrSrc = U301_TSC_ATTN_IRQ,
    .intrPriority = 3U,
};
#endif
/* -- Local Functions -- */
#if (M_DETECT_TCH_ATTN_EX_INT)
/**
 * @brief 
 * 
 */
static void MDetectTchAttn_Callback_BothEdgeISR(void)
{
    if(!mDetectTchAttnControl.bRegisterPass)
    {
        /* Waiting Register Finished. */
    }
    else
    {
        if(M_DETECT_TCH_ATTN_ISR_SETTING_FALLING == Cy_GPIO_GetInterruptEdge(U301_TSC_ATTN_PORT,U301_TSC_ATTN_NUM))
        {
            /* Set the interrupt trigger type to falling edge for ATTN */
            Cy_GPIO_SetInterruptEdge(U301_TSC_ATTN_PORT, U301_TSC_ATTN_NUM, M_DETECT_TCH_ATTN_ISR_SETTING_RISING);
            if(mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY)
            {

            }
            else
            {
                mDetectTchAttnControl.CallbackTchClick();
            }
            HAL_UART_Printf("ATTN Falling Edge ISR. \n");
        }
        else
        {
            /* Set the interrupt trigger type to rising edge for ATTN */
            Cy_GPIO_SetInterruptEdge(U301_TSC_ATTN_PORT, U301_TSC_ATTN_NUM, M_DETECT_TCH_ATTN_ISR_SETTING_FALLING);
            if(mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY)
            {
                HAL_UART_Printf("ATTN Rising Edge ISR - touch not ready. \n");
            }
            else
            {
                HAL_UART_Printf("ATTN Rising Edge ISR - touch ready. \n");
                mDetectTchAttnControl.CallbackTchClickRel();
            }
        }
    }
    
    /* Clears the triggered pin interrupt */
    Cy_GPIO_ClearInterrupt(U301_TSC_ATTN_PORT, U301_TSC_ATTN_NUM);
    NVIC_ClearPendingIRQ(tExternalInterruptConfig.intrSrc);
}
#endif

#if (!M_DETECT_TCH_ATTN_EX_INT)
/**
 * @brief 
 * 
 */
static void MDetectTchAttn_SMTouchAlertInit(void)
{
    if((mDetectTchAttnControl.CallbackTchAttnDiGet == NULL)\
        || (mDetectTchAttnControl.CallbackTchControllerGet == NULL)\
        || (mDetectTchAttnControl.CallbackTchClick == NULL)\
        || (mDetectTchAttnControl.CallbackTchClickRel == NULL)\
        || (mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY))
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_INIT;
    }
    else
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
    }
}
#endif

#if (!M_DETECT_TCH_ATTN_EX_INT)
/**
 * @brief 
 * 
 */
static void MDetectTchAttn_SMTouchAlertWaitAlert(void)
{
    if(mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY)
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
    }
    else
    {
        switch(mDetectTchAttnControl.eAttnTriType)
        {
            default:
                mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
                break;

            case ATTN_TRI_FALLING:
                if(mDetectTchAttnControl.CallbackTchAttnDiGet() == ATTN_TRI_FALLING)
                {
                    /* Gets Falling and Send Click Event to SDM. */
                    mDetectTchAttnControl.CallbackTchClick();
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_RELEASE;
                }
                else
                {
                    /* Waits Falling */
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
                }
                break;

            case ATTN_TRI_RISING:
                if(mDetectTchAttnControl.CallbackTchAttnDiGet() == ATTN_TRI_RISING)
                {
                    /* Gets Rising and Send Click Event to SDM. */
                    mDetectTchAttnControl.CallbackTchClick();
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_RELEASE;
                }
                else
                {
                    /* Waits Rising */
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
                }
                break;
        }
    }
}
#endif

#if (!M_DETECT_TCH_ATTN_EX_INT)
/**
 * @brief 
 * 
 */
static void MDetectTchAttn_SMTouchAlertWaitRel(void)
{
    if(mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY)
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
        mDetectTchAttnControl.CallbackTchClickRel();
    }
    else
    {
        switch(mDetectTchAttnControl.eAttnTriType)
        {
            default:
                mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_RELEASE;
                break;

            case ATTN_TRI_FALLING:
                if(mDetectTchAttnControl.CallbackTchAttnDiGet() == ATTN_TRI_RISING)
                {
                    /* Gets Rising and Send Click Event to SDM. */
                    mDetectTchAttnControl.CallbackTchClickRel();
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
                }
                else
                {
                    /* Waits Rising */
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_RELEASE;
                }
                break;

            case ATTN_TRI_RISING:
                if(mDetectTchAttnControl.CallbackTchAttnDiGet() == ATTN_TRI_FALLING)
                {
                    /* Gets Falling and Send Click Event to SDM. */
                    mDetectTchAttnControl.CallbackTchClickRel();
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
                }
                else
                {
                    /* Waits Falling */
                    mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_RELEASE;
                }
                break;
        }
    }
}
#endif
/* -- Global Functions -- */
/**
 * @brief 
 * 
 */
bool MDetectTchAttn_Register(\
CALLBACK_TCH_CONTROLLER_ST_GET CallbackTchControllerGet,\
CALLBACK_TCH_ATTN_DI_GET CallbackTchAttnDiGet,\
CALLBACK_TCH_CLICK CallbackTchClick,\
CALLBACK_TCH_CLICK_RELEASE CallbackTchClickRel,\
MDetectTchAttn_ATTNTriggerType_E eAttnTriType)
{
#if (M_DETECT_TCH_ATTN_EX_INT)
    if((CallbackTchControllerGet == NULL)\
    || (CallbackTchClick == NULL)\
    || (CallbackTchClickRel == NULL))
#else
    if((CallbackTchControllerGet == NULL)\
        || (CallbackTchAttnDiGet == NULL)\
        || (CallbackTchClick == NULL)\
        || (CallbackTchClickRel == NULL))
#endif
    {
        return false;
    }

    mDetectTchAttnControl.CallbackTchControllerGet = CallbackTchControllerGet;
    mDetectTchAttnControl.CallbackTchAttnDiGet = CallbackTchAttnDiGet;
    mDetectTchAttnControl.CallbackTchClick = CallbackTchClick;
    mDetectTchAttnControl.CallbackTchClickRel = CallbackTchClickRel;
    mDetectTchAttnControl.eAttnTriType = eAttnTriType;

#if (M_DETECT_TCH_ATTN_EX_INT)
    /* Registers Callback to both edge. */
    HAL_EXIT_Init(&tExternalInterruptConfig , MDetectTchAttn_Callback_BothEdgeISR);
    mDetectTchAttnControl.bRegisterPass = true;
#endif

    return true;
}

/**
 * @brief 
 * 
 */
void MDetectTchAttn_Routine2ms(void)
{

#if (M_DETECT_TCH_ATTN_EX_INT)
    if(mDetectTchAttnControl.CallbackTchAttnDiGet() == ATTN_TRI_RISING)
    {
        if(mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY)
        {
            HAL_UART_Printf("MDetectTchAttn_Routine2ms - touch not ready. \n");
        }
        else
        {
            HAL_UART_Printf("MDetectTchAttn_Routine2ms - touch ready - Clear. \n");
            mDetectTchAttnControl.CallbackTchClickRel();
        }
    }
#else
    switch(mDetectTchAttnControl.eStateMachine)
    {
        default:
            break;

        case TCH_ALERT_INIT:
            MDetectTchAttn_SMTouchAlertInit();
            break;

        case TCH_ALERT_WAIT_ALERT:
            MDetectTchAttn_SMTouchAlertWaitAlert();
            break;

        case TCH_ALERT_WAIT_RELEASE:
            MDetectTchAttn_SMTouchAlertWaitRel();
            break;
    }
#endif
}

/* -- END -- */
