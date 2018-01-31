/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                      ATMEL  AVR32 UC3 Sample code
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : APP.C
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

#include  <includes.h>
#include <stdlib.h>
//#include <malloc.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "sdramc.h"
#include "UartPrint.h"
#if uC_TCPIP_MODULE > 0
#include "tcpipApp.h"     
#include "ifDispatcher.h"                                           
#endif


#define showLED


/*
**************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/

OS_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/

static  void  AppTaskStart(void *p_arg);
static  void  AppTaskCreate(void);

/*
*    globals and enums
*/

typedef struct sdramSpace{
//	CalculationTable01   calcTable01;
//	Snapshot01  snapshots [scoreQMsgSize];
//	TopHisto     topHisto;
	CPU_INT08U  dummyData [1000];
} sdramSpace;

typedef volatile sdramSpace* sdP;

sdP sdp = SDRAM;



/*$PAGE*/
/*
**************************************************************************************************************
*                                                MAIN
**************************************************************************************************************
*/

int  main (void)
{
#if (OS_TASK_NAME_SIZE > 7) && (OS_TASK_STAT_EN > 0)
    CPU_INT08U  err;
#endif


    CPU_IntDis();                                                       /* Disable all interrupts until we are ready to accept them */
    OSInit();  	
	BSP_Init();          
	
	init_err_printf();                                           

    OSTaskCreateExt(AppTaskStart,                                      
                    (void *)0,
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppTaskStartStk[0],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE > 7) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_CHAR *)"Startup", &err);
#endif

    OSStart();     // pn 15.jan 2017 why here appTaskStart and OSStart will execute concurrently ???? is this always ok?
	                                                   
	info_printf("EVK1100_Interface to cyclone-III rustlight fpga starting\n");

    return (0);                                                        
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description: This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments  : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT08U  i;


    (void)p_arg;
                                                        

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                       /* Determine CPU capacity                                  */
#endif

#if uC_TCPIP_MODULE > 0
	AppInit_TCPIP();                                               
#endif

	initIfDipatcher();
	
    AppTaskCreate();                                                


	INT32U   loopCnt = 0;
    while (1) {          
		loopCnt ++;
        for (i = 1; i <= 6; i++) {
#ifdef showLED
			LED_On(i);
#endif			
            OSTimeDlyHMSM(0, 0, 4, 0);
#ifdef showLED
            LED_Off(i);
#endif
        }
        for (i = 4; i <= 7; i++) {
#ifdef showLED
            LED_On(9 - i);
#endif			
            OSTimeDlyHMSM(0, 0, 4, 0);
#ifdef showLED
            LED_Off(9 - i);
#endif			
        }
//		info_printf("AppTaskStart loopCnt: %u \r\n",loopCnt);
		info_printf(".");
    }
}


/*
**************************************************************************************************************
*                                        CREATE APPLICATION TASKS
*
* Description: This function creates the application tasks.
*
* Arguments  : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
**************************************************************************************************************
*/

// bad luck the following methods need (more or less) to be implemented as well  (or some own defines
// needed to be implemented into
// the framework code, what is not so nice for future possible upgrades

// moreover it is not possible to compile an application with OS_TASK_SW_HOOK_EN set to 0
// because this will give a build error in any case what shows that the
// framework code itself is not so completely finished in its development.
// so it seems the easiest just to implement these method, since probable they get optimized out anyhow

void          App_TaskCreateHook      (OS_TCB          *ptcb){}
void          App_TaskDelHook         (OS_TCB          *ptcb){}
void          App_TaskIdleHook        (void){}
void          App_TaskStatHook        (void){}
void          App_TCBInitHook         (OS_TCB          *ptcb){}
void          App_TaskSwHook          (void){}

void          App_TimeTickHook        (void)   {}
	

static  void  AppTaskCreate (void)
{  
	
#if uC_TCPIP_MODULE > 0
	startTcpipThread();
#endif
	startIfDispatcher();
}

















