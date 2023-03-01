///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        VideoImageConnection.h
//
// Author:      David Borland
//
// Description: Handles copying data from a video (or series of videos) to an image
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef VIDEOIMAGECONNECTION_H
#define VIDEOIMAGECONNECTION_H


#include "AzraelVideo.h"
#include "AzraelImage.h"

#include <vector>


class VideoImageConnection {
public:
    VideoImageConnection(AzraelVideo* videoFile, AzraelImage* azraelImage);
    ~VideoImageConnection();

    bool Update();
    
    void AddVideo(AzraelVideo* video);

    AzraelImage* GetImage();
    AzraelVideo* GetCurrentVideo();

private:
    std::vector<AzraelVideo*> videos;
    AzraelImage* image;
};


#endif