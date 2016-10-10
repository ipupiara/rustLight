
#ifndef rustlightUdpClient_h
#define rustlightUdpClient_h

#define BufferSIZE 100
#define defaultClientAddress    "168.192.0.151"
#define defaultClientPort   "1234"

typedef void (*printFunction)(const char *emsg, ...);

class RustlightUdpClient
{
public:
	RustlightUdpClient();
	RustlightUdpClient(printFunction i_printf);
	virtual ~RustlightUdpClient();
	int initRustlightTcpIp(char* cliAddress, char* cliPort);

	void exitRustlightTcpIp();

	int communicateMsg(UINT32 msg);

protected:
	SOCKET rustlightSocket;
	sockaddr_in server;
	sockaddr_in client;
	printFunction info_printf;
};

#endif