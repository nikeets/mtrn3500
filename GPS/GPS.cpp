// GPS Module to display Northing, Easting, Height and CRC to console
#using <System.dll>

#include <conio.h>
#include <Math.h>
#include "GPS.h"
#include "SMStructs.h"
#include "SMObject.h"

#define CRC32_POLYNOMIAL 0xEDB88320L

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#pragma pack(4)

struct novatel_GPS // Novatel GPS format (112 bytes)
{
	unsigned int Header;					// Header message
	unsigned char Discards1[40];			// Discard messages
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];			// Discard messages
	unsigned int Checksum;					// CRC message
}nova_GPS;




int main() {

	// Declare handle to TcpClient object
	TcpClient^ Client;

	// Get your port number
	int PortNumber = 24000;

	// Instantiate the TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);

	// Configure Client Properties
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Form a data buffer to receive data
	array<unsigned char>^ RecvData = nullptr;
	RecvData = gcnew array<unsigned char>(256);

	// Create a stream object
	NetworkStream^ Stream = Client->GetStream();

	// Create variables
	bool hasHeader = false;
	unsigned int Header = 0;
	unsigned int GeneratedCRC = 0;
	int i = 0;
	int StartData = 0;
	unsigned char Data;
	unsigned char* bytePtr = (unsigned char*)&nova_GPS;


	while (!_kbhit())
	{

		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 500 ms
		System::Threading::Thread::Sleep(500);

		// Read the response from the GPS and store it in RecvData string
		Stream->Read(RecvData, 0, RecvData->Length);

		// Store the header stream into Header
		while (Header != 0xaa44121c && i < RecvData->Length)
		{
			Data = RecvData[i++];
			Header = ((Header << 8) | Data);

			if (Header == 0xaa44121c) {
				hasHeader = true;
			}
		}

		StartData = i - 4;				// Start of data stream 

		// Notice if there was no header found in stream of data
		if (hasHeader == false) {
			Console::WriteLine("hasHeader is now false");
			return 0;
		}

		// Filling the GPS data struct
		unsigned char* BytePtr = nullptr;
		BytePtr = (unsigned char*)&nova_GPS;
		for (int i = StartData; i < (StartData + sizeof(nova_GPS)); i++)
		{
			*(BytePtr++) = RecvData[i];
		}

		// Print out Easting, Northing, Height and Checksum
		Console::WriteLine("\t[Northing: {0,10:F3}, Easting: {1,10:F3}, Height: {2,10:F3}, Checksum: {3}]", nova_GPS.Northing, nova_GPS.Easting, nova_GPS.Height, nova_GPS.Checksum);

		// Comparing the calculated checksum with checksum sent from stream
		GeneratedCRC = CalculateBlockCRC32(112 - 4, bytePtr);
		Console::WriteLine("Calculated CRC: {0}, Server CRC: {1}, Calc=Server {2}", GeneratedCRC, nova_GPS.Checksum, GeneratedCRC == nova_GPS.Checksum);

		System::Threading::Thread::Sleep(200);

	}

	Stream->Close();
	Client->Close();

	Console::ReadKey();
}