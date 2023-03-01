///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Azrael.h
//
// Author:      David Borland
//
// Description: wxWidgets code for Azrael, a media installation developed with RENCI Faculty
//              Fellow Joyce Rudinsky for deployment in the Social Computing Room.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#ifndef AZRAEL_H
#define AZRAEL_H


#include <wx/wx.h>

#include <GL/glew.h>        // Must be included before glcanvas.h
#include <wx/glcanvas.h>

#include "Engine.h"


// Forward declarations
class AzraelGLCanvas;


// Event Ids
enum {
    // Timers
    RenderTimerId,
    TriggerTimerId,
    StateTimerId,
    ViolenceTimerId
};


// Define a new application type
class Azrael : public wxApp {
public:
    bool OnInit();
};


// Define a new frame type
class AzraelFrame : public wxFrame {
public:
    AzraelFrame(const wxString& title, const wxSize& size);
    ~AzraelFrame();

    bool Initialize();

    void OnTimer(wxTimerEvent& e);

private:
    AzraelGLCanvas* canvas1;
    AzraelGLCanvas* canvas2;
    wxGLContext* context;

    wxTimer* renderTimer;
    wxTimer* triggerTimer;
    wxTimer* stateTimer;
    wxTimer* violenceTimer;

    wxLogWindow* log;

    wxTextCtrl* text;
    wxStreamToTextRedirector* redirect;

    Engine* engine;

    void Render();
    
    DECLARE_EVENT_TABLE()
};


// Define a new OpenGL canvas type
class AzraelGLCanvas : public wxGLCanvas {
public:
    AzraelGLCanvas(wxWindow *parent, int* attribList, const wxPoint& pos, const wxSize& size);

    void OnPaint(wxPaintEvent& e);
    void OnEraseBackground(wxEraseEvent& e);
    void OnKey(wxKeyEvent& e);

private:
    DECLARE_EVENT_TABLE()
};


#endif