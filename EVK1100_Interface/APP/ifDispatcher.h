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
} dispatchMsg;

OS_MEM * dispatchMsgMem;
OS_EVENT*  dispatchMsgQ;

void initIfDipatcher();

void startIfDispatcher();



#endif /* IFDISPATCHER_H_ */