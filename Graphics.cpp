///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Graphics.cpp
//
// Author:      David Borland
//
// Description: Draws the graphics for Azrael.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "Graphics.h"

#include <GLSLShader.h>

#include <IL/il.h>
#include <IL/ilu.h>


Graphics::Graphics() {
    viewWidth = viewHeight = 1.0;
}

Graphics::~Graphics() {
    // Clean up
    glDeleteTextures(1, &backgroundLeft);
    glDeleteTextures(1, &backgroundRight);

    glDeleteProgram(fadeFragmentProgram);
    glDeleteProgram(horizontalBlurFragmentProgram);
    glDeleteProgram(verticalBlurFragmentProgram);
}


bool Graphics::Initialize(int windowWidth, int windowHeight, 
                          std::vector<AzraelImage*>* azraelImagery,
                          std::vector<AzraelImage*>* azraelAvatars,
                          AzraelImage** azraelViolentImage) {
    // Set size of the screen
    viewWidth = (float)windowWidth / (float)windowHeight;
    viewHeight = 1.0;

    // Copy the pointers to the images
    imagery = azraelImagery;
    avatars = azraelAvatars;
    violentImage = azraelViolentImage;

    // Initialize DevIL
    ilInit();

    // Initialize OpenGL
    if (!InitGL()) {
        wxLogMessage("Graphics::Initialize() : OpenGL initialization failed.");
        return false;
    }

    return true;
}


void Graphics::RenderLeft() {
    // Set projection to left half of window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, viewWidth / 2.0, 0.0, 1.0, -1.0, 1.0);


    // Draw background
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundLeft);

    // Flip the y texture coordinates, because ilFlipImage() isn't working correctly
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(0.0, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex2f(viewWidth * 0.5, 0.0);

        glTexCoord2f(1.0, 0.0);
        glVertex2f(viewWidth * 0.5, viewHeight);

        glTexCoord2f(0.0, 0.0);
        glVertex2f(0.0, viewHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);


    // Draw images
    Render();
}

void Graphics::RenderRight() {
    // Set projection to right half of window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(viewWidth / 2.0, viewWidth, 0.0, 1.0, -1.0, 1.0);


    // Draw background
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundRight);

    // Flip the y texture coordinates, because ilFlipImage() isn't working correctly
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(viewWidth * 0.5, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex2f(viewWidth, 0.0);

        glTexCoord2f(1.0, 0.0);
        glVertex2f(viewWidth, viewHeight);

        glTexCoord2f(0.0, 0.0);
        glVertex2f(viewWidth * 0.5, viewHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);


    // Draw images
    Render();
}


float Graphics::GetViewWidth() const {
    return viewWidth;
}

float Graphics::GetViewHeight() const {
    return viewHeight;
}

/*
void Graphics::SetNormalMode() {
    setToVictimize = false;
    victimize = false;
}

void Graphics::SetVictimMode() {
    setToVictimize = true;
}
*/

GLint Graphics::GetFadeFragmentProgram() const {
    return fadeFragmentProgram;
}

GLhandleARB Graphics::GetOpacityParameter() const {
    return opacityParameter;
}

GLhandleARB Graphics::GetShiftParameter() const {
    return shiftParameter;
}


GLint Graphics::GetHorizontalBlurFragmentProgram() const {
    return horizontalBlurFragmentProgram;
}

GLhandleARB Graphics::GetHorizontalBlurParameter() const {
    return horizontalBlurParameter;
}


GLint Graphics::GetVerticalBlurFragmentProgram() const {
    return verticalBlurFragmentProgram;
}

GLhandleARB Graphics::GetVerticalBlurParameter() const {
    return verticalBlurParameter;
}


bool Graphics::InitGL() {
    // Initialize Glew for checking OpenGL extensions.
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        wxLogMessage("Graphics::InitGL() : %d", glewGetErrorString(err));
        return false;
    }

    // Check extensions   
    if (!GLEW_ARB_texture_rectangle) {
        wxLogMessage("Graphics::InitGL() : GL_ARB_texture_rectangle not supported on this graphics card.");
        return false;
    }
    if (!GLEW_ARB_pixel_buffer_object) {
        wxLogMessage("Graphics::InitGL() : GL_ARB_pixel_buffer_object not supported on this graphics card.");
        return false;
    }
    if (!GLEW_EXT_framebuffer_object) {
        wxLogMessage("Graphics::InitGL() : GL_EXT_framebuffer_object not supported on this graphics card.");
        return false;
    }
    if (!GLEW_ARB_shading_language_100) {
        wxLogMessage("Graphics::InitGL() : GL_ARB_shading_language not supported on this graphics card.");
        return false;
    } 


    // Load the fragment shaders
    GLSLShader shader;
    
    std::string fileName = "fade.glsl";
    if (!shader.LoadShader(fileName, fadeFragmentProgram)) {        
        wxLogMessage("Graphics::InitGL() : Could not open fragment program %s", fileName.c_str());
        return false;
    }
    opacityParameter = glGetUniformLocationARB(fadeFragmentProgram, "opacity");
    shiftParameter = glGetUniformLocationARB(fadeFragmentProgram, "shift");

    fileName = "horizontalBlur.glsl";
    if (!shader.LoadShader(fileName, horizontalBlurFragmentProgram)) {        
        wxLogMessage("Graphics::InitGL() : Could not open fragment program %s", fileName.c_str());
        return false;
    }
    horizontalBlurParameter = glGetUniformLocationARB(horizontalBlurFragmentProgram, "kernelRadius");

    fileName = "verticalBlur.glsl";
    if (!shader.LoadShader(fileName, verticalBlurFragmentProgram)) {        
        wxLogMessage("Graphics::InitGL() : Could not open fragment program %s", fileName.c_str());
        return false;
    }
    verticalBlurParameter = glGetUniformLocationARB(verticalBlurFragmentProgram, "kernelRadius");


    // Turn off depth testing
    glDisable(GL_DEPTH_TEST);

    // Turn off lighting
    glDisable(GL_LIGHTING);

    // Set the blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Create the background
    CreateBackground();


    return true;
}

void Graphics::Render() const {
//    DrawOverlays();

    // Draw
    for (int i = 0; i < (int)imagery->size(); i++) {
        ((*imagery)[i])->Render();
    }

    for (int i = 0; i < (int)avatars->size(); i++) {
        ((*avatars)[i])->Render();
    }

    if ((*violentImage)) (*violentImage)->Render();
}


void Graphics::DrawOverlays() const {
    // Draw quadrant overlays
    glEnable(GL_BLEND);

    float qWidth = viewWidth * 0.25;
    float opacity = 0.05f;

    glColor4f(1.0, 0.0, 0.0, opacity);
    glBegin(GL_QUADS);
        glVertex2f(0.0, 0.0);
        glVertex2f(qWidth * 0.5, 0.0);
        glVertex2f(qWidth * 0.5, viewHeight);
        glVertex2f(0.0, viewHeight);

        glVertex2f(qWidth * 3.5, 0.0);
        glVertex2f(qWidth * 3.5 + qWidth * 0.5, 0.0);
        glVertex2f(qWidth * 3.5 + qWidth * 0.5,viewHeight);
        glVertex2f(qWidth * 3.5, viewHeight);
    glEnd();

    glColor4f(0.0, 1.0, 0.0, opacity);
    glBegin(GL_QUADS);
        glVertex2f(qWidth * 0.5, 0.0);
        glVertex2f(qWidth * 0.5 + qWidth, 0.0);
        glVertex2f(qWidth * 0.5 + qWidth, viewHeight);
        glVertex2f(qWidth * 0.5, viewHeight);
    glEnd();

    glColor4f(0.0, 0.0, 1.0, opacity);
    glBegin(GL_QUADS);
        glVertex2f(qWidth * 1.5, 0.0);
        glVertex2f(qWidth * 1.5 + qWidth, 0.0);
        glVertex2f(qWidth * 1.5 + qWidth, viewHeight);
        glVertex2f(qWidth * 1.5, viewHeight);
    glEnd();

    glColor4f(1.0, 1.0, 0.0, opacity);
    glBegin(GL_QUADS);
        glVertex2f(qWidth * 2.5, 0.0);
        glVertex2f(qWidth * 2.5 + qWidth, 0.0);
        glVertex2f(qWidth * 2.5 + qWidth, viewHeight);
        glVertex2f(qWidth * 2.5, viewHeight);
    glEnd();
}


void Graphics::CreateBackground() {    
    // Load background images
    std::string fileName = "Media/Images/bkg_scroll_3_left.jpg";
    wxLogMessage("Loading %s", fileName.c_str());
    LoadTexture(backgroundLeft, fileName);

    fileName = "Media/Images/bkg_scroll_3_right.jpg";
    wxLogMessage("Loading %s", fileName.c_str());
    LoadTexture(backgroundRight, fileName);

    wxLogMessage("");
}

void Graphics::LoadTexture(GLuint& texture, const std::string& fileName) {
    ILuint image;
    ilGenImages(1, &image);

    ilBindImage(image);
    ilLoadImage(fileName.c_str());
    int width = ilGetInteger(IL_IMAGE_WIDTH);
    int height = ilGetInteger(IL_IMAGE_HEIGHT);

    ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ilGetData());

    ilDeleteImages(1, &image);
}