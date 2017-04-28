/*
 * tcpipApp.c
 *
 * Created: 19.09.2016 23:48:27
 *  Author: mira
 */ 


#include  <includes.h>

#include <stdlib.h>
//#include <malloc.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

//#include "sdramc.h"
#include "tcpipApp.h"
#include "UartPrint.h"
#include "ifDispatcher.h"

NET_IP_ADDR   ownIp;
NET_IP_ADDR   msk;
NET_IP_ADDR   gateway;
NET_ERR       err;

OS_STK  tcp_ip_Thread_MethodStk[tcp_ip_Thread_Method_STK_SIZE];

#define tcp_ip_Buffer_Size   256

char   tcp_ip_RecvBuffer [tcp_ip_Buffer_Size];
char   tcp_ip_SendBuffer [tcp_ip_Buffer_Size];

void AppInit_TCPIP (void)
{
	#if EMAC_CFG_MAC_ADDR_SEL == EMAC_MAC_ADDR_SEL_CFG
	NetIF_MAC_Addr[0] = 0x00;
	NetIF_MAC_Addr[1] = 0x50;
	NetIF_MAC_Addr[2] = 0xC2;
	NetIF_MAC_Addr[3] = 0x25;
	NetIF_MAC_Addr[4] = 0x60;
	NetIF_MAC_Addr[5] = 0x01;
	#endif

	#if uC_TTCP_MODULE > 0
	BSP_USART_Init(TTCP_COMM_SEL, 38400);                           
	#endif

	err = Net_Init();                                        
	
	if (err != NET_ERR_NONE)   
	{
		info_printf("AppInit_TCPIP 1 net error %i\n",err);
	}

	ownIp      = NetASCII_Str_to_IP((CPU_CHAR *) ownIPAddress,  &err);
	msk     = NetASCII_Str_to_IP((CPU_CHAR *)"255.255.0.0", &err);
//	gateway = NetASCII_Str_to_IP((CPU_CHAR *)"192.168.1.1",   &err);

	err     = NetIP_CfgAddrThisHost(ownIp, msk);
	if (err != NET_ERR_NONE)
	{
		info_printf("AppInit_TCPIP 2 net error %i\n",err);
	}
	err     = NetIP_CfgAddrDfltGateway(gateway);
	if (err != NET_ERR_NONE)
	{
		info_printf("AppInit_TCPIP 3 net error %i\n",err);
	}

	#if uC_TTCP_MODULE > 0
	TTCP_Init();                                               
	#endif
	info_printf("tcp/ip network initialized ...\n");
}



static  void  tcp_ip_Thread_Method (void *p_arg)
{
	NET_SOCK_ID sock;
	NET_SOCK_ADDR_IP server_sock_addr_ip;
	NET_SOCK_ADDR_LEN server_sock_addr_ip_size;
	NET_SOCK_ADDR_IP client_sock_addr_ip;
	NET_SOCK_ADDR_LEN client_sock_addr_ip_size;
	NET_SOCK_RTN_CODE rx_size, tx_size;
	CPU_BOOLEAN attempt_rx;
	NET_ERR netErr;
	CPU_INT08U	osErr;
	dispatchMsg* dmPtr;
	INT8U continueInt = 1;

	sock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
					NET_SOCK_TYPE_DATAGRAM,
					NET_SOCK_PROTOCOL_UDP,
					&netErr);
	if (netErr != NET_SOCK_ERR_NONE) {
		err_printf("NetSock Open failed\n");
	}
	
	server_sock_addr_ip_size = sizeof(server_sock_addr_ip);
	Mem_Clr((void *)&server_sock_addr_ip,   (CPU_SIZE_T) server_sock_addr_ip_size);
	
	server_sock_addr_ip.Family = NET_SOCK_ADDR_FAMILY_IP_V4;
	server_sock_addr_ip.Addr = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILD_CARD);
	server_sock_addr_ip.Port = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);
#warning: 	" tobe debugged  in method  tcp_ip_Thread_Method    ip.addr in server_sock_add_ip"
	NetSock_Bind((NET_SOCK_ID ) sock,
				(NET_SOCK_ADDR *)&server_sock_addr_ip,
				(NET_SOCK_ADDR_LEN) NET_SOCK_ADDR_SIZE,
				(NET_ERR *)&netErr);
	if (netErr != NET_SOCK_ERR_NONE) {
		NetSock_Close(sock, &netErr);
		err_printf("Net sock Bind failed\n");
	}
	
	while (continueInt)  {
		CPU_INT32U msgUint = 0;
		do {
			client_sock_addr_ip_size = sizeof(client_sock_addr_ip);
			memset(tcp_ip_RecvBuffer,0,sizeof(tcp_ip_RecvBuffer));
			
			rx_size = NetSock_RxDataFrom((NET_SOCK_ID ) sock,
								(void *) tcp_ip_RecvBuffer,
								(CPU_INT16S ) sizeof(tcp_ip_RecvBuffer),
								(CPU_INT16S ) NET_SOCK_FLAG_NONE,
								(NET_SOCK_ADDR *)&client_sock_addr_ip,
								(NET_SOCK_ADDR_LEN *)&client_sock_addr_ip_size,
								(void *) 0,
								(CPU_INT08U ) 0,
								(CPU_INT08U *) 0,
								(NET_ERR *)&netErr);
			switch (netErr) {
				case NET_SOCK_ERR_NONE:
					attempt_rx = DEF_NO;
					break;
				case NET_SOCK_ERR_RX_Q_EMPTY:
				case NET_OS_ERR_LOCK:
					attempt_rx = DEF_YES;
					break;
				default:
					attempt_rx = DEF_NO;
					break;
			}
		} while (attempt_rx == DEF_YES);
//		msgUint = (CPU_INT32U) tcp_ip_RecvBuffer;
//		msgUint = atol(tcp_ip_RecvBuffer);
		sscanf(tcp_ip_RecvBuffer,"%x",&msgUint);
		info_printf("Net received %i  msgUint %X  bytes : [%s]\n",rx_size,msgUint,tcp_ip_RecvBuffer);
		dmPtr = OSMemGet(dispatchMsgMem,&osErr);
		if (osErr != OS_NO_ERR) {
			err_printf("error get memory in method tcp_ip_Thread_Method, err = %d\n ",netErr);
		}  else {
			dmPtr->dispatchData = msgUint;
			osErr = OSQPost(dispatchMsgQ,dmPtr);
			if (osErr != OS_NO_ERR) {
				err_printf("error OSQPost in method tcp_ip_Thread_Method, err = %d\n ",netErr);
			}		
		}
		
		//snprintf((char*)&tcp_ip_SendBuffer,sizeof(tcp_ip_SendBuffer)," msg received");
		//
		//tx_size = NetSock_TxDataTo((NET_SOCK_ID ) sock,
					//(void *) tcp_ip_SendBuffer,
					//(CPU_INT16S ) strlen((char*) tcp_ip_SendBuffer)+ 1,
					//(CPU_INT16S ) NET_SOCK_FLAG_NONE,
					//(NET_SOCK_ADDR *)&client_sock_addr_ip,
					//(NET_SOCK_ADDR_LEN) client_sock_addr_ip_size,
					//(NET_ERR *)&err);
		//
		//info_printf("net sent (ret error code %i) %i bytes : &s\n",err,tx_size, tcp_ip_SendBuffer);
	}
	NetSock_Close(sock, &netErr);
	if (netErr != NET_SOCK_ERR_NONE) {
		err_printf("Net Sock Close error\n");
	}
}

void startTcpipThread()
{
	INT8U retVal = 0;
	retVal = OSTaskCreateExt(tcp_ip_Thread_Method,
					(void *)0,
					(OS_STK *)&tcp_ip_Thread_MethodStk[tcp_ip_Thread_Method_STK_SIZE - 1],
					tcp_ip_Thread_TASK_PRIO,
					tcp_ip_Thread_TASK_PRIO,
					(OS_STK *)&tcp_ip_Thread_MethodStk[0],
					tcp_ip_Thread_Method_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	
	if (retVal !=  OS_NO_ERR  ) {
		err_printf("error create sec100 tcp_ip Thread\n");
	}  else {
		info_printf("tcp/ip started as %s:%d\n",ownIPAddress,UDP_SERVER_PORT);
	}

	#if (OS_TASK_NAME_SIZE > 10)
	OSTaskNameSet(tcp_ip_Thread_TASK_PRIO, (CPU_CHAR *)"tcp_ip", &err);
	#endif
}