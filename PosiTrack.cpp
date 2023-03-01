///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        PosiTrack.cpp
//
// Author:      David Borland
//
// Description: Write pan and tilt values over RS-232 (using vrpn) to 
//              control a PosiTrack device.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "PosiTrack.h"

#include <vrpn_Shared.h>
#include <vrpn_Serial.h>

#include <Quat.h>

#include <wx/log.h>


const float PosiTrack::panMin = 1.0;
const float PosiTrack::panMax = 359.0;
const float PosiTrack::tiltMin = 0.0;
const float PosiTrack::tiltMax = 180.0;


PosiTrack::PosiTrack() {
}

PosiTrack::~PosiTrack() {
    vrpn_close_commport(port);
}


bool PosiTrack::Initialize(const std::string& portName) {
    // Open the port
    port = vrpn_open_commport(portName.c_str(), 9600, 8, vrpn_SER_PARITY_NONE);

    // Check validity
    if (port < 0) {
        return false;
    }
    
    // Try to clear
    if (vrpn_set_rts(port) == 0) {
        return false;
    }

    // Want info back
    TurnOnResponse();

    // Set up
    SendCommand("CLEAR ERRORS");

    SendCommand("ACCEL CONTROL=OFF");    
    SendCommand("PANACC127");
    SendCommand("TILTACC127");

    SendCommand("CLEAR HOME");

    // Set to maximum speed for pan and tilt
    SendCommand("G1S127");
    SendCommand("G2S127");

    SendCommand("G1A0S127");
    SendCommand("G2A0S127");


    // Generate values
    Wiggle();

    return true;
}


void PosiTrack::SetPosition(const Vec3& p) {
    position = p;
}

void PosiTrack::SetOrientation(const Vec3& v) {
    orientation = v;
    orientation.Normalize();
}


void PosiTrack::PointAt(const Vec3& p) {
    // Get the current device levels
    ReadLevels();


    // Vector from the PosiTrack to the given position
    Vec3 v = p - position;

    // Get euler angles from a quaternion that rotates the home
    // orientation to the vector
    Quat q(orientation, v);
    double yaw, pitch, roll;
    q.GetEulerAngles(yaw, pitch, roll);

    float angleRange = 20.0f;


    // Get desired pan angle
    float panAngle = (float)Quat::RadiansToDegrees(yaw);
    if (panAngle < 0.0) panAngle += 360.0f;
    panAngle = 360.0f - panAngle;

    // Set the pan
    float pan = panAngle - currentPanAngle;
    if (pan > angleRange) pan = angleRange;
    else if (pan < -angleRange) pan = -angleRange;
    pan = pan / angleRange;
    Pan(pan);


    // Get desired tilt angle
    float tiltAngle = 90.0f - (float)Quat::RadiansToDegrees(pitch);

    // Set the tilt    
    float tilt = tiltAngle - currentTiltAngle;
    if (tilt > angleRange) tilt = angleRange;
    else if (tilt < -angleRange) tilt = -angleRange;
    tilt = tilt / angleRange;
    Tilt(-tilt);
}


void PosiTrack::SetPanAngle(float pan) {
    // Constrict angle to pan range
    if (pan < panMin) pan = panMin;
    else if (pan > panMax) pan = panMax;

    // Map angle to 0...65535
    unsigned short position = (unsigned short)((pan - panMin) / (panMax - panMin) * 65535);

    // Convert to a string
    const int bufferSize = 6;
    char buffer[bufferSize];
    memset(buffer, 0, bufferSize);
    sprintf(buffer, "%d", position);

    // Send the command
    std::string command = "G1L";
    command += buffer;

    SendCommand(command);
}

void PosiTrack::SetTiltAngle(float tilt) {
    // Constrict angle to tilt range
    if (tilt < tiltMin) tilt = tiltMin;
    else if (tilt > tiltMax) tilt = tiltMax;

    // Map angle to 0...65535
    unsigned short position = (unsigned short)((tilt - tiltMin) / (tiltMax - tiltMin) * 65535);

    // Convert to a string
    const int bufferSize = 6;
    char buffer[bufferSize];
    memset(buffer, 0, bufferSize);
    sprintf(buffer, "%d", position);

    // Send the command
    std::string command = "G2L";
    command += buffer;

    SendCommand(command);
}


void PosiTrack::Wiggle() {
    Pan(1.0);
    Tilt(1.0);

    vrpn_SleepMsecs(500);

    Pan(-1.0);
    Tilt(-1.0);

    vrpn_SleepMsecs(500);
}


void PosiTrack::SendCommand(const std::string& command) {
    unsigned char bSize = (unsigned char)command.size() + 5;
    unsigned char* buffer = new unsigned char[bSize];

    buffer[0] = '*';                                // Attention Byte
    buffer[1] = 5;                                  // Command Number
    buffer[2] = 0;                                  // ?
    buffer[3] = (unsigned char)command.size();      // Num Bytes
    for (int i = 0; i < (unsigned char)command.size(); i++) {
        buffer[4 + i] = command[i];                 // Command   
    }
    buffer[bSize - 1] = CheckSum(buffer, bSize - 1);// Check Sum       

    vrpn_write_characters(port, buffer, bSize);
    vrpn_drain_output_buffer(port);

    delete [] buffer;
}

void PosiTrack::TurnOnResponse() {
    unsigned char buffer[5];

    buffer[0] = '*';                                // Attention Byte
    buffer[1] = 10;                                 // Command Number
    buffer[2] = 240;                                // Mask1
    buffer[3] = 0;                                  // Mask2
    buffer[4] = CheckSum(buffer, 4);                // Check Sum

    vrpn_write_characters(port, buffer, 5);
    vrpn_drain_output_buffer(port);
}

void PosiTrack::TurnOffResponse() {
    unsigned char buffer[5];

    buffer[0] = '*';                                // Attention Byte
    buffer[1] = 10;                                 // Command Number
    buffer[2] = 0;                                  // Mask1
    buffer[3] = 0;                                  // Mask2
    buffer[4] = CheckSum(buffer, 4);                // Check Sum

    vrpn_write_characters(port, buffer, 5);
    vrpn_drain_output_buffer(port);
}


void PosiTrack::Pan(float speed) {
    unsigned char buffer[6];

    unsigned char level = (unsigned char)((speed + 1.0f) * 0.5f * 255);

    buffer[0] = '*';
    buffer[1] = 3;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = level;
    buffer[5] = CheckSum(buffer, 5);

    vrpn_write_characters(port, buffer, 6);
    vrpn_drain_output_buffer(port);
}

void PosiTrack::Tilt(float speed) {
    unsigned char buffer[6];

    unsigned char level = (unsigned char)((speed + 1.0f) * 0.5f * 255);

    buffer[0] = '*';
    buffer[1] = 3;
    buffer[2] = 0;
    buffer[3] = 1;
    buffer[4] = level;
    buffer[5] = CheckSum(buffer, 5);

    vrpn_write_characters(port, buffer, 6);
    vrpn_drain_output_buffer(port);
}


void PosiTrack::ReadLevels() {
    unsigned char buffer[256];

    // Get most recent update
    int count;
    int i = 0;

    while ((count = vrpn_read_available_characters(port, buffer, 256)) > 0) {
        unsigned char* b;
        int i = 0;
        while (i < count) {
            b = buffer + i;

            // Find attention byte
            if (b[0] == '&' && 
                b[1] == 8   &&
                b[2] == 0   &&
               (b[3] == 5 || b[3] == 6)) {

                unsigned short position = ((unsigned short)b[4] << 8) | (unsigned short)b[5];

                if (b[3] == 5) {
                    currentPanAngle = ((float)position / 65535.0f) * (panMax - panMin) + panMin;
//wxLogMessage("currentPanAngle = %f", currentPanAngle);
                }
                else {
                    currentTiltAngle = ((float)position / 65535.0f) * (tiltMax - tiltMin) + tiltMin;
//wxLogMessage("currentTiltAngle = %f", currentTiltAngle);
                }

                i += 7;
            }
            else {
                i++;
            }
        }
    }
}


unsigned char PosiTrack::CheckSum(const unsigned char* buffer, unsigned char size) {
    unsigned long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return (unsigned char)(sum % 256);
}