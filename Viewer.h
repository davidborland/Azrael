///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Viewer.h
//
// Author:      David Borland
//
// Description: A Viewer being tracked.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef Viewer_H
#define Viewer_H


#include <Vec2.h>
#include <Quat.h>

#include <vrpn_Shared.h>


class Viewer {
public:
    Viewer();
    ~Viewer();

    void Update(const Vec3& newPosition, const timeval& newTime);

    void SetCurrentClosestDistance(float distance);
    float GetAverageClosestDistance();

    const Vec3& GetPosition() const;

    int GetQuadrant() const;
    int GetOldQuadrant() const;

    bool TriggerFragment();

    void Reset();

    void SetWorkspace(const Vec2& roomMinimum, const Vec2& roomMaximum);

    Vec2 ProjectToWall() const;

private:
    Vec3 position;
    Vec2 velocity;
    timeval time;

    Vec2 oldPosition;
    Vec2 oldVelocity;
    timeval oldTime;

    double distanceSum;
    unsigned long distanceCount;

    int quadrant;
    int oldQuadrant;

    Vec2 roomMin;
    Vec2 roomMax;

    // Cheat, since we know the room is square, and starts at zero...
    Vec2 roomCenter;

    bool triggerFragment;

    void ComputeQuadrant();
};


#endif

