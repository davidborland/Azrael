///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        ProjectorShutter.cpp
//
// Author:      David Borland
//
// Description: Use ENTTEC DMX USB PRO widget to talk to wahlberg projector shutter.  Code
//              borrows heavily from DMXUSB Pro example code from www.enttec.com
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include <windows.h>

#include "ProjectorShutter.h"

#include <string>

#include <wx/log.h>


const unsigned char ProjectorShutter::DMX_START_CODE = 0x7E;
const unsigned char ProjectorShutter::DMX_END_CODE = 0xE7;
const unsigned char ProjectorShutter::LSB_CODE = 0xFF;
const unsigned int ProjectorShutter::SHIFT_BYTE = 8;
const unsigned int ProjectorShutter::HEADER_LENGTH = 4;

const unsigned short ProjectorShutter::SET_DMX_TX_MODE = 6;


ProjectorShutter::ProjectorShutter() {
    comHandle = NULL;
}

ProjectorShutter::~ProjectorShutter() {
    CloseHandle(comHandle);
}


bool ProjectorShutter::Initialize() {
    // Search for a COM port in the registry
    unsigned char deviceName[256];
    bool test;
    for (int i = 0; i < 50; i++) {
        if ((test = SearchForDevice(i, deviceName))) break;
	}

	if (!test) {
        wxLogMessage("ProjectorShutter::Initialize() : No device connected");
		return false;
	}

    wxLogMessage("ProjectorShutter::Initialize() : Found device at %s", deviceName);


    // Initialize the device
    char comString [10];
	sprintf(comString, "\\\\.\\%s", deviceName); 
    LPSTR portName = _T(comString);

    comHandle = CreateFile(portName, 
                           GENERIC_READ | GENERIC_WRITE, 
        		           0,								// DWORD dwShareMode, 
		                   NULL,							// LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
		                   OPEN_EXISTING,					// DWORD dwCreationDispostion, 
		                   0, //FILE_FLAG_OVERLAPPED,		// DWORD dwFlagsAndAttributes, 
		                   NULL);							// HANDLE hTemplateFile

    if (comHandle < 0) {
        wxLogMessage("ProjectorShutter::Initialize() : Could not open COM port");
        return false;
    }

    
    // Set parameters
    SetParameters();


    // Flush buffers
    if (!FlushFileBuffers(comHandle)) {
        wxLogMessage("ProjectorShutter::Initialize() : Could not flush buffers");
        return false;
    }

    return true;
}


void ProjectorShutter::Open() {
    const unsigned short size = 512;
    unsigned char data[size];
    memset(data, 0, size);
    SendData(SET_DMX_TX_MODE, data, size);
}

void ProjectorShutter::Close() {
    const unsigned short size = 512;
    unsigned char data[size];
    memset(data, 128, size);
    SendData(SET_DMX_TX_MODE, data, size);
}


bool ProjectorShutter::SendData(unsigned short label, unsigned char* data, unsigned short length) {
	BOOL res = 0;
	DWORD bytesWritten = 0;
	unsigned short size = 0;
	unsigned char index = 0;
    int result;

	unsigned char header[HEADER_LENGTH];
	header[index] = DMX_START_CODE;
	header[++index] = label;
	header[++index] = length & LSB_CODE;
	header[++index] = length >> SHIFT_BYTE;


    // Write the header first
	result = WriteFile(comHandle,					
		              (unsigned char*)header,		
		               HEADER_LENGTH,							
		               &bytesWritten,
		               NULL);
    if (!result || (bytesWritten != HEADER_LENGTH)) {
        wxLogMessage("ProjectorShutter::SendData() : Can't write header");
        return false;
    }


    // Write the frame
	result = WriteFile(comHandle,					
		              (unsigned char*)data,			
		               length,							
		               &bytesWritten,
		               NULL);
	if (!result || (bytesWritten != length)) {
        wxLogMessage("ProjectorShutter::SendData() : Can't write data");
        return false;
    }


    // Write the end of frame
    unsigned char endCode = DMX_END_CODE;
	result = WriteFile(comHandle,					
		              (unsigned char*)&endCode,		
		               1,								
		               &bytesWritten,
		               NULL);
	if (!result || (bytesWritten != 1)) {
        wxLogMessage("ProjectorShutter::SendData() : Can't write end");
        return false;
    }

	return true;
}


bool ProjectorShutter::SearchForDevice(int port, unsigned char* deviceName) {
    HKEY hKey;
    DWORD deviceNameLen;
    DWORD keyNameLen;
    char keyName[256];

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        wxLogMessage("ProjectorShutter::SearchForDevice : Could not open registry key");
		return false;
	}
	
	deviceNameLen = 80;
    keyNameLen = 100;
    if (RegEnumValue(hKey, port, keyName, &keyNameLen, NULL, NULL, deviceName, &deviceNameLen) != ERROR_SUCCESS) {
        wxLogMessage("ProjectorShutter::SearchForDevice : Could not read registry value");
        return false;
    }

    if (!strncmp(keyName,"\\Device\\VCP", 11)) {
		// Found a serial COM device
		return true;
	}

 	RegCloseKey(hKey);

	return false;
}

void ProjectorShutter::SetParameters() {
	// SetCommState
	DCB dcb;
	GetCommState(comHandle, &dcb);

	dcb.fBinary = TRUE;         // Binary mode, no EOF check
	dcb.fErrorChar = FALSE;     // Disable error replacement
	dcb.fAbortOnError = FALSE;  // Disable abort on error

	// Set the baud rate
	dcb.BaudRate = 57600;

	// Set the data characteristics
	dcb.ByteSize = 8;           // 8 data bits
	dcb.StopBits = ONESTOPBIT;  // 1 stop bit
	dcb.fParity = NOPARITY;     // No parity
	dcb.Parity = 0;

	// Disable all flow control stuff
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fInX = FALSE;
	dcb.fOutX = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutxCtsFlow = FALSE;

    SetCommState(comHandle, &dcb);


    // Set timeouts
    COMMTIMEOUTS timeouts;
	GetCommTimeouts(comHandle, &timeouts);
	
    // Set timimg values
	timeouts.ReadIntervalTimeout = 500;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 500;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 500;

	SetCommTimeouts(comHandle, &timeouts);
}