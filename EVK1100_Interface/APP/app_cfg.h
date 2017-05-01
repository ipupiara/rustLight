/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                            ATMEL  AVR32 UC3  Application Configuration File
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : APP_CFG.H
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE CONFIGURATION
*********************************************************************************************************
*/

#include <ttcp_cfg.h>


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  uC_TCPIP_MODULE                 DEF_ENABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
//#define  uC_TCPIP_MODULE                 DEF_DISABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */

#define  uC_TTCP_MODULE                  DEF_DISABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
//#warning might be changed
/*
**************************************************************************************************************
*                                               STACK SIZES
**************************************************************************************************************
*/

//#define unique_STK_SIZE          1300
#define super_STK_SIZE          2048
#define unique_STK_SIZE          1024

#define  APP_TASK_START_STK_SIZE          unique_STK_SIZE

#define  tcp_ip_Thread_Method_STK_SIZE          unique_STK_SIZE  

#define  NET_OS_CFG_TMR_TASK_STK_SIZE        172
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE      384
#define  ifDispatcher_Thread_Method_STK_SIZE   unique_STK_SIZE

/*
**************************************************************************************************************
*                                             TASK PRIORITIES
**************************************************************************************************************
*/

#define  APP_TASK_START_PRIO                1
#define  OS_TASK_TMR_PRIO                   5


#define  SerialQ_TASK_PRIO                   13

#define tcp_ip_Thread_TASK_PRIO              15
#define ifDispatcher_Thread_TASK_PRIO         16


#define  NET_OS_CFG_IF_RX_TASK_PRIO            7   // lower prio as recommended in tcp-ip manuals, pn 21 aug 12
#define  NET_OS_CFG_TMR_TASK_PRIO             8

//#warning might be changed .. check amount processed and prios for all processes in application -integraton of net processes


#define BOARD EVK1100
