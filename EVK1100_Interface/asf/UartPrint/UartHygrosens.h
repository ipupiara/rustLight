/*
 * UartHygrosens.h
 *
 * Created: 07.01.2020 11:26:01
 *  Author: mira
 */ 


#ifndef UARTHYGROSENS_H_
#define UARTHYGROSENS_H_

#define HeatingLowerLimit	27.0
#define HeatingUpperLimit	27.5

CPU_INT08U  serialHygrosensOn;


CPU_INT08U init_HygrosenseReceiver();


#endif /* UARTHYGROSENS_H_ */