///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Tracking.cpp
//
// Author:      David Borland
//
// Description: Main Tracking of Azrael.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "Tracking.h"

#include <algorithm>

#include <wx/datetime.h>


Tracking::Tracking() {
    gotWorkspace = false;
}

Tracking::~Tracking() {
    delete tracker;

    trackerLog.close();
}


bool Tracking::Initialize(std::vector<AzraelImage*>* avatarImages) {
    // Initialize VRPN
    tracker = new vrpn_Tracker_Remote("UbiSense@localhost");
    tracker->register_change_handler(this, &Tracking::HandleTracker);

// This should be how it's done, but it sometimes hangs...
/*
    tracker->register_change_handler(this, &Tracking::HandleWorkspace);

    // Get the workspace from the tracker
    tracker->request_workspace();
    for (int i = 0; i < 5000; i++) {
        tracker->mainloop();

        vrpn_SleepMsecs(1);

        if (gotWorkspace) break;
    }
    if (!gotWorkspace) {
        wxLogMessage("Tracking::Initialize() : Could not get workspace from tracker");
        return false;
    }   
*/
    // ...so just hard code it...
    SetRoomExtents(Vec2(0.0, 0.0), Vec2(6.5, 6.5));


    // Open the tracker log
    wxDateTime now = wxDateTime::Now();
    std::string fileName = "Logs/";
    fileName += now.FormatISODate().c_str();
    fileName += "_";
    fileName += now.FormatISOTime().c_str();
    fileName += ".txt";

    for (int i = 0; i < (int)fileName.size(); i++) {
        if (fileName[i] == ':') fileName[i] = '-';
    }
    trackerLog.open(fileName.c_str(), std::fstream::out);

    // Copy the avatarImages pointer
    avatars = avatarImages;

    return true;
}


void Tracking::Update() {
    tracker->mainloop();    
}


void Tracking::Reset() {
    for (int i = 0; i < (int)viewers.size(); i++) {
        viewers[i]->Reset();
    }
    
    timeval t;
    vrpn_gettimeofday(&t, NULL);
    trackerLog << "Reset " << t.tv_sec << " " << t.tv_usec << std::endl;

    victim = NULL;
}


const Vec2& Tracking::GetRoomMin() const {
    return roomMin;
}

const Vec2& Tracking::GetRoomMax() const {
    return roomMax;
}


int Tracking::GetNumberOfViewers() const {
    return (int)viewers.size();
}

Viewer* Tracking::GetViewer(int index) const {
    // XXX : No bounds checking on index is performed
    return viewers[index];
}


float Tracking::GetAverageDistance(const Vec2& position, int n) const {
    std::vector<float> distances;

    for (int i = 0; i < (int)viewers.size(); i++) {
        Vec2 viewerPos = viewers[i]->GetPosition();
        distances.push_back((viewerPos - position).Magnitude());
    }

    if (n > 0 && n < (int)viewers.size()) {
        sort(distances.begin(), distances.end());
    }
    else {
        n = (int)viewers.size();
    }

    float average = 0.0;
    for (int i = 0; i < n; i++) {
        average += distances[i];
    }
    average /= n;

    return average;
}


void Tracking::PickVictim() {
    float maxDistance = -1.0;
    int victimIndex = -1;
    victim = NULL;
    for (int i = 0; i < (int)viewers.size(); i++) {
        float distance = viewers[i]->GetAverageClosestDistance();
        if (distance > maxDistance) {
            maxDistance = distance;
            victim = viewers[i];
            victimIndex = i;
        }
    }

    trackerLog << "Victim " << victimIndex << std::endl;
}

Viewer* Tracking::GetVictim() {
    return victim;
}


void Tracking::SetRoomExtents(const Vec2& min, const Vec2& max) {
    roomMin = min;
    roomMax = max;

    for (int i = 0; i < (int)viewers.size(); i++) {
        viewers[i]->SetWorkspace(roomMin, roomMax);
    }

    gotWorkspace = true;
}

void Tracking::UpdateViewer(int index, const Vec3& position, const timeval& time) {
    // Check for removal
    if (position.X() < -9.0 && 
        position.Y() < -9.0 && 
        position.Z() < -9.0) {

        // DON'T REMOVE DURING VICTIMIZING
        if (!victim) {
            // Check for validity
            if (index >= (int)sensors.size()) return;
            if (sensors[index] == NULL) return;

            // Find the viewer
            for (int i = 0; i < (int)viewers.size(); i++) {
                if (viewers[i] == sensors[index] /* Pointer comparison...*/) {
                    viewers.erase(viewers.begin() + i);

                    // Put a check here just to be safe...
                    if (i < (int)avatars->size()) {
                        delete (*avatars)[i];
                        avatars->erase(avatars->begin() + i);
                    }

                    delete sensors[index];
                    sensors[index] = NULL;

                    // Add to the log
                    trackerLog << index << " Remove " << time.tv_sec << " " << time.tv_usec << std::endl; 

                    return;
                }
            }
        }
    }


    // Update the sensors, increasing the vector size if necessary
    if (index >= (int)sensors.size()) {
        int oldSize = (int)sensors.size();
        sensors.resize(index + 1);

        // Add NULL pointers for sensors not seen yet
        for (int i = oldSize; i < index; i++) {
            sensors[i] = NULL;
        }

        // Create a new sensor
        sensors[index] = new Viewer();
        sensors[index]->SetWorkspace(roomMin, roomMax);

        // Add to the viewers
        viewers.push_back(sensors[index]);
    }
    else if (sensors[index] == NULL) {
        // Create a new sensor
        sensors[index] = new Viewer();
        sensors[index]->SetWorkspace(roomMin, roomMax);

        // Add to the viewers
        viewers.push_back(sensors[index]);
    }


    // Get closest distance from other viewers
    if ((int)viewers.size() > 1) {
        Vec2 p0 = sensors[index]->GetPosition();
        float closest = 10.0;
        for (int i = 0; i < (int)sensors.size(); i++) {
            if (i != index && sensors[i] != NULL) {
                Vec2 p1 = sensors[i]->GetPosition();
                float distance = p0.Distance(p1);
                if (distance < closest) {
                    closest = distance;
                }
            }
        }
        sensors[index]->SetCurrentClosestDistance(closest);
    }
 

    // Update
    sensors[index]->Update(position, time);


    // Add to the log
    trackerLog << index << " " << 
                  position.X() << " " << position.Y() << " " << position.Z() << " " <<
                  time.tv_sec << " " << time.tv_usec << std::endl; 
}

/*
void VRPN_CALLBACK Tracking::HandleWorkspace(void* userData, const vrpn_TRACKERWORKSPACECB w) {
    Tracking* tracking = static_cast<Tracking*>(userData);
    tracking->SetRoomExtents(Vec2(w.workspace_min[0], w.workspace_min[1]), 
                             Vec2(w.workspace_max[0], w.workspace_max[1]));
}
*/

void VRPN_CALLBACK Tracking::HandleTracker(void* userData, const vrpn_TRACKERCB t) {
    Tracking* tracking = static_cast<Tracking*>(userData);
    tracking->UpdateViewer(t.sensor, 
                           Vec3(t.pos[0], t.pos[1], t.pos[2]), 
                           t.msg_time);
}