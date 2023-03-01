///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AzraelImage.cpp
//
// Author:      David Borland
//
// Description: Can move around the screen, and perform different effects.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include <wx/log.h>


#include "AzraelImage.h"


const unsigned int AzraelImage::maxBlurRadius = 16;

   
AzraelImage::AzraelImage() : ToroidalImage() {
    desiredScale = 1.0;

    scaleDirection = 0;

    fade = false;
    fadeOut = false;
    opacity = 1.0;
    float fadeOpacityMin = 0.2f;
    float fadeOpacityMax = 0.6f;
    fadeOpacity = (float)rand() / (float)RAND_MAX * (fadeOpacityMax - fadeOpacityMin) + fadeOpacityMin;

    shiftAmount = 0;

    quadrant = -1;

    timer = timerMax = 0;

    blurRadius = maxBlurRadius;
    actualBlurRadius = maxBlurRadius;

    alignType = None;
    alignBottom = false;

    dontScale = false;
}

AzraelImage::~AzraelImage() {
    glDeleteFramebuffersEXT(1, &fbo);
    glDeleteTextures(1, &tempBlurTexture);
    glDeleteTextures(1, &finalBlurTexture);
}


void AzraelImage::SetTexture(GLuint textureMap, unsigned int width, unsigned int height, PixelFormat type) {
    Image::SetTexture(textureMap, width, height, pixelFormat);

    CreateBlurTextures();
}


void AzraelImage::Update() {
    // Timer
    if (timerMax > 0) {
        timer++;
    }

    // Update position
    Vec2 v = desiredPosition - position;
    position += v * 0.1;


    // Update scale
    float oldScale = scale;
    const float scaleStep = 0.01f;

    if (scaleDirection == -1) {
        if (scale > desiredScale) {
            scale -= scaleStep;
        }
        else {
            scale = desiredScale;

            scaleDirection = 0;
        }
    }
    else if (scaleDirection == 1) {
        if (scale < desiredScale) {
            scale += scaleStep;
        }
        else {
            scale = desiredScale;

            scaleDirection = 0;
        }
    }

    // Might need to keep aligned if scale changed
    if (alignType != None) {
        float offset = aspectRatio * 0.5 * (oldScale - scale);
        if (alignType == Left) position.X() -= offset;
        else if (alignType == Right) position.X() += offset;

        desiredPosition.X() = position.X();
    }
    if (alignBottom) {
        position.Y() -= 0.5 * (oldScale - scale);

        desiredPosition.Y() = position.Y();
    }


    // Update actual blur radius
    if (actualBlurRadius < blurRadius) actualBlurRadius++;
    else if (actualBlurRadius > blurRadius) actualBlurRadius--;


    // Fade
    if (fade) {
        float fadeStep = 0.01f;
        if (opacity > fadeOpacity) opacity -= fadeStep;
        else opacity = fadeOpacity;
    }
    else if (fadeOut) {
        float fadeStep = 0.01f;
        if (opacity > 0.0) opacity -= fadeStep;
    }
}


void AzraelImage::SetDesiredPosition(const Vec2& desiredValue) {
    desiredPosition = desiredValue;
}

void AzraelImage::SetDesiredScale(float desiredValue) {
    if (!dontScale) {
        desiredScale = desiredValue;

        if (scale < desiredScale) scaleDirection = 1;
        else if (scale > desiredScale) scaleDirection = -1;
        else scaleDirection = 0;
    }
    else {
        scale = desiredScale = 1.0;
    }
}


void AzraelImage::SetDontScale(bool flag) {
    dontScale = flag;
}


void AzraelImage::GeneratePosition() {
    if (quadrant < 0) {
        // Put anywhere in X
        position.X() = (float)rand() / (float)RAND_MAX * (xMax - xMin);

        // Constrain so all is showing in Y
        float yPlay = 1.0 - scale;
        float yOffset = ((float)rand() / (float)RAND_MAX - 0.5) * yPlay;
        position.Y() = 0.5 + yOffset;
    }
    else {
        // Put in the given quadrant
        float quadrantWidth = (xMax - xMin) * 0.25;

        // X
        if (alignType == Left) {
            position.X() = quadrant * quadrantWidth + aspectRatio * 0.5 * scale;
        }
        else if (alignType == Right) {
            float q = quadrant == 0 ? 4 : quadrant;
            position.X() = q * quadrantWidth - aspectRatio * 0.5 * scale;
        }
        else {     
            // Generate a random number in the range 0...1
            float value = (float)rand() / (float)RAND_MAX;

            // Offset to the correct quadrant
            if (quadrant == 0) {
                // Quadrant 0 wraps around the screen
                if (value < 0.5) {
                    position.X() = value * quadrantWidth;
                }
                else {
                    position.X() = 3.5 * quadrantWidth + (1.0 - value) * quadrantWidth;
                }
            }
            else {
                position.X() = (quadrant - 0.5) * quadrantWidth + value * quadrantWidth;
            }
        }


        // Y
        if (alignBottom) {
            position.Y() = 0.5 * scale;
        }
        else {
            float yPlay = 1.0 - scale;
            float yOffset = ((float)rand() / (float)RAND_MAX - 0.5) * yPlay;
            position.Y() = 0.5 + yOffset;
        }
    }

    desiredPosition = position;
}


void AzraelImage::Fade() {
    fade = true;
    fadeOut = false;
}

void AzraelImage::FadeOut() {
    fadeOut = true;
    fade = false;
}


bool AzraelImage::FadedOut() {
    if (opacity <= 0.0) {
        return true;
    }
    else {
        return false;
    }
}

bool AzraelImage::Rendered() {
    return true;
}

void AzraelImage::SetRendered() {
}

void AzraelImage::Shift() {
    shiftAmount = rand() % 16 + 5;
}

void AzraelImage::NoShift() {
    shiftAmount = 0;
}


void AzraelImage::SetAlignType(AlignType align) {
    alignType = align;
}

AzraelImage::AlignType AzraelImage::GetAlignType() const {
    return alignType;
}


void AzraelImage::SetAlignBottom(bool align) {
    alignBottom = align;
}

bool AzraelImage::AlignBottom() const {
    return alignBottom;
}


void AzraelImage::SetQuadrant(int whichQuadrant) {
    quadrant = whichQuadrant;
}

int AzraelImage::GetQuadrant() const {
    return quadrant;
}



bool AzraelImage::DeleteOnNewQuadrant() {
    return false;
}


void AzraelImage::SetTimer() {
    int min = 25;
    int max = 50;
    timerMax = rand() % (max - min) + min;
}

bool AzraelImage::TimedOut() {
    return timer > timerMax;
}


void AzraelImage::SetFadeFragmentProgram(GLhandleARB fragmentProgram, GLint parameter1, GLint parameter2) {
    fadeFragmentProgram = fragmentProgram;
    opacityParameter = parameter1;
    shiftParameter = parameter2;
}


void AzraelImage::SetBlurFragmentPrograms(GLhandleARB horizontalFragmentProgram, GLint horizontalParameter,
                                          GLhandleARB verticalFragmentProgram, GLint verticalParameter) {
    horizontalBlurFragmentProgram = horizontalFragmentProgram;
    horizontalBlurParameter = horizontalParameter;
    verticalBlurFragmentProgram = verticalFragmentProgram;
    verticalBlurParameter = verticalParameter;
}


void AzraelImage::PreRender() {
    // Enable blending
    glEnable(GL_BLEND);
    glColor4f(1.0, 1.0, 1.0, 1.0);


    // Enable texturing
//    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    // Bind the texture
    GLint useTexture = texture;
    if (actualBlurRadius > 0) {
        DoBlur();
        useTexture = finalBlurTexture;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, useTexture);


    // Fade fragment program
    glUseProgramObjectARB(fadeFragmentProgram);
    glUniform1fARB(opacityParameter, (GLfloat)opacity);
    glUniform1iARB(shiftParameter, (GLint)shiftAmount);
}


void AzraelImage::PostRender() {
    // Fragment program
    glUseProgramObjectARB(0);

    // Disable texturing
    glDisable(GL_TEXTURE_RECTANGLE_ARB);

    // Disable blending
    glDisable(GL_BLEND);
}


void AzraelImage::DoBlur() {
    // Setup for both blur passes
    glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    glViewport(0, 0, resolution[0], resolution[1]);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);


    // Horizontal blur
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);

    glUseProgramObjectARB(horizontalBlurFragmentProgram);
    glUniform1iARB(horizontalBlurParameter, actualBlurRadius);
    
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex2f(0.0, 0.0);

        glTexCoord2d(resolution[0], 0);
        glVertex2f(1.0, 0.0);

        glTexCoord2d(resolution[0], resolution[1]);
        glVertex2f(1.0, 1.0);

        glTexCoord2d(0, resolution[1]);
        glVertex2f(0.0, 1.0);
    glEnd();


    // Vertical blur
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tempBlurTexture);

    glUseProgramObjectARB(verticalBlurFragmentProgram);
    glUniform1iARB(verticalBlurParameter, actualBlurRadius);
    
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex2f(0.0, 0.0);

        glTexCoord2d(resolution[0], 0);
        glVertex2f(1.0, 0.0);

        glTexCoord2d(resolution[0], resolution[1]);
        glVertex2f(1.0, 1.0);

        glTexCoord2d(0, resolution[1]);
        glVertex2f(0.0, 1.0);
    glEnd();


    // Restore state
    glPopMatrix();

    glPopAttrib();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glUseProgramObjectARB(0);
}


void AzraelImage::CreateTexture() {
    Image::CreateTexture();

    CreateBlurTextures();
}


void AzraelImage::CreateBlurTextures() {
    // Create the fbo
    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    
    // Create the first texture
    glGenTextures(1, &tempBlurTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tempBlurTexture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    if (pixelFormat == LUMINANCE) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, resolution[0], resolution[1], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    }
    else if (pixelFormat == RGB || pixelFormat == BGR) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, resolution[0], resolution[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    else if (pixelFormat == RGBA || pixelFormat == BGRA) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, resolution[0], resolution[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    // Attach the texture to the fbo
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tempBlurTexture, 0);


    // Create the second texture
    glGenTextures(1, &finalBlurTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, finalBlurTexture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    if (pixelFormat == LUMINANCE) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, resolution[0], resolution[1], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    }
    else if (pixelFormat == RGB || pixelFormat == BGR) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, resolution[0], resolution[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    else if (pixelFormat == RGBA || pixelFormat == BGRA) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, resolution[0], resolution[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    // Attach the texture to the fbo
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, finalBlurTexture, 0);


    // Unbind the fbo
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void AzraelImage::CleanUp() {
    Image::CleanUp();

    glDeleteBuffersARB(1, &fbo);
    glDeleteTextures(1, &tempBlurTexture);
    glDeleteTextures(1, &finalBlurTexture);
}
