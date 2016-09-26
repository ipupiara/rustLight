/* timecli.c */
/* Gets the current time from a UDP server */
/* Last modified: September 23, 2005 */
/* http://www.gomorgan89.com */
/* Link with library file wsock32.lib */


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>

#pragma comment(lib,"ws2_32.lib")

#define BufferSIZE 500
#define clientAddress    "168.192.0.151"
#define clientPort   "1234"


void usage(void);
SOCKET rustlightSocket;	
sockaddr_in server;
sockaddr_in client;


	void  err_printf(char *emsg, ...)
	{
		va_list ap;

		va_start(ap, emsg);
		fprintf(stderr, emsg, ap);
		va_end(ap);
	}

	void exitRustlightTcpIp()
	{
		closesocket(rustlightSocket);
		err_printf( "socket closed\n");
		WSACleanup();
		err_printf( "aft wsa cleanup\n");
	}

	int initRustlightTcpIp(char* cliAddress, char* cliPort)
	{
		int res = 0;
		int a1, a2, a3, a4;
		hostent *hostP;
		char host_name[256];
		unsigned int port_number;
		WSADATA rustlightWsDATA;
		DWORD  sotimeout = 0x00000400;
		


		if (sscanf_s((const char *)cliAddress, "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
		{
			err_printf( "address error.\n");
			++ res;
		}
		if (sscanf_s((const char *)cliPort, "%u", &port_number) != 1)
		{
			err_printf( "portnumber error.\n");
			++ res;
		}

		err_printf( "bef wsa startup\n");
		if (WSAStartup(0x0101, &rustlightWsDATA) != 0)
		{
			err_printf( "Could not open Windows connection.\n");
			++ res;
		}
		err_printf( "WSA Started up\n");


		rustlightSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (rustlightSocket == INVALID_SOCKET)
		{
			err_printf( "Could not create socket.\n");
			WSACleanup();
			++ res;
		}
		err_printf( "created  socket\n");

		memset((void *)&server, '\0', sizeof(struct sockaddr_in));

		server.sin_family = AF_INET;
		server.sin_port = htons(port_number);

		server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

		memset((void *)&client, '\0', sizeof(struct sockaddr_in));

		client.sin_family = AF_INET;
		client.sin_port = htons(0);

		gethostname(host_name, sizeof(host_name));
		hostP = gethostbyname(host_name);

		if (hostP == NULL)
		{
			err_printf( "Could not get host name.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			++ res;
		}

		err_printf( "host with name %s got addr %u.%u.%u.%u\n", host_name,
			(unsigned char)hostP->h_addr_list[0][0], (unsigned char)hostP->h_addr_list[0][1],
			(unsigned char)hostP->h_addr_list[0][2], (unsigned char)hostP->h_addr_list[0][3]);

		client.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)hostP->h_addr_list[0][0];
		client.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)hostP->h_addr_list[0][1];
		client.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)hostP->h_addr_list[0][2];
		client.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)hostP->h_addr_list[0][3];

		if (bind(rustlightSocket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1)
		{
			err_printf( "Cannot bind address to socket.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			++ res;
		}

		err_printf( "aft socket bind to ip\n");

		res = setsockopt(rustlightSocket, IPPROTO_UDP, SO_SNDTIMEO, (char*)&sotimeout, sizeof(sotimeout));
		if (res != 0) {
			err_printf( "Error setting send timeout data.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			++ res;
		}

		res = setsockopt(rustlightSocket, IPPROTO_UDP, SO_RCVTIMEO, (char*)&sotimeout, sizeof(sotimeout));
		if (res != 0) {
			err_printf( "Error setting receive timeout data.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			++ res;
		}

		return res;
	}
									


	int communicateMsg(UINT32 msg)
	{
		int cnt;
		char send_buffer[BufferSIZE];
		char receive_buffer[BufferSIZE];
		int server_length;
		int res;

		memset(send_buffer, 0, sizeof(send_buffer));

		cnt = 3;
/*		while (argc > cnt) {
			strncat(send_buffer, (char*)argv[cnt], sizeof(send_buffer));
			strncat(send_buffer, " ", sizeof(send_buffer));
			cnt = cnt + 1;
		}*/
		
		snprintf(send_buffer, sizeof(send_buffer), "%X\n", msg);

		err_printf( "sending message: %s \n", send_buffer);


		server_length = sizeof(struct sockaddr_in);
		if ((res=sendto(rustlightSocket, send_buffer, (int)strlen(send_buffer) + 1, 0, (struct sockaddr *)&server, server_length)) == -1)
		{
			err_printf( "Error transmitting data.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			return res;
		}
		err_printf( "message sent, now receiving (without timeout).... \n");

		memset(receive_buffer, 0, sizeof(receive_buffer));

		if ((res = recvfrom(rustlightSocket, (char *)&receive_buffer, (int)sizeof(receive_buffer), 0, (struct sockaddr *)&server, &server_length)) < 0)
		{
			err_printf( "Error receiving data.\n");
			closesocket(rustlightSocket);
			WSACleanup();
			return res;
		}
		err_printf( "socket received\n");
		err_printf( "received: %s\n", receive_buffer);
		return res;
	}

int main(int argc, char **argv)
{
	int res = 0;
	
	res = initRustlightTcpIp(clientAddress, clientPort);

	if (res == 0) {
		res = communicateMsg(0x010202a5);
	}

	exitRustlightTcpIp();
	err_printf("aft wsa cleanup\n");
	return(0);
}
