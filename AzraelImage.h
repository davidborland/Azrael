///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AzraelImage.h
//
// Author:      David Borland
//
// Description: Abstract class for moving around the screen and perform different effects
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef AZRAELIMAGE_H
#define AZRAELIMAGE_H


#include <ToroidalImage.h>

#include <VideoFile.h>


class AzraelImage : public ToroidalImage {
public:    
    AzraelImage();
    virtual ~AzraelImage();

    virtual void SetTexture(GLuint textureMap, unsigned int width, unsigned int height, PixelFormat type);

    virtual void Update();
    virtual void UpdateDistance(float distance) = 0;

    void SetDesiredPosition(const Vec2& desiredValue);
    void SetDesiredScale(float desiredValue);

    void SetDontScale(bool flag);

    virtual void GeneratePosition();

    void Fade();
    void FadeOut();
    bool FadedOut();

    virtual bool Rendered();
    virtual void SetRendered();

    virtual void Shift();
    virtual void NoShift();

    enum AlignType {
        None,
        Left,
        Right
    };

    void SetAlignType(AlignType align);
    AlignType GetAlignType() const;

    void SetAlignBottom(bool align);
    bool AlignBottom() const;

    void SetQuadrant(int whichQuadrant);
    int GetQuadrant() const;

    virtual bool DeleteOnNewQuadrant();

    void SetTimer();
    bool TimedOut();

    void SetFadeFragmentProgram(GLhandleARB fragmentProgram, GLint parameter1, GLint parameter2);
    void SetBlurFragmentPrograms(GLhandleARB horizontalFragmentProgram, GLint horizontalParameter,
                                 GLhandleARB verticalFragmentProgram, GLint verticalParameter); 

protected:
    // Decoupling from actual values for smoother animation
    Vec2 desiredPosition;
    float desiredScale;

    int scaleDirection;

    bool fade;
    bool fadeOut;
    float opacity;
    float fadeOpacity;

    int shiftAmount;

    AlignType alignType;
    bool alignBottom;

    bool dontScale;

    int quadrant;

    int timer;
    int timerMax;

    unsigned int blurRadius;
    unsigned int actualBlurRadius;

    const static unsigned int maxBlurRadius;

    GLhandleARB fadeFragmentProgram;
    GLint opacityParameter;
    GLint shiftParameter;

    GLhandleARB horizontalBlurFragmentProgram;
    GLint horizontalBlurParameter;

    GLhandleARB verticalBlurFragmentProgram;
    GLint verticalBlurParameter;

    GLuint fbo;
    GLuint tempBlurTexture;
    GLuint finalBlurTexture;

    virtual void PreRender();
    virtual void PostRender();
    void DoBlur(); 

    virtual void CreateTexture();
    void CreateBlurTextures();    
    virtual void CleanUp();
};


#endif