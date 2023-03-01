///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        QuadrantImage.cpp
//
// Author:      David Borland
//
// Description: Images restricted to quadrants.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "QuadrantImage.h"


QuadrantImage::QuadrantImage() : AzraelImage() {
}


void QuadrantImage::UpdateDistance(float distance) {
    // Set the amount of blur
    float maxDistance = 5.0;
    float minDistance = 2.5;
    distance = distance > maxDistance ? maxDistance : distance;
    distance = distance < minDistance ? minDistance : distance;

    blurRadius = (unsigned int)((distance - minDistance) / (maxDistance - minDistance) * maxBlurRadius);
}