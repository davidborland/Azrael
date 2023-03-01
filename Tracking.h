///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Tracking.h
//
// Author:      David Borland
//
// Description: Handles viewer tracking for Azrael.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TRACKING_H
#define TRACKING_H


#include <vector>

#include <wx/log.h>     // This must be included before Video.h

#include <fstream>

#include <vrpn_Tracker.h>

#include "Viewer.h"
#include "AzraelImage.h"


class Tracking {
public:
    Tracking();
    ~Tracking();

    bool Initialize(std::vector<AzraelImage*>* avatarImages);

    void Update();

    void Reset();

    const Vec2& GetRoomMin() const;
    const Vec2& GetRoomMax() const;

    int GetNumberOfViewers() const;

    Viewer* GetViewer(int index) const;

    // Computes the average distance of the viewers from a position.  If n > 0,
    // returns the average distance of the n closest viewers
    float GetAverageDistance(const Vec2& position, int n = -1) const;

    void PickVictim();
    Viewer* GetVictim();

    // Called from vrpn callbacks
    void SetRoomExtents(const Vec2& min, const Vec2& max);
    void UpdateViewer(int index, const Vec3& position, const timeval& time);

private:
    std::vector<Viewer*> sensors;   // The index here relates directly to the Tracker sensor in vrpn
    std::vector<Viewer*> viewers;   // Pointers to the potentialViewers that are active
    Viewer* victim;

    std::vector<AzraelImage*>* avatars;

    vrpn_Tracker_Remote* tracker;

    Vec2 roomMin;
    Vec2 roomMax;

    bool gotWorkspace;

    std::fstream trackerLog;

    // Vrpn callbacks
//    static void VRPN_CALLBACK HandleWorkspace(void* userData, const vrpn_TRACKERWORKSPACECB w);
    static void VRPN_CALLBACK HandleTracker(void* userData, const vrpn_TRACKERCB t); 
};


#endif