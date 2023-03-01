///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        ViolentImage.cpp
//
// Author:      David Borland
//
// Description: Violent imagery.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "ViolentImage.h"


ViolentImage::ViolentImage() : AzraelImage() {
    blurRadius = 0;
    actualBlurRadius = 0;
}


void ViolentImage::UpdateDistance(float distance) {
    if (distance < 1.5) {
        scale = 0.5;
        desiredScale = (float)scale;
    }
    else {
        scale = 0.05;
        desiredScale = (float)scale;
    }
}
