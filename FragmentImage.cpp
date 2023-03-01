///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        FragmentImage.h
//
// Author:      David Borland
//
// Description: Images representing fragments.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "FragmentImage.h"


FragmentImage::FragmentImage() : AzraelImage() {
    blurRadius = 0;
    actualBlurRadius = 0;
}


void FragmentImage::UpdateDistance(float distance) {
    // Set the scale
    float maxDistance = 10.0;
    scale = distance / maxDistance;
    desiredScale = (float)scale;
}


void FragmentImage::Shift() {
    // Don't shift
}