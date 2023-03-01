///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        QuadrantImage.h
//
// Author:      David Borland
//
// Description: Images restricted to quadrants.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef QUADRANTIMAGE_H
#define QUADRANTIMAGE_H


#include "AzraelImage.h"


class QuadrantImage : public AzraelImage {
public:    
    QuadrantImage();

    virtual void UpdateDistance(float distance);
};


#endif