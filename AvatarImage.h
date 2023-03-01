///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AvatarImage.h
//
// Author:      David Borland
//
// Description: Images representing viewers.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef AVATARIMAGE_H
#define AVATARIMAGE_H


#include "AzraelImage.h"


class AvatarImage : public AzraelImage {
public:    
    AvatarImage();

    virtual void UpdateDistance(float distance);

private:
    float minScale;
    float maxScale;

    float minDistance;
    float maxDistance;

    float maxOpacity;
};


#endif