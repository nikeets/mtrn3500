//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h> //_kbhit()
#include "SMObject.h"
#include "SMStructs.h"
#include <Math.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {

	// Create Shared Memory objects
	SMObject PMObj(_TEXT("PMObj"), sizeof(ProcessManagement));
	PMObj.SMAccess();
	ProcessManagement* PMSMPtr = (ProcessManagement*)PMObj.pData;

	SMObject LaserObj(_TEXT("Laser"), sizeof(SM_Laser));
	LaserObj.SMAccess();
	SM_Laser* LaserSMPtr = (SM_Laser*)LaserObj.pData;

	// Declare handle to TcpClient object
	TcpClient^ Client;

	// Get your port number
	int PortNumber = 23000;

	// Instantiate the TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);

	// Configure Client Properties
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Allocate space for send data buffer
	array<unsigned char>^ SendData = nullptr;
	SendData = gcnew array<unsigned char>(1024);

	// Form a data buffer to receive data
	array<unsigned char>^ RecvData = nullptr;
	RecvData = gcnew array<unsigned char>(5000);

	// Create a string to store received data
	String^ PrintData = nullptr;
	String^ LaserData = nullptr;

	// Create an array to store formatted LaserData
	array<String^>^ StringArray = nullptr;

	// Create variables to hold LaserData
	double StartAngle = 0;
	double Resolution = 0;
	int NumRanges = 0;
	array<double>^ Range = nullptr;
	array<double>^ RangeX = nullptr;
	array<double>^ RangeY = nullptr;

	// Prepare your data
	String^ DataRequest = gcnew String("sRN LMDscandata");
	String^ Authentication = gcnew String("5208444\n");

	// Create a stream object
	NetworkStream^ Stream = Client->GetStream();

	// Encoding data to send
	SendData = Encoding::ASCII->GetBytes(Authentication);

	// Authenticating to connect to the Laser rangefinder
	Stream->Write(SendData, 0, SendData->Length);
	Stream->Read(RecvData, 0, RecvData->Length);
	PrintData = Encoding::ASCII->GetString(RecvData); // Binary byte to String decoding
	Console::WriteLine(PrintData);

	// Set SendData as the DataRequest
	SendData = Encoding::ASCII->GetBytes(DataRequest);

	while (!_kbhit())
	{
		// Sending the sRN LMDscandata request
		Stream->WriteByte(0x02); // STX byte
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03); // ETX byte

		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);

		// Read the response from the Laser rangefinder and store it in RecvData string
		Stream->Read(RecvData, 0, RecvData->Length);

		// Store the received data into LaserData string
		LaserData = Encoding::ASCII->GetString(RecvData);

		// Print to console the response from Laser rangefinder (including everything that's NOT the rangefinder data)
		// Console::WriteLine(LaserData);

		// Split LaserData into individual sub strings separated by spaces
		StringArray = LaserData->Split(' ');

		// Indexes of StartAngle, Resolution and NumRanges from the manual
		StartAngle = System::Convert::ToInt32(StringArray[23], 16);
		Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
		NumRanges = System::Convert::ToInt32(StringArray[25], 16);

		// Store NumRanges into Shared Memory
		// LaserSMPtr->NumPoints = NumRanges;

		// Print the StartAngle, Resolution and NumRanges to the console
		Console::Write("Start Angle: {0, 10:F3} Resolution: {1, 10:F3} NumRanges: {2}", StartAngle, Resolution, NumRanges);

		// Setting arrays to for the x, y and range data
		Range = gcnew array<double>(NumRanges);
		RangeX = gcnew array<double>(NumRanges);
		RangeY = gcnew array<double>(NumRanges);

		// Storing Range, X and Y values into the arrays
		for (int i = 0; i < NumRanges; i++)
		{
			// Convert the data from rangefinder into usable values
			Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
			RangeX[i] = Range[i] * sin(i * Resolution);
			RangeY[i] = -Range[i] * cos(i * Resolution);

			// Put X and Y values into the Shared Memory array
			// LaserSMPtr->xRange[i] = RangeX[i];
			// LaserSMPtr->yRange[i] = RangeY[i];
		}

		// Print Range, X and Y values to console of directly in front of the robot
		Console::WriteLine("\t[X {0}: {1, 10:F3}, Y {2}: {3, 10:F3}]", NumRanges / 2, RangeX[NumRanges / 2], NumRanges / 2, RangeY[NumRanges / 2]);

		// Wait 1000ms before repeating
		System::Threading::Thread::Sleep(1000);
	}

	Stream->Close();
	Client->Close();

	Console::ReadKey();
}