/*
 * ifDispatcher.c
 *
 * Created: 20.09.2016 15:53:22
 *  Author: mira
 */ 

#include  <includes.h>

#include <stdlib.h>
//#include <malloc.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

//#include "sdramc.h"
#include "ifDispatcher.h"
#include "UartPrint.h"


OS_STK  dispatcherMethod_STK[ifDispatcher_Thread_Method_STK_SIZE];

#define dispatchQMemSz    14

#define dispatchQSz  dispatchQMemSz

dispatchMsg dispatchMsgArray[dispatchQMemSz];
void* dispatchQMsgPtrArray[dispatchQSz];


void initIfDipatcher()
{
	INT8U	err_init = OS_NO_ERR;

		if (err_init == OS_NO_ERR) {
			memset(dispatchMsgArray,0x00,sizeof(dispatchMsgArray));
		}
		
		if (err_init == OS_NO_ERR) {
			dispatchMsgMem = OSMemCreate(&dispatchMsgArray[0], dispatchQMemSz, sizeof(dispatchMsg), &err_init);
		}
		if (err_init == OS_NO_ERR) {
			OSMemNameSet(dispatchMsgMem, (INT8U*)"dispatchMsgMem", &err_init);
		}
		if (err_init == OS_NO_ERR) {
			dispatchMsgQ = OSQCreate(&dispatchQMsgPtrArray[0], dispatchQSz);
			if (! dispatchMsgQ) err_init = 0xFF;
		}
		if (err_init != OS_NO_ERR) {
			err_printf("error initDispatcher, err_init = %i\n",err_init);
		}
}

static  void  ifDispatcher_Thread_Method (void *p_arg)
{
	INT8U err;
	dispatchMsg* dmPtr;

	while (1) {
		dmPtr = (dispatchMsg *)OSQPend(dispatchMsgQ, 1051, &err);
		if (err == OS_NO_ERR) {
				
	
			err = OSMemPut(dispatchMsgMem, (void *)dmPtr);
			if (err != OS_NO_ERR) {
				err_printf("error put back memory in method ifDispatcher_Thread_Method, err = %d\n ",err);
			}
		} else {
			if (err != OS_TIMEOUT)  {
				err_printf("error OSQPend in method ifDispatcher_Thread_Method, err = %d, != OS_Timeout !\n",err);
			}
		}
	}
}

void startIfDispatcher()
{
		INT8U retVal = 0;
		retVal = OSTaskCreateExt(ifDispatcher_Thread_Method,
						(void *)0,
						(OS_STK *)&dispatcherMethod_STK[ifDispatcher_Thread_Method_STK_SIZE - 1],
						ifDispatcher_Thread_TASK_PRIO,
						ifDispatcher_Thread_TASK_PRIO,
						(OS_STK *)&dispatcherMethod_STK[0],
						ifDispatcher_Thread_Method_STK_SIZE,
						(void *)0,
						OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		
		if (retVal !=  OS_NO_ERR  ) {
			err_printf("error create sec100 tcp_ip Thread\n");
		}  else {
			info_printf("ifDispatcher started\n");
		}

		#if (OS_TASK_NAME_SIZE > 10)
			OSTaskNameSet(tcp_ip_Thread_TASK_PRIO, (CPU_CHAR *)"tcp_ip", &retVal);
		#endif
		
		
}
