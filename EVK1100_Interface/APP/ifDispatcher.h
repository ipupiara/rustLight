/*
 * ifDispatcher.h
 *
 * Created: 20.09.2016 15:52:52
 *  Author: mira
 */ 


#ifndef IFDISPATCHER_H_
#define IFDISPATCHER_H_


typedef struct dispatchMsg {
	INT32U dispatchData;
	char  dummyArray[10];
} dispatchMsg;

OS_MEM * dispatchMsgMem;
OS_EVENT*  dispatchMsgQ;

void initIfDipatcher();

void startIfDispatcher();

void  setPinAsOutputWithValue (CPU_INT16U pin, INT8U val);  // made public for use in "friend" UartPrint.c

#endif /* IFDISPATCHER_H_ */