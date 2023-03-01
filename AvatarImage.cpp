///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AvatarImage.h
//
// Author:      David Borland
//
// Description: Images representing viewers.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "AvatarImage.h"


AvatarImage::AvatarImage() : AzraelImage() {
    blurRadius = 0;

    minScale = 0.01f;
    maxScale = 0.2f;
    minDistance = 1.0f;
    maxDistance = 5.0f;
    maxOpacity = 0.5f;
}


void AvatarImage::UpdateDistance(float distance) {
    // Set the scale
    if (distance < minDistance) distance = minDistance;
    else if (distance > maxDistance) distance = maxDistance;

    scale = (distance - minDistance) / (maxDistance - minDistance);
    scale = scale * (maxScale - minScale) + minScale;
    desiredScale = (float)scale;

    // Set the opacity
    opacity = maxOpacity - (float)scale;
}