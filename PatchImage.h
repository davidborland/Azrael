///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        PatchImage.h
//
// Author:      David Borland
//
// Description: Images of patches for victim.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef PATCHIMAGE_H
#define PATCHIMAGE_H

#include "AzraelImage.h"


class PatchImage : public AzraelImage {
public:    
    PatchImage();

    virtual void UpdateDistance(float distance);

    virtual bool Rendered();
    virtual void SetRendered();

private:
    bool rendered;
};


#endif