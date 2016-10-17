// TriacControlCenter.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"
// #include "rustlightUpdClient.h"
#include <string.h>
//#include <time.h>
//#include <winsock.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>


//#pragma comment(lib,"ws2_32.lib")


using namespace TriacControlCenter;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew Form1());
	return 0;
}
