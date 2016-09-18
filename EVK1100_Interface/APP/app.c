/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                      ATMEL  AVR32 UC3 Sample code
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : APP.C
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

#include  <includes.h>

#include <stdlib.h>
//#include <malloc.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

/*
**************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/


#if uC_TCPIP_MODULE > 0
NET_IP_ADDR   ip;
NET_IP_ADDR   msk;
NET_IP_ADDR   gateway;
NET_ERR       err;
#endif


OS_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];
OS_STK  tcp_ip_Thread_MethodStk[tcp_ip_Thread_Method_STK_SIZE];

/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/


#if uC_TCPIP_MODULE > 0
static  void  AppInit_TCPIP(void);
#endif

static  void  AppTaskStart(void *p_arg);
static  void  AppTaskCreate(void);

/*
*    globals and enums
*/

/*
*   tcp-ip_Thread Variables and Types
*/


#if uC_TCPIP_MODULE > 0
#define tcp_ip_Buffer_Size   256

char   tcp_ip_RecvBuffer [tcp_ip_Buffer_Size];
char   tcp_ip_SendBuffer [tcp_ip_Buffer_Size];
#endif



/*
*    tcp-ip Methods
*/

#if uC_TCPIP_MODULE > 0

#define UDP_SERVER_PORT 10001

static  void  tcp_ip_Thread_Method (void *p_arg)
{
	NET_SOCK_ID sock;
	NET_SOCK_ADDR_IP server_sock_addr_ip;
	NET_SOCK_ADDR_LEN server_sock_addr_ip_size;
	NET_SOCK_ADDR_IP client_sock_addr_ip;
	NET_SOCK_ADDR_LEN client_sock_addr_ip_size;
	NET_SOCK_RTN_CODE rx_size, tx_size;
	CPU_BOOLEAN attempt_rx;
	NET_ERR err;
	CPU_INT08U secB;
	CPU_INT16S minB;
	CPU_INT08U continueInt;
	
	continueInt = 1;
	sock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
	NET_SOCK_TYPE_DATAGRAM,
	NET_SOCK_PROTOCOL_UDP,
	&err);
	if (err != NET_SOCK_ERR_NONE) {
		err_printf("NetSock Open failed\n");
	}
	
	server_sock_addr_ip_size = sizeof(server_sock_addr_ip);
	Mem_Clr((void *)&server_sock_addr_ip,   (CPU_SIZE_T) server_sock_addr_ip_size);
	
	server_sock_addr_ip.Family = NET_SOCK_ADDR_FAMILY_IP_V4;
	server_sock_addr_ip.Addr = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILD_CARD);
	server_sock_addr_ip.Port = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);
	
	NetSock_Bind((NET_SOCK_ID ) sock,
	(NET_SOCK_ADDR *)&server_sock_addr_ip,
	(NET_SOCK_ADDR_LEN) NET_SOCK_ADDR_SIZE,
	(NET_ERR *)&err);
	if (err != NET_SOCK_ERR_NONE) {
		NetSock_Close(sock, &err);
		err_printf("Net sock Bind failed\n");
	}
	
	while (continueInt)  {
		do {
			client_sock_addr_ip_size = sizeof(client_sock_addr_ip);
			
			rx_size = NetSock_RxDataFrom((NET_SOCK_ID ) sock,
			(void *) tcp_ip_RecvBuffer,
			(CPU_INT16S ) sizeof(tcp_ip_RecvBuffer),
			(CPU_INT16S ) NET_SOCK_FLAG_NONE,
			(NET_SOCK_ADDR *)&client_sock_addr_ip,
			(NET_SOCK_ADDR_LEN *)&client_sock_addr_ip_size,
			(void *) 0,
			(CPU_INT08U ) 0,
			(CPU_INT08U *) 0,
			(NET_ERR *)&err);
			switch (err) {
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
		info_printf("Net received %i  bytes : %s\n",rx_size,tcp_ip_RecvBuffer);
		
		secB = 0; //currentSec100Info.sec;
		minB = 0; //currentSec100Info.min;
		//   not threadsave, but if ever an accident happens, it is of no consequence in this case.... murphis law...  :-)
		
		snprintf((char*)&tcp_ip_SendBuffer,sizeof(tcp_ip_SendBuffer),"dcfAlways Server at min %i sec %i - still prototye",minB,secB);
		
		tx_size = NetSock_TxDataTo((NET_SOCK_ID ) sock,
		(void *) tcp_ip_SendBuffer,
		(CPU_INT16S ) strlen((char*) tcp_ip_SendBuffer)+ 1,
		(CPU_INT16S ) NET_SOCK_FLAG_NONE,
		(NET_SOCK_ADDR *)&client_sock_addr_ip,
		(NET_SOCK_ADDR_LEN) client_sock_addr_ip_size,
		(NET_ERR *)&err);
		
		info_printf("net sent (ret error code %i) %i bytes : &s\n",err,tx_size, tcp_ip_SendBuffer);
	}
	NetSock_Close(sock, &err);
	if (err != NET_SOCK_ERR_NONE) {
		err_printf("Net Sock Close error\n");
	}
}
#endif



#if uC_TCPIP_MODULE > 0

static void AppInit_TCPIP (void)
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
	BSP_USART_Init(TTCP_COMM_SEL, 38400);                               /* Initialize the TTCP UART for command entry               */
	#endif

	err = Net_Init();                                                   /* Initialize uC/TCP-IP                                     */

	ip      = NetASCII_Str_to_IP((CPU_CHAR *)"192.168.1.155",  &err);
	msk     = NetASCII_Str_to_IP((CPU_CHAR *)"255.255.255.0", &err);
	gateway = NetASCII_Str_to_IP((CPU_CHAR *)"192.168.1.1",   &err);

	err     = NetIP_CfgAddrThisHost(ip, msk);
	err     = NetIP_CfgAddrDfltGateway(gateway);

	#if uC_TTCP_MODULE > 0
	TTCP_Init();                                                        /* Initialize uC/TTCP                                       */
	#endif
	info_printf("tcp/ip network initialized ...\n");
}
#endif






/*$PAGE*/
/*
**************************************************************************************************************
*                                                MAIN
**************************************************************************************************************
*/

int  main (void)
{
#if (OS_TASK_NAME_SIZE > 7) && (OS_TASK_STAT_EN > 0)
    CPU_INT08U  err;
#endif


    CPU_IntDis();                                                       /* Disable all interrupts until we are ready to accept them */

    OSInit();                                                           /* Initialize "uC/OS-II, The Real-Time Kernel"              */

    OSTaskCreateExt(AppTaskStart,                                       /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppTaskStartStk[0],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE > 7) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_CHAR *)"Startup", &err);
#endif

    OSStart();                                                          /* Start multitasking (i.e. give control to uC/OS-II)       */

    return (0);                                                         /* Avoid warnings for GCC std prototype (int main (void))   */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description: This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments  : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT08U  i;


    (void)p_arg;

    BSP_Init();                                                         /* Initialize BSP functions                                */

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                       /* Determine CPU capacity                                  */
#endif


#if uC_TCPIP_MODULE > 0
AppInit_TCPIP();                                                    /* Initialize uC/TCP-IP and associated appliations          */
#endif


    AppTaskCreate();                                                    /* Create application tasks                                */

    while (1) {                                                         /* Task body, always written as an infinite loop.          */
        for (i = 1; i <= 6; i++) {
            LED_On(i);
            OSTimeDlyHMSM(0, 0, 1, 0);
            LED_Off(i);
        }
        for (i = 4; i <= 7; i++) {
            LED_On(9 - i);
            OSTimeDlyHMSM(0, 0, 1, 0);
            LED_Off(9 - i);
        }
    }
}


/*
**************************************************************************************************************
*                                        CREATE APPLICATION TASKS
*
* Description: This function creates the application tasks.
*
* Arguments  : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
**************************************************************************************************************
*/

// bad luck the following methods need (more or less) to be implemented as well  (or some own defines
// needed to be implemented into
// the framework code, what is not so nice for future possible upgrades

// moreover it is not possible to compile an application with OS_TASK_SW_HOOK_EN set to 0
// because this will give a build error in any case what shows that the
// framework code itself is not so completely finished in its development.
// so it seems the easiest just to implement these method, since probable they get optimized out anyhow

void          App_TaskCreateHook      (OS_TCB          *ptcb){}
void          App_TaskDelHook         (OS_TCB          *ptcb){}
void          App_TaskIdleHook        (void){}
void          App_TaskStatHook        (void){}
void          App_TCBInitHook         (OS_TCB          *ptcb){}
void          App_TaskSwHook          (void){}

void          App_TimeTickHook        (void)   {}
	

static  void  AppTaskCreate (void)
{  
	
	
#if uC_TCPIP_MODULE > 0
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
	}

	#if (OS_TASK_NAME_SIZE > 10)
	OSTaskNameSet(tcp_ip_Thread_TASK_PRIO, (CPU_CHAR *)"tcp_ip", &err);
	#endif
#endif
	
}

















