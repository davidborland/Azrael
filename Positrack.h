///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        PosiTrack.h
//
// Author:      David Borland
//
// Description: Write pan and tilt values over RS-232 (using vrpn) to 
//              control a PosiTrack device.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#ifndef POSITRACK_H
#define POSITRACK_H


#include <string>
#include <Vec3.h>


class PosiTrack {
public:
    PosiTrack();
    ~PosiTrack();

    bool Initialize(const std::string& portName = "COM1");

    void SetPosition(const Vec3& p);
    void SetOrientation(const Vec3& v);

    void PointAt(const Vec3& p);   

    void SetPanAngle(float pan);
    void SetTiltAngle(float tilt);

    void Wiggle();

private:
    int port;

    Vec3 position;
    Vec3 orientation;

    float currentPanAngle;
    float currentTiltAngle;

    static const float panMin;
    static const float panMax;

    static const float tiltMin;
    static const float tiltMax;

    static const float PI;

    void SendCommand(const std::string& command);

    void TurnOnResponse();
    void TurnOffResponse();

    void Pan(float speed);
    void Tilt(float speed);

    void ReadLevels();

    unsigned char CheckSum(const unsigned char* buffer, unsigned char size);
};


#endif