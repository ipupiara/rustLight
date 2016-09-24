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

#define dispatchQMemSz    32

#define dispatchQSz  dispatchQMemSz

typedef struct pinData                                         
{
	INT8U     pinNr;
	INT8S     bytePos;
} pinPosData;

#define pinPosArraySize  13

pinPosData pinPosArray [pinPosArraySize] = {{1,0},{2,1},{3,2},
								{1,3},{2,4},{3,5},
								{1,6},{2,7},{3,8},
								{1,9},{2,16},{3,17},
								{1,24} };
									
#define strobePin	123		

dispatchMsg dispatchMsgArray[dispatchQMemSz];
void* dispatchQMsgPtrArray[dispatchQSz];

void  setPinAsOutputWithValue (CPU_INT16U pin, INT8U val)
{
	volatile  avr32_gpio_port_t  *gpio_port;

	gpio_port        = &AVR32_GPIO.port[pin >> 5];
	INT8U pinOfs     = pin & 0x1F;
	INT16U mask     = (1<< pinOfs);
	
	gpio_port->oders = mask; // The GPIO output driver is enabled for that pin.
	gpio_port->gpers = mask; // The GPIO module controls that pin.
	if (val == 0) {
		gpio_port->ovrc  = mask;
	} else {
		gpio_port->ovrs  = mask;
	}
	
}


INT32U shrUnmaskDWord(INT8U sLeft, INT32U sVal)
{
	INT32U msk = (1 << sLeft);
	INT32U  res = (sVal & msk);
	return res;
}



//  this method uses timedelay statements, but they
//should be as short as possible concerning the hardware driven
void dispatchMesg(dispatchMsg* dmPtr)   
{										
	INT32U msgW;
	msgW = dmPtr->dispatchData;
	INT8U i1 ;
	for ( i1 = 0; i1 < pinPosArraySize - 1; ++ i1 )  {
		INT8U valNotZero = (shrUnmaskDWord(pinPosArray[i1].bytePos, msgW) > 0);
		setPinAsOutputWithValue(pinPosArray[i1].pinNr,valNotZero);
	}	
	OSTimeDlyHMSM(0,0,0,1);
	setPinAsOutputWithValue( strobePin,1);
	OSTimeDlyHMSM(0,0,0,1);
	setPinAsOutputWithValue( strobePin,0);
}

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
			info_printf("tcpip received %X\n",dmPtr);
			dispatchMesg(dmPtr);				
	
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
			OSTaskNameSet(tcp_ip_Thread_TASK_PRIO, (INT8U *)"tcp_ip", &retVal);
		#endif
		
		
}
