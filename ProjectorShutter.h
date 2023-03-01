///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        ProjectorShutter.h
//
// Author:      David Borland
//
// Description: Use ENTTEC DMX USB PRO widget to talk to wahlberg projector shutter.  Code
//              borrows heavily from DMXUSB Pro example code from www.enttec.com
//
///////////////////////////////////////////////////////////////////////////////////////////////


#ifndef PROJECTORSHUTTER_H
#define PROJECTORSHUTTER_H


class ProjectorShutter {
public:
    ProjectorShutter();
    ~ProjectorShutter();

    bool Initialize();

    void Open();
    void Close();

private:
    HANDLE comHandle;

    // Some constants
    static const unsigned char DMX_START_CODE;
    static const unsigned char DMX_END_CODE;
    static const unsigned char LSB_CODE;
    static const unsigned int SHIFT_BYTE;
    static const unsigned int HEADER_LENGTH;

    static const unsigned short SET_DMX_TX_MODE;

    bool SendData(unsigned short label, unsigned char* data, unsigned short length);
    bool SearchForDevice(int port, unsigned char* deviceName);
    void SetParameters();
};


#endif