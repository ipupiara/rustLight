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
#include <stdlib.h>

#include "includes.h"
#include "UartHygrosens.h"
#include "UartPrint.h"

/*
 *
 *************   Hygrosense Receiver Stuff   ***********
 */

void  setHygrosensPinAsOutputWithValue (CPU_INT16U pin, INT8U val)
{
	volatile  avr32_gpio_port_t  *gpio_port;

	gpio_port        = &AVR32_GPIO.port[pin >> 5];
	INT8U pinOfs     = pin & 0x1F;
	INT32U mask     = (1<< pinOfs);
	
	gpio_port->oders = mask; // The GPIO output driver is enabled for that pin.
	gpio_port->gpers = mask;
	gpio_port->odmerc = mask;
	
	if (val == 0) {
		gpio_port->ovrc  = mask;
		} else {
		gpio_port->ovrs  = mask;
	}
	
}


#define SerialHygrosensMethod_STK_SIZE   unique_STK_SIZE

OS_STK  SerialHygrosensMethodStk[SerialHygrosensMethod_STK_SIZE];




#define   HYGROSENS_USART_COM               1
#define   HYGROSENS_USART                   AVR32_USART1;

#define   HYGROSENS_USART_IRQ               AVR32_USART1_IRQ
#define	  hygrosensUsartSpeed   4800


//#define   HYGROSENS_USART_COM               0
//#define   HYGROSENS_USART                   AVR32_USART0;
//
//#define   HYGROSENS_USART_IRQ               AVR32_USART0_IRQ
//#define	  hygrosensUsartSpeed   4800



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

float latestTemperature;
float latestHumidity;

#define hygrosensMemorySz    5
#define hygrosensExpectedStrlen 66
#define hygrosensStrSz    68    // pn 9.June2012 attention: a value of e.g. 123 will give an unhandled exception.... ????
#warning: "exception tobe tested ???"

typedef CPU_INT08U hygrosensStringType [hygrosensStrSz];

typedef struct hygrosensStringMemory {
	hygrosensStringType serialStr;  // ATTENTION serialStr must be at first place, so that &(serialMem) == &(serialStr)
} hygrosensStringMemory;

hygrosensStringType* rxBuffer;

OS_MEM *hygrosensMsgsMem;
OS_EVENT*  hygrosensTaskMsgQ;
hygrosensStringMemory hygrosensMem[hygrosensMemorySz];
void* hygrosensTaskMsgPtrs[hygrosensMemorySz];

INT32U hygrosensThreadMethodLoops;


void OnHygrosensError(char * errSt,CPU_INT08U err)
{
	err_printf(errSt);
	err_printf("  ->error returned %i",err);
}

void releaseBuffer(hygrosensStringType* memo)
{
	CPU_INT08U	err;
	err = OSMemPut( hygrosensMsgsMem,(void *) memo);
	if ( err != OS_NO_ERR) {
		//	do something but dont loop--->> err_printf("Q post err tickHook\n");
		OnHygrosensError("releaseBuffer error",err);
	}
}

void envoyBuffer(hygrosensStringType* memo)
{
	CPU_INT08U	err;
	err = OSQPost(hygrosensTaskMsgQ, (void *)memo);
	if ( err != OS_NO_ERR) {
		//	do something but dont loop--->> err_printf("Q post err tickHook\n");
		OnHygrosensError("envoyBuffer error",err);
	}
}

hygrosensStringType* getBuffer(CPU_INT08U* err)
{	
	hygrosensStringType* res = NULL;
	res = (hygrosensStringType *) OSMemGet(hygrosensMsgsMem, err);
	if ( err != OS_NO_ERR) {
		//	do something but dont loop--->> err_printf("Q post err tickHook\n");
		OnHygrosensError("getBuffer error",*err);
		res = NULL;  // just tobe sure
	}
	return res;
}


void addCharToBuffer(CPU_INT08U rxCh)
{
	CPU_INT08U err;
	if (rxCh == startChar)  {
		amtCharRcvd = 0;
		if (rxBuffer == NULL) {
			rxBuffer = getBuffer(&err); 
		}  
		if (rxBuffer != NULL) {
			*rxBuffer[amtCharRcvd] = rxCh;
			rxState = rxReceiving;
		}
	} else
	if (rxState == rxReceiving)  {
		++ amtCharRcvd;
		if (amtCharRcvd < hygrosensStrSz) {  //  prevent string overflow
			*rxBuffer [amtCharRcvd] = rxCh;
		}
		if ((rxCh == stopChar) && (amtCharRcvd == hygrosensExpectedStrlen) ) {   // no  chars lost, valid message checked
			rxState = rxReceived;
			envoyBuffer(rxBuffer);
			rxBuffer = NULL;
		}
	}
}





char * reallyWorkingStrstr(const char *inStr, const char *subStr)
{
	char firstSubChar;
	size_t lenSubStr;
	firstSubChar = *subStr++;
	if (!firstSubChar)
	return (char *) inStr;	// Trivial empty string case

	lenSubStr = strlen(subStr);
	size_t lenInStr = strlen(inStr);
	CPU_INT16U curPos = 0;
	do {
		char currentInChar;

		do {
			currentInChar = *inStr++;
			++ curPos;
			if ( ((curPos + lenSubStr) > lenInStr) ||  (!currentInChar))
			return (char *) 0;
		} while (currentInChar != firstSubChar);
	} while (strncmp(inStr, subStr, lenSubStr) != 0);
	#warning: accesses memory outside of string ( -> access violation) ->  above fix for this tobe tested (second condition is actually redundant)
	return (char *) (inStr - 1);
}


#define hygrosensPin   AVR32_PIN_PX31

void setHeating(INT8U val)
{
	setHygrosensPinAsOutputWithValue(AVR32_PIN_PX31,  val);
//	setPinAsOutputWithValue (AVR32_PIN_PX31,  val);	
}



void switchHeating(CPU_INT08U heatingNeedsOn)
{
	if (heatingNeedsOn > 0) {
		
		setHeating(1);
//		heatingPort |=  0xff;  // (1 < heatingPin);
//		heatingIsOn= 1;
	}  else   {
		setHeating(0);
//		heatingPort &= 0x00;   //~(1 < heatingPin);
//		heatingIsOn = 0;
	}
}

void controlTemperature(float* temp)
{
	if (*temp < HeatingLowerLimit)  {
		switchHeating(1);
	}
	if (*temp > HeatingUpperLimit)  {
		switchHeating(0);
	}
}

void onDataReceivedUart1(char * rcvMsg)        
{
	char tempS [5];
	char hydS [5];
	
	char * v01Pos = (char *) 0;
	char * v02Pos = (char *) 0;

	memset (tempS,0,sizeof(tempS));
	memset (hydS,0,sizeof(hydS));
	     	
	v01Pos = reallyWorkingStrstr((char* )&rxBuffer,"V01");
	v02Pos = reallyWorkingStrstr((char*)&rxBuffer,"V02");
				
	strncpy(tempS,v01Pos+3,4);
	strncpy(hydS,v02Pos+3,4);
		
	if ((v01Pos != NULL) && (v02Pos != NULL ))  {
		char* endP = tempS+3;
		float temp = strtoul(tempS ,&endP,0x10) ;
		temp = temp / 100;
		endP = hydS + 3;
		float hyd = strtoul(hydS ,&endP,0x10) ;
		hyd = hyd / 200;
		
		latestTemperature = temp;
		latestHumidity = hyd;
		
		controlTemperature(&temp);
	}
}


void  hygrosensRxISR (void)
{
	volatile  avr32_usart_t  *usart;
	CPU_INT08U rxCh;

	usart = &HYGROSENS_USART;
	if (usart->CSR.rxrdy == 1) {
	    rxCh =  (CPU_INT08U) usart->RHR.rxchr;
		addCharToBuffer(rxCh);
	}
}


static  void  SerialHygrosensThreadMethod (void *p_arg)
{
	INT8U err;
	hygrosensStringType* msgStr;

	while (1) {
		msgStr = (hygrosensStringType *)OSQPend(hygrosensTaskMsgQ, 6823, &err);
		if (err == OS_NO_ERR) {
			onDataReceivedUart1((char*) msgStr);
			releaseBuffer(msgStr);
		}
		++hygrosensThreadMethodLoops;
		info_printf("hygrosenseLoops %5i",hygrosensThreadMethodLoops);
	}
}


CPU_INT08U init_HygrosenseReceiver()
{
	serialHygrosensOn = 0;
	rxBuffer = NULL;
	latestHumidity = 0.0;
	latestTemperature = 0.0;
	CPU_INT08U  err_init_hygrosens = OS_NO_ERR;
	hygrosensThreadMethodLoops = 0;
	
	if (err_init_hygrosens == OS_NO_ERR) {
		hygrosensMsgsMem = OSMemCreate(&hygrosensMem[0], hygrosensMemorySz, sizeof(hygrosensStringMemory), &err_init_hygrosens);
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		OSMemNameSet(hygrosensMsgsMem, (INT8U*)"hygrosensMsgMem", &err_init_hygrosens);
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		hygrosensTaskMsgQ = OSQCreate(&hygrosensTaskMsgPtrs[0], hygrosensMemorySz);
		if (! hygrosensTaskMsgQ) err_init_hygrosens = 0xFF;
	}
	

	
	if (err_init_hygrosens == OS_NO_ERR) {
		BSP_USART_Init (HYGROSENS_USART_COM, hygrosensUsartSpeed);  
	}
	if (err_init_hygrosens == OS_NO_ERR) {
		if (BSP_INTC_IntReg(&hygrosensRxISR, HYGROSENS_USART_IRQ, 1) == BSP_INTC_ERR_NONE ) {
			err_init_hygrosens = OS_NO_ERR;
			} else {
			err_init_hygrosens = 0xFF;
		}
	}
	serialHygrosensOn = (err_init_hygrosens == OS_NO_ERR);
	
	return err_init_hygrosens;
}


CPU_INT08U start_HygrosenseReceiver()
{
	CPU_INT08U  err_init_hygrosens = OS_NO_ERR;
	
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
	
	if (err_init_hygrosens == OS_NO_ERR )  {
		BSP_USART_IntEn (HYGROSENS_USART_COM, (1<< AVR32_USART_IER_RXRDY));
	}
}