///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Viewer.cpp
//
// Author:      David Borland
//
// Description: A Viewer being tracked.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "Viewer.h"

#include <wx/log.h>


Viewer::Viewer() {
    distanceSum = 0.0;
    distanceCount = 0.0;

    triggerFragment = false;

    quadrant = oldQuadrant = -1;
}

Viewer::~Viewer() {

}


void Viewer::Update(const Vec3& newPosition, const timeval& newTime) {
    // Normalize the new time
    timeval normTime = vrpn_TimevalNormalize(newTime);


    // Update old values
    oldPosition = position;
    oldVelocity = velocity;
    oldTime.tv_sec = time.tv_sec;
    oldTime.tv_usec = time.tv_usec;

    // Update new values
    position = newPosition;
    time.tv_sec = normTime.tv_sec;
    time.tv_usec = normTime.tv_usec; 

    
    // Compute the inverse of the number of seconds elapsed
    float seconds = 0.001 * vrpn_TimevalMsecs(vrpn_TimevalDiff(time, oldTime));

    // Compute the velocity
    velocity = (position - oldPosition) * (1.0 / seconds);


    // Check if accelerating
    float accelerationThreshold = 1.5;
    if ((velocity - oldVelocity).Magnitude() > accelerationThreshold) {
        triggerFragment = true;
    }


    // Update the quadrant
    ComputeQuadrant();
}


void Viewer::SetCurrentClosestDistance(float distance) {
    distanceSum += distance;
    distanceCount++;
}

float Viewer::GetAverageClosestDistance() {
    if (distanceCount == 0) return 0.0f;
    
    return (float)(distanceSum / (double)distanceCount);
}


const Vec3& Viewer::GetPosition() const {
    return position;
}


int Viewer::GetQuadrant() const {
    return quadrant;
}

int Viewer::GetOldQuadrant() const {
    return oldQuadrant;
}


bool Viewer::TriggerFragment() {
    bool trigger = triggerFragment;
    triggerFragment = false;

    return trigger;
}


void Viewer::Reset() {
    distanceSum = 0.0;
    distanceCount = 0;

    triggerFragment = false;
}


void Viewer::SetWorkspace(const Vec2& roomMinimum, const Vec2& roomMaximum) {
    roomMin = roomMinimum;
    roomMax = roomMaximum;

    roomCenter = Vec2(roomMaximum.X() - roomMinimum.X(),
                      roomMaximum.Y() - roomMinimum.Y());
    roomCenter *= 0.5;
}


Vec2 Viewer::ProjectToWall() const {
    Vec2 result = position;

    Vec2 distance = position - roomCenter;

    if (abs(distance.Y()) > abs(distance.X())) {
        // Project onto horizontal wall
        if (distance.Y() > 0) {
            // Wall 0
            result.Y() = roomMax.Y();
        }
        else {
            // Wall 2
            result.Y() = 0.0;
        }
    }
    else {
        // Project onto vertical wall        
        if (distance.X() > 0) {
            // Wall 1
            result.X() = roomMax.X();
        }
        else {
            // Wall 3
            result.X() = 0.0;
        }
    }

    return result;
}


void Viewer::ComputeQuadrant() {
    float roomWidth = roomMax.X() - roomMin.X();
    float roomLength = roomMax.Y() - roomMin.Y();

    // XXX : Assuming roomMin == 0.0 below

    oldQuadrant = quadrant;

    if (position.Y() > roomLength * 0.5) {
        if (position.X() < roomWidth * 0.5) {
            quadrant = 0;
        }
        else {
            quadrant =  1;
        }
    }
    else {
        if (position.X() > roomWidth * 0.5) {
            quadrant =  2;
        }
        else {
            quadrant =  3;
        }
    }

    if (oldQuadrant == -1) oldQuadrant = quadrant;
}