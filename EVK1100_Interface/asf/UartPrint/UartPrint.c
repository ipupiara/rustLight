/*
 * UartPrint.c
 *
 *  Created on: 30.05.2012
 *      Author: duenda
 */

#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <stdarg.h>



#include "includes.h"
#include "UartPrint.h"
//#include "udc.h"



CPU_INT32U  debugUartLoopCnt, debugUartPosFlag;

/*
 *
 *    Serial  printing stuff
 *
 *
 */
#define SerialpPrintQMethod_STK_SIZE   unique_STK_SIZE

OS_STK  SerialPrintQMethodStk[SerialpPrintQMethod_STK_SIZE];
//static  void  SerialQMethod (void *p_arg);


#define serialPrintStrBufSz    14
#define serialPrintStrSz    128    // pn 9.June2012 attention: a value of e.g. 123 will give an unhandled exception.... ????
#warning: "exception tobe tested ???"
typedef struct serialPrintMem {
  char serialStr [serialPrintStrSz];  // ATTENTION serialStr must be at first place, so that &(serialMem) == &(serialStr)
} serialPrintMem;

CPU_INT08U  serialPrintOn;

OS_EVENT *SerialPrintQSem;
OS_MEM *serialPrintMsgMem;
OS_EVENT*  serialPrintMsgTaskQ;
serialPrintMem serialPrintStrBuf[serialPrintStrBufSz];
void* serialPrintTaskQMsg[serialPrintStrBufSz];

// error print methods, just try to work a bit with hardware and BSP
//

#ifdef use_USART_PRINT_PDCA

#define PDCA_CHANNEL_USART_PRINTF 0
#define AVR32_PDCA_PID_USART_PRINT_TX       AVR32_PDCA_PID_USART0_TX
#define App_IRQ_PDCA_USART_PRINT  AVR32_PDCA_IRQ_0

CPU_INT32U amtPdcaErrors;

OS_FLAG_GRP *pdcaStatus;
#define TransferFinished (OS_FLAGS)0x0001
#define ReloadCounterZero (OS_FLAGS)0x0002

void* reloadDataP;
void* transferDataP;

#endif

#define   PRINT_USART_COM               0
#define   PRINT_USART                   AVR32_USART0;

#ifdef use_USART_PRINT_TxRdyInt

#define   PRINT_USART_IRQ               AVR32_USART0_IRQ

#endif




CPU_INT32U  amtErrorPrintfCalls;			// amt calls to err_printf
CPU_INT32U  amtPrintErr;   // errors during print out, where err_printf should not be called


void OnPrintError()
{
  ++amtPrintErr;  // at least keep the number for debugging, maybe later an led could be switched on or so...
}


#ifdef use_USART_PRINT_PDCA


// PDCA channel options
 static const pdca_channel_options_t PDCA_OPTIONS =
 {
   .addr = NULL,  //(void *)ascii_anim1,              // memory address
   .pid = AVR32_PDCA_PID_USART_PRINT_TX,           // select peripheral - data are transmit on USART TX line.
   .size =  0 ,//sizeof(ascii_anim1),              // transfer counter
   .r_addr = NULL,                           // next memory address
   .r_size = 0,                              // next transfer counter
   .transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer
 };


char* errorResetStr = "Error reset\n";

/*
 * actually an error should never happen, but if it does, just try to signalise it somehow
 * and reset it if posssible
 * communicate transferComplete and reloadCounterZero to the application 

    pn, 9.7.12
    pdca interrupt was implemented specifically only for uart, while in a non-prototype design,
    pdca interrupt would be held generally for an arbitrary amount of pdca-channels working on a
    notification method per pdca-channel call design or a similar design, eg. an
    OSFlag registry per channel

*/

void APP_PDCA_ISR (void)
{
	INT32U isr;
	INT8U err;
	isr = pdca_get_interrupt_status(PDCA_CHANNEL_USART_PRINTF);
	if (isr & AVR32_PDCA_ISR_TERR_MASK)  {
		amtPdcaErrors ++;
//		pdca_load_channel(PDCA_CHANNEL_USART_PRINTF, errorResetStr, sizeof(errorResetStr));
        // errorResetStr may not be mixed into SerialMsgTaskQ! so let method handle errors
		// load method clears also error state, try communicate the error if possible
	}
	if (isr & AVR32_PDCA_ISR_RCZ_MASK) {
		OSFlagPost(pdcaStatus, ReloadCounterZero, OS_FLAG_SET, &err);
		pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
	}
	if (isr & AVR32_PDCA_IMR_TRC_MASK) {
		OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_SET, &err);
		 pdca_disable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
	}
}
//#warning is pdca isr needed for tcp/ip ?
// pn 21. Aug 12   seems that tcp-ip doesnt need pdca, seems to work all with macb interrupts 

void initPDCA(INT8U *perr)
{
    
    debugUartLoopCnt = debugUartPosFlag = 0;
    
	transferDataP = reloadDataP = NULL;
	  pdcaStatus = OSFlagCreate( 0, perr);  // will obviousely be set by interrupts, as seen when debugging
	  if (*perr == OS_ERR_NONE)  {
		  OSFlagNameSet(pdcaStatus, (INT8U *) "pdcaStatusFlags", perr);
	  }
	  if (*perr == OS_ERR_NONE)  {
		  if (BSP_INTC_IntReg(&APP_PDCA_ISR, App_IRQ_PDCA_USART_PRINT, 1) == BSP_INTC_ERR_NONE ) {
			*perr = OS_NO_ERR;
		} else {
			*perr = 0xFF;
		}
	  }
	  if (*perr ==  OS_ERR_NONE) {
		  pdca_init_channel(PDCA_CHANNEL_USART_PRINTF, &PDCA_OPTIONS);
		  pdca_enable_interrupt_transfer_error(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable(PDCA_CHANNEL_USART_PRINTF);
	  }
}

static  void  SerialQMethod (void *p_arg)
{
	INT8U err;
	serialPrintMem* sm;
//	volatile  avr32_usart_t  *usart;
	OS_FLAGS flags;

//	usart = &APP_USART;
     while (1) {
         ++ debugUartLoopCnt;
         debugUartPosFlag = 1;
    	 sm = (serialPrintMem *)OSQPend(serialPrintMsgTaskQ, 3167, &err);
         debugUartPosFlag = 2;
    	 if (err == OS_NO_ERR) {
    		 flags = OSFlagPend(pdcaStatus, ReloadCounterZero,
						 OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 1619, &err);
             debugUartPosFlag = 3;
    		 if (err == OS_NO_ERR)  {
    			 if (transferDataP != NULL) {
    				 err = OSMemPut(serialPrintMsgMem, transferDataP);
					 if (err != OS_NO_ERR) {
						 OnPrintError();
					 }
    			 }
    			 transferDataP = reloadDataP;
    			 reloadDataP = (void*) sm;
    			 pdca_reload_channel(PDCA_CHANNEL_USART_PRINTF,sm,strlen(sm->serialStr));
    			 OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_CLR, &err);  // no more valid
    			 pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
    			 pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
    			 // prevent loop on pdca interrupts that otherwise obivousely happens
    		 } else  {
                 // 16 secs no reload counter zero... something with pdca must be wrong
                 // reset everything
                 pdca_disable(PDCA_CHANNEL_USART_PRINTF);
                 if (transferDataP != NULL) {
                     err = OSMemPut(serialPrintMsgMem, transferDataP);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     transferDataP = NULL;
                 }
                 if (reloadDataP != NULL) {                                     
                     err = OSMemPut(serialPrintMsgMem, reloadDataP);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     reloadDataP = NULL;
                 }
                 if (sm != NULL) {                                     
                     err = OSMemPut(serialPrintMsgMem, sm);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     sm = NULL;
                 }
                 pdca_resetErrorFlag(PDCA_CHANNEL_USART_PRINTF);
                 pdca_enable(PDCA_CHANNEL_USART_PRINTF);
                    
                 OSFlagPost(pdcaStatus, ReloadCounterZero, OS_FLAG_SET, &err);  // pn 4. sept 12 probable needed to continue, still tobe tested
    			 OnPrintError();
#warning:  the timeout wait for reloadCounterZero in UartPrint.c is still tobe tested
    		 }
		 } else {  // normally will be timeout  .. see it optimistic :-)
             debugUartPosFlag = 4;
			 flags = OSFlagQuery(pdcaStatus, &err);
             debugUartPosFlag = 5;
                         if (err == OS_NO_ERR)  {
                             if (flags & TransferFinished ) {
                                     if (transferDataP) {
                                         err = OSMemPut(serialPrintMsgMem, transferDataP);
                                         if (err != OS_NO_ERR) {
                                                 OnPrintError();
                                         }
                                         transferDataP = NULL;
                                     }
                                     if (reloadDataP) {
                                         err = OSMemPut(serialPrintMsgMem, reloadDataP);
                                         if (err != OS_NO_ERR) {
                                                 OnPrintError();
                                         }
                                         reloadDataP = NULL;
                                     }
                                     OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_CLR, &err);
                                     // if (err != OS_NO_ERR) ...
                              } // transfer finished
                         } // flags queried ok .. else {  error ...}
		 }  // timeout
	 } // while 1
}



#endif


#ifdef use_USART_PRINT_TxRdyInt

void  APP_USARTRxTxISR (void)
{
    volatile  avr32_usart_t  *usart;
    INT8U err;

    usart = &PRINT_USART;
//    if (usart->CSR.rxrdy == 1) {
//        APP_RxISRHandler();
//    }
    if (usart->CSR.txrdy == 1) {
    	err = OSSemPost(SerialPrintQSem);
    	if (err != OS_NO_ERR){
//    		err_printf("couldnt signal sema in USART ISR\n");
    	}
    	BSP_USART_IntDis (PRINT_USART_COM, (1<< AVR32_USART_IER_TXRDY));
    	// PN 24. May 2012
    	// contrary to other AVR processors, it seems necessary to disable TXRDY interrupts
    	// manually, while other processors ATMega32/64 just trigger TXRDY interrupt once
    	// further testing will take place.
    }
}

/*
 * On a first attempt we try to send char after char to the uart for transmit
 * as soon a TX is ready, signalled by an interrupt.
 * Later we should try to use DMA/PDCA instead to transfer a whole buffer at once.
 */

static  void  SerialQMethod (void *p_arg)
{
	INT8U err;
	serialPrintMem* sm;
	INT8U strInd;
	volatile  avr32_usart_t  *usart;

	usart = &PRINT_USART;
     while (1) {
    	 sm = (serialPrintMem *)OSQPend(serialPrintMsgTaskQ, 1097, &err);
    	 if (err == OS_NO_ERR) {
    		 for (strInd = 0; strInd < (strlen(sm->serialStr)  ); ++ strInd){
    			 OSSemPend(SerialPrintQSem, 107, &err);
    			 if (err == OS_NO_ERR){
    				 OSSemSet(SerialPrintQSem, 0, &err);  // by initalization to value of 1, the count
												 // might get higher than one, what does not make sense
												 // and next char would be lost
    				 if (err  != OS_NO_ERR) {
    					 err_printf("Sem set 0 error, SerialQMeth");
    				 }
    				 usart->thr      = sm->serialStr[strInd];
    				 BSP_USART_IntEn (PRINT_USART_COM, (1<< AVR32_USART_IER_TXRDY));
    			    	// PN 24. May 2012
    			    	// contrary to other AVR processors, it seems necessary to disable TXRDY interrupts
    			    	// manually, while other processors ATMega32/64 just trigger TXRDY interrupt once
    			    	// further testing will take place.
    			 } else {
					 if ((err == OS_TIMEOUT)
							 && (!((usart->csr & AVR32_USART_CSR_TXRDY_MASK) == 0)))
					 {   // ready to send byte, but sema accidentally not set, recover situation
						 usart->thr      = sm->serialStr[strInd];
					 }
    			 }
    		 }

        	 err = OSMemPut(serialPrintMsgMem, (void *)sm);
        	 if (err != OS_NO_ERR) {
        	    OnPrintError();   // err_printf("mem put problem sec100 method\n");
        	 }
    	 } else {
    	  // no Message  received, has timed out
    	 }
    }
}

#endif

void init_err_printf()
{
	serialPrintOn = 0;
	amtErrorPrintfCalls = 0;
	amtPrintErr = 0;
	CPU_INT08U err_init_print = OS_NO_ERR;

	if (err_init_print == OS_NO_ERR) {
		BSP_USART_Init (PRINT_USART_COM, printUsartSpeed);  // start with something slow, later increase

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
		if (BSP_INTC_IntReg(&APP_USARTRxTxISR, PRINT_USART_IRQ, 2) == BSP_INTC_ERR_NONE ) {
			err_init_print = OS_NO_ERR;
		} else {
			err_init_print = 0xFF;
		}
		BSP_USART_IntEn (PRINT_USART_COM, (1<< AVR32_USART_IER_TXRDY));
	}
#endif 
        
    if (err_init_print == OS_NO_ERR )  {
		err_init_print = OSTaskCreateExt(SerialQMethod,                                    
									(void *)0,
									(OS_STK *)&SerialPrintQMethodStk[SerialpPrintQMethod_STK_SIZE - 1],
									SerialQ_TASK_PRIO,
									SerialQ_TASK_PRIO,
									(OS_STK *)&SerialPrintQMethodStk[0],
									SerialpPrintQMethod_STK_SIZE,
									(void *)0,
									OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	}
        
    OSTaskNameSet(SerialQ_TASK_PRIO, (INT8U *)"SerQ", &err_init_print);
	serialPrintOn = (err_init_print == OS_NO_ERR);
}
 #warning: check task stack overwriting by unknown source observed earlier when stack had to be grown tramendousely so that the software did not crash
 // it was then suspected that erroneous code caused this

//char bufferS [serialStrSz] ;

void info_printf( char *emsg, ...)
{
	va_list ap;
	CPU_INT08S  res;
	CPU_INT08U	err;
	serialPrintMem* sm;

	va_start(ap, emsg);

	if (serialPrintOn == 1) {
		sm = (serialPrintMem *) OSMemGet(serialPrintMsgMem, &err);
		if( sm != 0 ) {
			res = vsnprintf(sm->serialStr, serialPrintStrSz-1,  emsg, ap);
			if (res < 0) res = serialPrintStrSz;

			sm->serialStr[serialPrintStrSz-1] = 0;

//            attemptSendUSBString(sm->serialStr);     // temporarely commented out, pn 31 aug 12
            
			err = OSQPost(serialPrintMsgTaskQ, (void *)sm);
			if ( err != OS_NO_ERR) {
			//	do something but dont loop--->> err_printf("Q post err tickHook\n");
				OnPrintError();
			}

		} else	{  //OSMemGet
			// do somethingg ???
			OnPrintError();
		}
	} else {   // serialOn
		// might turn on an led or so
		OnPrintError();
	}
	va_end(ap);
	//	printf(emsg, ap);
}

// just for usage with short strings, otherwise sizes of buffers need to be increased
void  err_printf ( char *emsg, ...)
{
	va_list ap;

	va_start(ap, emsg);
	++ amtErrorPrintfCalls;
	info_printf(emsg, ap);
	va_end(ap);
}


/*
 * end of serial printing stuff
 

 
 
 */



