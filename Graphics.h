///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Graphics.h
//
// Author:      David Borland
//
// Description: Draws the graphics for Azrael.
//
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <wx/log.h>     // This must be included before Video.h

#include <vector>

#include "AzraelImage.h"
#include "AvatarImage.h"
#include "ViolentImage.h"


class Graphics {
public:
    Graphics();
    ~Graphics();

    bool Initialize(int windowWidth, int windowHeight, 
                    std::vector<AzraelImage*>* azraelImages,
                    std::vector<AzraelImage*>* azraelAvatars,
                    AzraelImage** violentImage);

    void RenderLeft();
    void RenderRight();

    float GetViewWidth() const;
    float GetViewHeight() const;

    GLint GetFadeFragmentProgram() const;
    GLhandleARB GetOpacityParameter() const;
    GLhandleARB GetShiftParameter() const;

    GLint GetHorizontalBlurFragmentProgram() const;
    GLhandleARB GetHorizontalBlurParameter() const;

    GLint GetVerticalBlurFragmentProgram() const;
    GLhandleARB GetVerticalBlurParameter() const;

private:
    std::vector<AzraelImage*>* imagery;
    std::vector<AzraelImage*>* avatars;
    AzraelImage** violentImage;

    float viewWidth;
    float viewHeight;

    GLuint backgroundLeft;
    GLuint backgroundRight;

    GLhandleARB fadeFragmentProgram;
    GLint opacityParameter;
    GLint shiftParameter;

    GLhandleARB horizontalBlurFragmentProgram;
    GLint horizontalBlurParameter;

    GLhandleARB verticalBlurFragmentProgram;
    GLint verticalBlurParameter;

    bool InitGL();
    void Render() const;
    void DrawOverlays() const;

    void CreateBackground();
    void LoadTexture(GLuint& texture, const std::string& fileName);
};


#endif