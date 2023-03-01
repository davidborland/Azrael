///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        GuardImage.h
//
// Author:      David Borland
//
// Description: Images of guards.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef GUARDIMAGE_H
#define GUARDIMAGE_H

#include "AzraelImage.h"


class GuardImage : public AzraelImage {
public:    
    GuardImage();

    virtual void UpdateDistance(float distance);

    virtual bool DeleteOnNewQuadrant();
};


#endif