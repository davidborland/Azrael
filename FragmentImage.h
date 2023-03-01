///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        FragmentImage.h
//
// Author:      David Borland
//
// Description: Images of fragments.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef FRAGMENTIMAGE_H
#define FRAGMENTIMAGE_H


#include "AzraelImage.h"


class FragmentImage : public AzraelImage {
public:    
    FragmentImage();

    virtual void UpdateDistance(float distance);

    virtual void Shift();
};


#endif