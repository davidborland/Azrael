///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        ViolentImage.h
//
// Author:      David Borland
//
// Description: Violent imagery
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef VIOLENTIMAGE_H
#define VIOLENTIMAGE_H


#include "AzraelImage.h"


class ViolentImage : public AzraelImage {
public:    
    ViolentImage();

    virtual void UpdateDistance(float distance);
};


#endif