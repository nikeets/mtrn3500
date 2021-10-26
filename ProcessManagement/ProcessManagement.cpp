
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define NUM_UNITS 5

bool IsProcessRunning(const char* processName);
void StartProcesses();

//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("GPS.exe"),
	TEXT("LASER.exe"),
	TEXT("VehicleControl.exe"),
	TEXT("Display.exe"),
	TEXT("Camera.exe")
};

int main()
{
    // Creating Shared Memory Objects
    SMObject PMObj(_TEXT("PMObj"), sizeof(ProcessManagement));
    PMObj.SMCreate();
    PMObj.SMAccess();
    ProcessManagement *PMSMPtr = nullptr;

    SMObject LaserObj(_TEXT("LaserObj"), sizeof(SM_Laser));
    LaserObj.SMCreate();
    LaserObj.SMAccess();
    SM_Laser *LaserSMPtr = (SM_Laser*)LaserObj.pData;;

    SMObject GPSObj(_TEXT("GPSObj"), sizeof(SM_GPS));
    GPSObj.SMCreate();
    GPSObj.SMAccess();
	SM_GPS *GPSSMPtr = nullptr;

    // Starting Processess
	StartProcesses();
	
	while (!_kbhit()) {}
    // Ending Processes
    ::_tsystem(_T("taskkill /F /T /IM GPS.exe"));
    System::Threading::Thread::Sleep(50);

    ::_tsystem(_T("taskkill /F /T /IM Laser.exe"));
    System::Threading::Thread::Sleep(50);

    ::_tsystem(_T("taskkill /F /T /IM Vehicle_Control.exe"));
    System::Threading::Thread::Sleep(50);

    ::_tsystem(_T("taskkill /F /T /IM Display.exe"));
    System::Threading::Thread::Sleep(50);

    ::_tsystem(_T("taskkill /F /T /IM Camera.exe"));
    System::Threading::Thread::Sleep(50);

    ::_tsystem(_T("taskkill /F /T /IM ProcessManagement.exe"));
}


//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp((const char *)entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


void StartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning((const char *)Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", (const char *)Units[i], GetLastError());
				_getch();
			}
			std::cout << "Started: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}

