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
#define rxBufferSz  68
enum rxStates {
	rxIdle,
	rxReceiving,
	rxReceived,
};

INT8U rxBuffer [rxBufferSz];
INT8U  rxState;
INT8U amtCharRcvd;

void envoyBuffer()
{
	
}


void addCharToBuffer(INT8U rxCh)
{
	if (rxCh == startChar)  {
		amtCharRcvd = 0;
		rxBuffer [amtCharRcvd] = rxCh;
		rxState = rxReceiving;
	} else
	if (rxState == rxReceiving)  {
		++ amtCharRcvd;
		if (amtCharRcvd < rxBufferSz) {
			rxBuffer [amtCharRcvd] = rxCh;
		}
		if (rxCh == stopChar) {   // no  chars lost
			rxState = rxReceived;
			envoyBuffer();
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
		BSP_USART_Init (HYGROSENS_USART_COM, hygrosensUsartSpeed);  

//		if (!(SerialPrintQSem = OSSemCreate(0)))
		// after initialisation, probable a character can be sent
		// worst case we just loose it
//		{
			//err_printf("could not create serialQSem\n");
			err_init_hygrosens = 0xFF;
//		}
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		if (BSP_INTC_IntReg(&hygrosensRxTxISR, HYGROSENS_USART_IRQ, 1) == BSP_INTC_ERR_NONE ) {
			err_init_hygrosens = OS_NO_ERR;
			} else {
			err_init_hygrosens = 0xFF;
		}
		BSP_USART_IntEn (HYGROSENS_USART_COM, (1<< AVR32_USART_IER_RXRDY));
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
	serialHygrosensOn = (err_init_hygrosens == OS_NO_ERR);
	
	
/*	serialPrintOn = 0;
	amtErrorPrintfCalls = 0;
	amtPrintErr = 0;
	err_init_print = OS_NO_ERR;

	if (err_init_print == OS_NO_ERR) {
		BSP_USART_Init (APP_USART_PRINT_COM, usartSpeed);  // start with something slow, later increase

		if (!(SerialPrintQSem = OSSemCreate(0)))
		// after initialisation, probable a character can be sent
		// worst case we just loose it
		{
			//err_printf("could not create serialQSem\n");
			err_init_print = 0xFF;
		}
	}
	
	if (err_init_print == OS_NO_ERR)  {
		#ifdef use_USART_PRINT_PDCA
		initPDCA(&err_init_print);
		#endif
	}
	memset(serialPrintStrBuf,0x00,sizeof(serialPrintStrBuf));
	
	if (err_init_print == OS_NO_ERR) {
		serialPrintMsgMem = OSMemCreate(&serialPrintStrBuf[0], serialPrintStrBufSz, sizeof(serialPrintMem), &err_init_print);
	}
	if (err_init_print == OS_NO_ERR) {
		OSMemNameSet(serialPrintMsgMem, (INT8U*)"serialMsgMem", &err_init_print);
	}
	if (err_init_print == OS_NO_ERR) {
		serialPrintMsgTaskQ = OSQCreate(&serialPrintTaskQMsg[0], serialPrintStrBufSz);
		if (! serialPrintMsgTaskQ) err_init_print = 0xFF;
	}

	#ifdef use_USART_TxRdyInt

	if (err_init_print == OS_NO_ERR) {
		if (BSP_INTC_IntReg(&APP_USARTRxTxISR, App_IRQ_USART, 1) == BSP_INTC_ERR_NONE ) {
			err_init_print = OS_NO_ERR;
			} else {
			err_init_print = 0xFF;
		}
		BSP_USART_IntEn (APP_USART_PRINT_COM, (1<< AVR32_USART_IER_TXRDY));
	}
	#endif
	
	if (err_init_print == OS_NO_ERR )  {
		err_init_print = OSTaskCreateExt(SerialQMethod,
		(void *)0,
		(OS_STK *)&SerialPrintQMethodStk[SerialpRrintQMethod_STK_SIZE - 1],
		SerialQ_TASK_PRIO,
		SerialQ_TASK_PRIO,
		(OS_STK *)&SerialPrintQMethodStk[0],
		SerialpRrintQMethod_STK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	}
	
	OSTaskNameSet(SerialQ_TASK_PRIO, (INT8U *)"SerQ", &err_init_print);
	serialPrintOn = (err_init_print == OS_NO_ERR);
	
	*/
	return err_init_hygrosens;
}
