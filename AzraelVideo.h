///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AzraelVideo.h
//
// Author:      David Borland
//
// Description: Adds some information about alignment.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef AZRAELVIDEO_H
#define AZRAELVIDEO_H


#include <VideoFile.h>

#include "AzraelImage.h"


class AzraelVideo : public VideoFile {
public:
    AzraelVideo();

    void SetAlignType(AzraelImage::AlignType align);
    void SetAlignBottom(bool align);

    AzraelImage::AlignType GetAlignType() const;
    bool GetAlignBottom() const;

    void SetDontScale(bool flag);
    bool DontScale() const;

private:
    AzraelImage::AlignType alignType;
    bool alignBottom;

    bool dontScale;
};


#endif