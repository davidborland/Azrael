///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AzraelVideo.cpp
//
// Author:      David Borland
//
// Description: Adds some information about alignment.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "AzraelVideo.h"


AzraelVideo::AzraelVideo() : VideoFile() {
    alignType = AzraelImage::None;
    alignBottom = false;

    dontScale = false;
}


void AzraelVideo::SetAlignType(AzraelImage::AlignType align) {
    alignType = align;
}

void AzraelVideo::SetAlignBottom(bool align) {
    alignBottom = align;
}


AzraelImage::AlignType AzraelVideo::GetAlignType() const {
    return alignType;
}

bool AzraelVideo::GetAlignBottom() const {
    return alignBottom;
}


void AzraelVideo::SetDontScale(bool flag) {
    dontScale = flag;
}

bool AzraelVideo::DontScale() const {
    return dontScale;
}