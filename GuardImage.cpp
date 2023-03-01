///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        GuardImage.h
//
// Author:      David Borland
//
// Description: Images of guards.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "GuardImage.h"


GuardImage::GuardImage() : AzraelImage() {
    blurRadius = 0;
    actualBlurRadius = 0;
}


void GuardImage::UpdateDistance(float distance) {
    // Set the amount of blur
    float maxDistance = 5.0;
    float minDistance = 2.5;
    distance = distance > maxDistance ? maxDistance : distance;
    distance = distance < minDistance ? minDistance : distance;

    blurRadius = (unsigned int)((1.0 - (distance - minDistance) / (maxDistance - minDistance)) * maxBlurRadius);
}


bool GuardImage::DeleteOnNewQuadrant() {
    return true;
}