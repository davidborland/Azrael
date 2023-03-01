///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        PatchImage.cpp
//
// Author:      David Borland
//
// Description: Images of patches for victim.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "PatchImage.h"


PatchImage::PatchImage() : AzraelImage() {
    blurRadius = 0;
    actualBlurRadius = 0;
    rendered = false;
}


void PatchImage::UpdateDistance(float distance) {
/*
    // Set the amount of blur
    float maxDistance = 5.0;
    float minDistance = 2.5;
    distance = distance > maxDistance ? maxDistance : distance;
    distance = distance < minDistance ? minDistance : distance;

    blurRadius = (unsigned int)((distance - minDistance) / (maxDistance - minDistance) * maxBlurRadius);
*/
}


bool PatchImage::Rendered() {
    return rendered;
}

void PatchImage::SetRendered() {
    rendered = true;
}