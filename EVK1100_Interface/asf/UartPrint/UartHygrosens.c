/*
 * UartHygrosens.c
 *
 * Created: 07.01.2020 11:25:31
 *  Author: mira
 */ 
#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <stdarg.h>

#include "includes.h"
#include "UartHygrosens.h"

/*
 *
 *************   Hygrosense Receiver Stuff   ***********
 */


#define SerialHygrosensMethod_STK_SIZE   unique_STK_SIZE

OS_STK  SerialHygrosensMethodStk[SerialHygrosensMethod_STK_SIZE];



#define   HYGROSENS_USART_COM               1
#define   HYGROSENS_USART                   AVR32_USART1;

#define   HYGROSENS_USART_IRQ               AVR32_USART1_IRQ
#define	  hygrosensUsartSpeed   9600



#define startChar 0x40
#define stopChar   0x24
#define amtChars  66

enum rxStates {
	rxIdle,
	rxReceiving,
	rxReceived,
};

INT8U  rxState;
INT8U amtCharRcvd;

#define hygrosensMemorySz    5
#define hygrosensStrSz    68    // pn 9.June2012 attention: a value of e.g. 123 will give an unhandled exception.... ????
#warning: "exception tobe tested ???"

typedef CPU_INT08U hygrosensStringType [hygrosensStrSz];

typedef struct hygrosensStringMemory {
	hygrosensStringType serialStr;  // ATTENTION serialStr must be at first place, so that &(serialMem) == &(serialStr)
} hygrosensStringMemory;

hygrosensStringType* rxBuffer;

OS_EVENT *hygrosensQSem;
OS_MEM *hygrosensMsgMem;
OS_EVENT*  hygrosensTaskMsgQ;
hygrosensStringMemory hygrosensMem[hygrosensMemorySz];
void* hygrosensTaskMsgs[hygrosensMemorySz];


void OnHygrosensError()
{
	
}

void envoyBuffer(hygrosensStringType* memo)
{
	CPU_INT08U	err;
	err = OSQPost(hygrosensTaskMsgQ, (void *)memo);
	if ( err != OS_NO_ERR) {
		//	do something but dont loop--->> err_printf("Q post err tickHook\n");
		OnHygrosensError();
	}
}

hygrosensStringType* getBuffer(CPU_INT08U* err)
{	
	hygrosensStringType* res = NULL;

	res = (hygrosensStringType *) OSMemGet(hygrosensMsgMem, err);
	return res;
}


void addCharToBuffer(CPU_INT08U rxCh)
{
	CPU_INT08U err;
	if (rxCh == startChar)  {
		amtCharRcvd = 0;
		rxBuffer = getBuffer(&err);
		*rxBuffer[amtCharRcvd] = rxCh;
		rxState = rxReceiving;
	} else
	if (rxState == rxReceiving)  {
		++ amtCharRcvd;
		if (amtCharRcvd < hygrosensStrSz) {
			*rxBuffer [amtCharRcvd] = rxCh;
		}
		if (rxCh == stopChar) {   // no  chars lost
			rxState = rxReceived;
			envoyBuffer(rxBuffer);
		}
	}
}


static  void  SerialHygrosensThreadMethod (void *p_arg)
{
}


void  hygrosensRxTxISR (void)
{
	volatile  avr32_usart_t  *usart;
	INT8U err;

	usart = &HYGROSENS_USART;
	//    if (usart->CSR.rxrdy == 1) {
	//        APP_RxISRHandler();
	//    }
	//if (usart->CSR.txrdy == 1) {
		//err = OSSemPost(SerialPrintQSem);
		//if (err != OS_NO_ERR){
			////    		err_printf("couldnt signal sema in USART ISR\n");
		//}
		//BSP_USART_IntDis (PRINT_USART_COM, (1<< AVR32_USART_IER_TXRDY));
		//// PN 24. May 2012
		//// contrary to other AVR processors, it seems necessary to disable TXRDY interrupts
		//// manually, while other processors ATMega32/64 just trigger TXRDY interrupt once
		//// further testing will take place.
	//}
}





CPU_INT08U init_HygrosenseReceiver()
{
	serialHygrosensOn = 0;
	CPU_INT08U  err_init_hygrosens = OS_NO_ERR;
	
	if (err_init_hygrosens == OS_NO_ERR) {
		hygrosensMsgMem = OSMemCreate(&hygrosensMem[0], hygrosensMemorySz, sizeof(hygrosensStringMemory), &err_init_hygrosens);
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		OSMemNameSet(hygrosensMsgMem, (INT8U*)"hygrosensMsgMem", &err_init_hygrosens);
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		hygrosensTaskMsgQ = OSQCreate(&hygrosensTaskMsgs[0], hygrosensMemorySz);
		if (! hygrosensTaskMsgQ) err_init_hygrosens = 0xFF;
	}
	
	if (err_init_hygrosens == OS_NO_ERR )  {
		err_init_hygrosens = OSTaskCreateExt(SerialHygrosensThreadMethod,
		(void *)0,
		(OS_STK *)&SerialHygrosensMethodStk[SerialHygrosensMethod_STK_SIZE - 1],
		SERIAL_HYGROSENS_TASK_PRIO,
		SERIAL_HYGROSENS_TASK_PRIO,
		(OS_STK *)&SerialHygrosensMethodStk[0],
		SerialHygrosensMethod_STK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	}
		
	if (err_init_hygrosens == OS_NO_ERR )  {
		OSTaskNameSet(SERIAL_HYGROSENS_TASK_PRIO, (INT8U *)"HygrosensTask", &err_init_hygrosens);
	}
	
	if (err_init_hygrosens == OS_NO_ERR) {
		BSP_USART_Init (HYGROSENS_USART_COM, hygrosensUsartSpeed);  
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		if (BSP_INTC_IntReg(&hygrosensRxTxISR, HYGROSENS_USART_IRQ, 1) == BSP_INTC_ERR_NONE ) {
			err_init_hygrosens = OS_NO_ERR;
			} else {
			err_init_hygrosens = 0xFF;
		}
		BSP_USART_IntEn (HYGROSENS_USART_COM, (1<< AVR32_USART_IER_RXRDY));
	}
	serialHygrosensOn = (err_init_hygrosens == OS_NO_ERR);
	
	return err_init_hygrosens;
}
