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

/* -- Marco Define -- */
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
    .CallbackTchControllerGet = NULL,
    .CallbackTchAttnDiGet = NULL,
    .CallbackTchClick = NULL,
    .CallbackTchClickRel = NULL,
};

/* -- Local Functions -- */
/**
 * @brief 
 * 
 */
static void MDetectTchAttn_SMTouchAlertInit(void)
{
    if((mDetectTchAttnControl.CallbackTchAttnDiGet == NULL) &&\
        (mDetectTchAttnControl.CallbackTchControllerGet == NULL) &&\
        (mDetectTchAttnControl.CallbackTchClick == NULL) &&\
        (mDetectTchAttnControl.CallbackTchClickRel == NULL) &&\
        (mDetectTchAttnControl.CallbackTchControllerGet() == TCH_CONTROLLER_NOTREADY))
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_INIT;
    }
    else
    {
        mDetectTchAttnControl.eStateMachine = TCH_ALERT_WAIT_ALERT;
    }
}
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
    if((CallbackTchControllerGet == NULL)\
        || (CallbackTchControllerGet == NULL)\
        || (CallbackTchControllerGet == NULL)\
        || (CallbackTchControllerGet == NULL))
    {
        return false;
    }

    mDetectTchAttnControl.CallbackTchControllerGet = CallbackTchControllerGet;
    mDetectTchAttnControl.CallbackTchAttnDiGet = CallbackTchAttnDiGet;
    mDetectTchAttnControl.CallbackTchClick = CallbackTchClick;
    mDetectTchAttnControl.CallbackTchClickRel = CallbackTchClickRel;
    mDetectTchAttnControl.eAttnTriType = eAttnTriType;
    return true;
}
/**
 * @brief 
 * 
 */
void MDetectTchAttn_Routine2ms(void)
{
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
}

/* -- END -- */
