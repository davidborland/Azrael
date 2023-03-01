///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Azrael.cpp
//
// Author:      David Borland
//
// Description: wxWidgets code for Azrael, a media installation developed with RENCI Faculty
//              Fellow Joyce Rudinsky for deployment in the Social Computing Room.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "Azrael.h"

#include <wx/textctrl.h>

#include <string>


/////////////////////////////////////////////////////////////////////////////////////////////
// Azrael
/////////////////////////////////////////////////////////////////////////////////////////////


IMPLEMENT_APP(Azrael)


bool Azrael::OnInit() {
    // Create the main frame window
    AzraelFrame* frame = new AzraelFrame("Azrael", wxSize(12288, 768));
//AzraelFrame* frame = new AzraelFrame("Azrael", wxSize(3840, (float)(3840 * 768) / (float)12288));

    // Show it.  Frames, unlike simple controls, are not shown initially when created.
    frame->Show();

    // Tell wxWidgets that this is a main frame
    SetTopWindow(frame);

    // Check initialization
    if (!frame->Initialize()) {
        wxLogMessage("Initialization failed.  Exiting...");
//        return false;
    }

    // Success:  wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// AzraelFrame
/////////////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(AzraelFrame, wxFrame)
    EVT_TIMER(RenderTimerId, AzraelFrame::OnTimer)
    EVT_TIMER(TriggerTimerId, AzraelFrame::OnTimer)
    EVT_TIMER(StateTimerId, AzraelFrame::OnTimer)
    EVT_TIMER(ViolenceTimerId, AzraelFrame::OnTimer)
END_EVENT_TABLE()


AzraelFrame::AzraelFrame(const wxString& title, const wxSize& size) 
: wxFrame((wxFrame*) NULL, wxID_ANY, title, wxPoint(0, 0), size, wxBORDER_NONE | wxSYSTEM_MENU) {
    // Create a log window for printing messages
    log = new wxLogWindow(this, "Log Window", true, false);

    // Not sure how to completely hide the cursor, but this makes it a single black pixel
    SetCursor(wxImage(1, 1));

    // Set the OpenGL attributes for the canvases
    int attribList[] = { WX_GL_RGBA,
                         WX_GL_DOUBLEBUFFER,
                         0 };

/*
text = new wxTextCtrl(this, wxID_ANY, "", wxPoint(0, 0), wxSize(size.GetX() / 2, size.GetY() / 2));
redirect = new wxStreamToTextRedirector(text);
*/

    // Create two OpenGL canvases
    int width = size.GetWidth() / 2;
    int height = size.GetHeight();
    canvas1 = new AzraelGLCanvas(this, attribList, wxPoint(0, 0), wxSize(width, height));
    canvas2 = new AzraelGLCanvas(this, attribList, wxPoint(width, 0), wxSize(width, height));


    context = new wxGLContext(canvas1);
    

    // Create a timer for rendering
    renderTimer = new wxTimer(this, RenderTimerId);

    // Create a timer for triggering events
    triggerTimer = new wxTimer(this, TriggerTimerId);

    // Create a timer for triggering changes in state
    stateTimer = new wxTimer(this, StateTimerId);  

    // Createa a timer for loading a violent image
    violenceTimer = new wxTimer(this, ViolenceTimerId);


    // Seed the random number generator
    srand(time(NULL));


    // Create the engine
    engine = new Engine();
}

AzraelFrame::~AzraelFrame() {
    delete engine;
    delete context;
}


bool AzraelFrame::Initialize() { 
    context->SetCurrent(*canvas1);

    int width, height;
    GetClientSize(&width, &height);
    if (!engine->Initialize((HWND)this->GetHandle(), width, height)) {
        wxLogMessage("Engine::Initialize() : Engine initialization failed.");
        return false;
    }

    renderTimer->Start(10);
    triggerTimer->Start(20 * 1000);

    int minStart = 8 * 60;
    int maxStart = 12 * 60;
    int start = minStart + rand() % (maxStart - minStart);
    stateTimer->Start(start * 1000, wxTIMER_ONE_SHOT);
//stateTimer->Start(30 * 1000, wxTIMER_ONE_SHOT);

    // Flip a coin for violence
    if (rand() % 2 == 0) {
        violenceTimer->Start((rand() % start) * 1000, wxTIMER_ONE_SHOT);
    }
//    violenceTimer->Start(30 * 1000, wxTIMER_ONE_SHOT);

    return true;
}


void AzraelFrame::OnTimer(wxTimerEvent& e) {
    if (e.GetId() == RenderTimerId) {
        context->SetCurrent(*canvas1);

        engine->Update();

        Render();
    }
    else if (e.GetId() == TriggerTimerId) {
        engine->Trigger();
    }
    else if (e.GetId() == StateTimerId) {
        if (engine->GetState() == Engine::Normal) {
            engine->Victimize();

            stateTimer->Start(60 * 1000, wxTIMER_ONE_SHOT);
//stateTimer->Start(10 * 1000, wxTIMER_ONE_SHOT);
        }
        else if (engine->GetState() == Engine::Victimizing) {
            engine->CoolDown1();

            stateTimer->Start(10 * 1000, wxTIMER_ONE_SHOT);
//stateTimer->Start(10 * 1000, wxTIMER_ONE_SHOT);
        }
        else if (engine->GetState() == Engine::CoolingDown1) {
            engine->CoolDown2();

            stateTimer->Start(20 * 1000, wxTIMER_ONE_SHOT);
//stateTimer->Start(10 * 1000, wxTIMER_ONE_SHOT);
        }
        else if (engine->GetState() == Engine::CoolingDown2) {
            engine->Reset();

            int minStart = 8 * 60;
            int maxStart = 12 * 60;
            int start = minStart + rand() % (maxStart - minStart);
            stateTimer->Start(start * 1000, wxTIMER_ONE_SHOT);
//stateTimer->Start(30 * 1000);

            // Flip a coin for violence
            if (rand() % 2 == 0) {
                violenceTimer->Start((rand() % start) * 1000, wxTIMER_ONE_SHOT);
            }
        }
    }
    else if (e.GetId() == ViolenceTimerId) {
        engine->DoViolence();
                
        // Apparently, the oneShot parameter does nothing, so stop it here
        violenceTimer->Stop();
    }
}


void AzraelFrame::Render() {
    context->SetCurrent(*canvas1);
    engine->RenderLeft();

    context->SetCurrent(*canvas2);
    engine->RenderRight();

    // XXX : Is this is the cause of the slow down/jumpiness...
    canvas1->SwapBuffers();
    canvas2->SwapBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// AzraelGLCanvas
/////////////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(AzraelGLCanvas, wxGLCanvas)
    EVT_PAINT(AzraelGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(AzraelGLCanvas::OnEraseBackground)
    EVT_CHAR(AzraelGLCanvas::OnKey)
END_EVENT_TABLE()


AzraelGLCanvas::AzraelGLCanvas(wxWindow* parent, int* attribList, const wxPoint& pos, const wxSize& size)
: wxGLCanvas(parent, wxID_ANY, attribList, pos, size) {
}


void AzraelGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(e)) { 
    wxPaintDC(this);
}

void AzraelGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(e)) {
    // Do nothing, to avoid flashing.
}

void AzraelGLCanvas::OnKey(wxKeyEvent& e) {
    int c = e.GetKeyCode();
    if (c == 'q') {
        // Quit
        exit(0);
    }
}