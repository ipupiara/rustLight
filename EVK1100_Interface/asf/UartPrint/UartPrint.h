/*
 * UartPrint.h
 *
 *  Created on: 30.05.2012
 *      Author: duenda
 */

#ifndef UARTPRINT_H_
#define UARTPRINT_H_

#define use_USART_PRINT_PDCA
#ifndef use_USART_PRINT_PDCA
#define use_USART_PRINT_TxRdyInt
#endif


void init_err_printf();
void  err_printf ( char *emsg, ...);
void info_printf( char *emsg, ...);

extern CPU_INT08U  serialPrintOn;


#define printUsartSpeed   57600


#endif /* UARTPRINT_H_ */
