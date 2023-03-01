///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        VideoImageConnection.cpp
//
// Author:      David Borland
//
// Description: Handles copying data from a video (or series of videos) to an image
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "VideoImageConnection.h"


VideoImageConnection::VideoImageConnection(AzraelVideo* azraelVideo, AzraelImage* azraelImage) {
    videos.push_back(azraelVideo);
    image = azraelImage;
}

VideoImageConnection::~VideoImageConnection() {
    for (int i = 0; i < (int)videos.size(); i++) {
        videos[i]->Stop();
    }
}


bool VideoImageConnection::Update() {
    videos.back()->Update();

    if (videos.back()->IsStopped()) {
        videos.pop_back();

        if ((int)videos.size() == 0) {
            return false;
        }

        videos.back()->Play();
    }
    else {
        image->SetTextureData(videos.back()->GetBuffer());
    }

    return true;
}


void VideoImageConnection::AddVideo(AzraelVideo* azraelVideo) {
    videos.insert(videos.begin(), azraelVideo);
}


AzraelImage* VideoImageConnection::GetImage() {
    return image;
}

AzraelVideo* VideoImageConnection::GetCurrentVideo() {
    return videos.back();
}