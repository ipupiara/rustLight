
#ifndef RustlightUpdClient_h
#define RustlightUpdClient_h

#define BufferSIZE 100
#define defaultClientAddress    "168.192.0.151"
#define defaultClientPort   "1234"

typedef void (*printFunction)(const char *emsg, ...);

class RustlightUpdClient
{
public:
	RustlightUpdClient();
	RustlightUpdClient(printFunction i_printf);
	virtual ~RustlightUpdClient();
	int initRustlightTcpIp(char* cliAddress, char* cliPort);

	void exitRustlightTcpIp();

	int communicateMsg(System::Int32 msg);
	static RustlightUpdClient* rustlightUpdClientSingleton;
	static void initClass(printFunction i_printf);

protected:
/*	SOCKET rustlightSocket;
	sockaddr_in server;
	sockaddr_in client;  */
	printFunction info_printf;

};

//typedef RustlightUpdClient* RustlightUpdClientPTR;

//RustlightUpdClientPTR pRustlightUpdClient;

#endif