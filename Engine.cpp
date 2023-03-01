///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Engine.cpp
//
// Author:      David Borland
//
// Description: Main engine of Azrael.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#include "Engine.h"

#include "QuadrantImage.h"
#include "FragmentImage.h"
#include "GuardImage.h"
#include "ViolentImage.h"
#include "PatchImage.h"

#include <VideoStream.h>

#include <fstream>
#include <time.h>


Engine::Engine() {
    graphics = new Graphics();
    tracking = new Tracking();

    projectorShutter = new ProjectorShutter();
    posiTrack = new PosiTrack();

    violentImage = NULL;
    violentConnection = NULL;

    ambientSound = NULL;
    victimRoomSound = NULL;
    victimCenterSound = NULL;

    numberOfOldImages = 0;

    currentLongSound = -1;
}

Engine::~Engine() {
    // Delete graphics and tracking
    delete graphics;
    delete tracking;

    delete projectorShutter;
    delete posiTrack;

    
    // Delete what's currently being shown
    for (int i = 0; i < (int)imagery.size(); i++) {
        delete imagery[i];
    }

    for (int i = 0; i < (int)avatars.size(); i++) {
        delete avatars[i];
    }

    if (violentImage) delete violentImage;
    if (violentConnection) delete violentConnection;


    // Delete imagery
    for (int i = 0; i < (int)quadrantImages.size(); i++) {
        ilDeleteImages(1, &quadrantImages[i]);
    }

    for (int i = 0; i < (int)quadrantVideos.size(); i++) {
        delete quadrantVideos[i];
    }

    for (int i = 0; i < (int)timelineVideos.size(); i++) {
        delete timelineVideos[i];
    }

    for (int i = 0; i < (int)avatarImages.size(); i++) {
        ilDeleteImages(1, &avatarImages[i]);
    }

    for (int i = 0; i < (int)fragmentTextures.size(); i++) {
        glDeleteTextures(1, &fragmentTextures[i].texture);
    }

    for (int i = 0; i < (int)patchTextures.size(); i++) {
        glDeleteTextures(1, &patchTextures[i].texture);
    }

    for (int i = 0; i < (int)guardVideos.size(); i++) {
        delete guardVideos[i];
    }

    for (int i = 0; i < (int)violentVideos.size(); i++) {
        delete violentVideos[i];
    }


    // Delete sounds
    if (ambientSound) delete ambientSound;
    if (victimRoomSound) delete victimRoomSound;
    if (victimCenterSound) delete victimCenterSound;

    for (int i = 0; i < (int)longSounds.size(); i++) {
        delete longSounds[i];
    }

    for (int i = 0; i < (int)fragmentSounds.size(); i++) {
        delete fragmentSounds[i];
    }

    BASS_Free();
}


bool Engine::Initialize(HWND win, int windowWidth, int windowHeight) {
    // Initialize the graphics
    if (!graphics->Initialize(windowWidth, windowHeight, &imagery, &avatars, &violentImage)) {
        wxLogMessage("Engine::Initialize() : Graphics initialization failed.");
        return false;
    }

    // Initialize the tracking
    if (!tracking->Initialize(&avatars)) {
        wxLogMessage("Engine::Initialize() : Tracking initialization failed.");
        return false;
    }

    // Initialize the audio
    if (!AudioStream::InitializeLibrary(win, true)) {
        wxLogMessage("Engine::Initialize() : Audio library initialization failed.");
        return false;
    }

    // Initialize the projector shutter
    if (!projectorShutter->Initialize()) {
        wxLogMessage("Engine::Initialize() : Projector shutter initialization failed.");
//        return false;
    }

    // Initialize the PosiTrack
    if (!posiTrack->Initialize()) {
        wxLogMessage("Engine::Initialize() : PosiTrack initialization failed.");
//        return false;
    }    
    posiTrack->SetPosition(Vec3(3.25, 3.25, 2.5));
    posiTrack->SetOrientation(Vec3(1.0, 0.0, 0.0)); 

    posiTrack->SetPanAngle(180.0);
    posiTrack->SetTiltAngle(0.0);


    // Load the images
    LoadImages();

    // Load the videos
    LoadVideos();

    // Load the audio
    LoadAudio();

    // Play the ambient sound
    if (ambientSound) {
        if (!ambientSound->Initialize(true)) {
            wxLogMessage("Engine::Initialize() : Couldn't initialize ambient sound.");
            return false;
        }
    }
    else {
        wxLogMessage("Engine::Initialize() : No ambient sound.");
        return false;
    }

    // Check the victim sounds
    if (victimRoomSound) {
        if (!victimRoomSound->Initialize(true, 5)) {
            wxLogMessage("Engine::Initialize() : Couldn't initialize victim room sound.");
            return false;
        }
        victimRoomSound->SetVolume(0.5, 0.0);
    }
    else {
        wxLogMessage("Engine::Initialize() : No victim room sound.");
        return false;
    }

    if (victimCenterSound) {
        if (!victimCenterSound->Initialize(true, 6)) {
            wxLogMessage("Engine::Initialize() : Couldn't initialize victim center sound.");
            return false;
        }
    }
    else {
        wxLogMessage("Engine::Initialize() : No victim center sound.");
        return false;
    }


    // Start fresh
    if (!Reset()) return false;


    return true;
}


void Engine::Update() {
    // Update the tracking
    tracking->Update(); 

    while (tracking->GetNumberOfViewers() > (int)avatars.size()) {
        // Show new avatar
        avatars.push_back(new AvatarImage());
        ShowImage(avatarImages[rand() % (int)avatarImages.size()], avatars.back());
        avatars.back()->SetPosition(Vec2(-10.0, -10.0));
        avatars.back()->SetDesiredPosition(Vec2(-10.0, -10.0));
    }

    if (state == Normal) {
        UpdateNormal();
    }
    else if (state == Victimizing) {
        UpdateVictimize();
    } 
    else if (state == CoolingDown1) {
        UpdateCoolDown1();
    }    
    else if (state == CoolingDown2) {
        UpdateCoolDown2();
    }
}


void Engine::Trigger() {
    triggerCount++;
}

void Engine::Victimize() {
    wxLogMessage("Victimizing!");

    // Move avatars
    for (int i = 0; i < (int)avatars.size(); i++) {
        avatars[i]->SetPosition(Vec2(-10.0, -10.0));
    }

    // Get rid of violence
    if (violentConnection) {
        delete violentConnection;
        delete violentImage;
        violentConnection = NULL;
        violentImage = NULL;
    }

    // Fade everything out
    for (int i = 0; i < (int)imagery.size(); i++) {
        if (imagery[i]->DeleteOnNewQuadrant()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
        }
        else {
            imagery[i]->Fade();
            if (imagery[i]->GetScale() == 0.8) {
                imagery[i]->SetDesiredScale(0.6f);
            }
            else {
                imagery[i]->SetDesiredScale(0.75);
            }
        }
    }

    // Stop current videos
    for (int i = 0; i < (int)connections.size(); i++) {
        connections[i].GetCurrentVideo()->Stop();
    }
    connections.clear();


    // No static
    for (int i = 0; i < (int)imagery.size(); i++) {
        imagery[i]->NoShift();
    }


    // Stop current audio
    longSounds[currentLongSound]->Stop();
    for (int i = 0; i < (int)fragmentSounds.size(); i++) {
        delete fragmentSounds[i];
    }
    fragmentSounds.clear();

    ambientSound->Stop();


    // Play audio over room and center speakers
    victimRoomSound->Play();
    victimRoomSound->SetVolume(0.5, 0.0);
    victimCenterSound->Play();
    posiTrack->Wiggle();
    

    // Pick a victim
    tracking->PickVictim();


    // Close the projector shutters
    projectorShutter->Close();


    // Set the state
    state = Victimizing;
}

void Engine::CoolDown1() {
    wxLogMessage("Cooling down 1");

    state = CoolingDown1;

    if (victimRoomSound) victimRoomSound->Stop();
    if (victimCenterSound) victimCenterSound->Stop();

    posiTrack->SetPanAngle(180.0);
    posiTrack->SetTiltAngle(0.0);
}

void Engine::CoolDown2() {
    wxLogMessage("Cooling down 2");

    state = CoolingDown2;

    numberOfOldImages = (int)imagery.size();
}

bool Engine::Reset() {
    wxLogMessage("Resetting.");

    // Clean up 
    if (violentImage) delete violentImage;
    if (violentConnection) delete violentConnection;
    violentImage = NULL;
    violentConnection = NULL;

    connections.clear();


    // Rewind videos
    for (int i = 0; i < (int)quadrantVideos.size(); i++) {
        quadrantVideos[i]->Rewind();
        quadrantVideos[i]->Stop();
    }

    for (int i = 0; i < (int)timelineVideos.size(); i++) {
        timelineVideos[i]->Rewind();
        timelineVideos[i]->Stop();
    }

    for (int i = 0; i < (int)guardVideos.size(); i++) {
        guardVideos[i]->Rewind();
        guardVideos[i]->Stop();
    }

    for (int i = 0; i < (int)violentVideos.size(); i++) {
        violentVideos[i]->Rewind();
        violentVideos[i]->Stop();
    }


    // Rewind sounds
    for (int i = 0; i < (int)longSounds.size(); i++) {
        longSounds[i]->Rewind();
        longSounds[i]->Stop();
    }

    // Play ambient sound
    ambientSound->Play();


    // Reset quadrant imagery to choose from
    CopyChoose();


    // Reset tracking
    tracking->Reset();


    // Set default values
    numberOfQuadrantImages = 0;
    numberOfFragmentImages = 0;

    canLoadGuard = true;

    triggerCount = 0;

    victimizeCount = 0;


    // Get a random quadrant to start with.
    // Don't use quadrant three, as this is where the entrance is.
    do {
        activeQuadrant = GenerateQuadrant();
    }
    while (activeQuadrant == 3);


    // Make sure there is an image
    Trigger();

    // Make sure there is a sound
    PlayLongAudio(QuadrantToAudioChannel(activeQuadrant));


    // Open the projector shutter
    projectorShutter->Open();


    state = Normal;


    return true;
}


void Engine::DoViolence() {
    wxLogMessage("Doing Violence.");

    // Show a violent video in a different quadrant
    violentImage = new ViolentImage();
    int quadrant;
    do {
        quadrant = GenerateQuadrant();
    }
    while (quadrant == activeQuadrant);
    violentImage->SetQuadrant(quadrant);
    PlayVideo(violentVideos[rand() % (int)violentVideos.size()], violentImage, true);
    violentConnection->GetCurrentVideo()->Play();
}


void Engine::RenderLeft() const {
    graphics->RenderLeft();
}

void Engine::RenderRight() const {
    graphics->RenderRight();
}


Engine::State Engine::GetState() {
    return state;
}


bool Engine::LoadImages() {
    std::string fileName = "Media/ImageInfo.txt";

    std::fstream file(fileName.c_str(), std::fstream::in);
    if (file.fail()) {
        wxLogMessage("Engine::LoadImages() : Couldn't open %s", fileName.c_str());
        return false;
    }

    wxLogMessage("Engine::LoadImages() : Parsing %s", fileName.c_str());

    ILuint image;
    bool loadOkay = false;
    std::string s;
    while(!file.eof()) {
        getline(file, s);

        if (s == "") {
            // Nothing
        }
        else if (s == "avatar") {
            if (loadOkay) {
                avatarImages.push_back(image);
                loadOkay = false;
            }
        }
        else if (s == "patch") {
            if (loadOkay) {
                Texture texture;
                LoadTexture(image, texture);
                patchTextures.push_back(texture);
                ilDeleteImages(1, &image);
                loadOkay = false;
            }
        }
        else if (s == "fragment") {
            if (loadOkay) {
                Texture texture;
                LoadTexture(image, texture);
                fragmentTextures.push_back(texture);
                ilDeleteImages(1, &image);
                loadOkay = false;
            }
        }
        else {
            if (loadOkay) {
                // Hasn't been added yet, so it's a quadrant image
                quadrantImages.push_back(image);
            }

            // Load new image
            wxLogMessage("Loading %s", s.c_str());
            loadOkay = LoadImage(s, image);
        }
    }
    wxLogMessage("");


    // Catch the last line
    if (loadOkay) {
        // Hasn't been added yet, so its a quadrant image
        quadrantImages.push_back(image);
    }


    file.close();

    return true;
}



bool Engine::LoadVideos() {
    std::string fileName = "Media/VideoInfo.txt";

    std::fstream file(fileName.c_str(), std::fstream::in);
    if (file.fail()) {
        wxLogMessage("Engine::LoadVideos() : Couldn't open %s", fileName.c_str());
        return false;
    }

    wxLogMessage("Engine::LoadVideos() : Parsing %s", fileName.c_str());

    AzraelVideo* video = NULL;
    std::string s;
    while(!file.eof()) {
        getline(file, s);

        if (s == "") {
            // Nothing
        }
        else if (s == "loop") {
            if (video) video->SetLoop(true);
        }        
        else if (s == "guard") {
            if (video) {
                guardVideos.push_back(video);
                video = NULL;
            }
        }
        else if (s == "violent") {
            if (video) {
                violentVideos.push_back(video);
                video = NULL;
            }
        }
        else if (s == "timeline") {
            if (video) {
                timelineVideos.push_back(video);
                video = NULL;
            }
        }
        else if (s == "alignRight") {
            if (video) video->SetAlignType(AzraelImage::Right);
        }
        else if (s == "alignLeft") {
            if (video) video->SetAlignType(AzraelImage::Left);
        }
        else if (s == "alignBottom") {
            if (video) video->SetAlignBottom(true);
        }
        else if (s == "dontScale") {
            if (video) video->SetDontScale(true);
        }
        else {
            if (video) {
                // Hasn't been added yet, so it's a quadrant video
                quadrantVideos.push_back(video);
            }

            // Load new video
            wxLogMessage("Loading %s", s.c_str());
            video = new AzraelVideo();
            video->SetName(s);
            if (!video->Initialize(VideoStream::RGBA)) {
                wxLogMessage("Engine::LoadVideos() : Video initialization failed.");

                // Remove
                delete video;
                video = NULL;
            }
        }
    }
    wxLogMessage("");


    // Catch the last line
    if (video) {
        // Hasn't been added yet, so it's a quadrant video
        quadrantVideos.push_back(video);
    }


    file.close();

    return true;
}


bool Engine::LoadAudio() {
    std::string fileName = "Media/AudioInfo.txt";

    std::fstream file(fileName.c_str(), std::fstream::in);
    if (file.fail()) {
        wxLogMessage("Engine::LoadAudio() : Couldn't open %s", fileName.c_str());
        return false;
    }

    wxLogMessage("Engine::LoadAudio() : Parsing %s", fileName.c_str());

    std::string s;
    std::string name;
    while(!file.eof()) {
        getline(file, s);

        if (s == "") {
            // Nothing
        }
        else if (s == "ambient") {
            ambientSound = new AudioStream();
            ambientSound->SetFileName(name);
            name = "";
        }                
        else if (s == "victimRoom") {
            victimRoomSound = new AudioStream();
            victimRoomSound->SetFileName(name);
            name = "";
        }
        else if (s == "victimCenter") {
            victimCenterSound = new AudioStream();
            victimCenterSound->SetFileName(name);
            name = "";
        }
        else if (s == "long") {
            wxLogMessage("Engine::LoadAudio() : Loading %s", name.c_str());
            longSounds.push_back(new AudioStream());
            longSounds.back()->SetFileName(name);
            if (!longSounds.back()->Initialize(true, 5)) {
                wxLogMessage("Engine::LoadAudio() : Couldn't load audio");
                delete longSounds.back();
                longSounds.pop_back();
            }
            name = "";
        }
        else {
            if (name != "") {
                // Hasn't been added yet, add the name to the fragment list
                fragmentSoundNames.push_back(name);
            }

            // Load new video
            name = s;
        }
    }
    wxLogMessage("");


    // Catch the last line
    if (name != "") {
        // Hasn't been added yet, so it's a fragment
        fragmentSoundNames.push_back(name);
    }


    file.close();

    return true;
}


void Engine::CopyChoose() {
    chooseQuadrantImages.clear();
    chooseQuadrantVideos.clear();
    chooseTimelineVideos.clear();

    for (int i = 0; i < (int)quadrantImages.size(); i++) {
        chooseQuadrantImages.push_back(quadrantImages[i]);
    }

    for (int i = 0; i < (int)quadrantVideos.size(); i++) {
        chooseQuadrantVideos.push_back(quadrantVideos[i]);
    }

    for (int i = 0; i < (int)timelineVideos.size(); i++) {
        chooseTimelineVideos.push_back(timelineVideos[i]);
    }
}


bool Engine::LoadImage(const std::string& fileName, ILuint& image) {
    // Load the image using DevIL
    ilGenImages(1, &image);            
    ilBindImage(image);

    if (!ilLoadImage(fileName.c_str())) {
        wxLogMessage("Engine::LoadImage() : Loading image failed");

        ilDeleteImages(1, &image);              

        return false;
    }


    // Flip the image in y
    iluFlipImage();           
    
    
    // Check the pixel format
    int ilPixelFormat = ilGetInteger(IL_IMAGE_FORMAT);

    // Check the number of components per pixel
    if (ilPixelFormat == IL_LUMINANCE) {
        ilConvertImage(IL_LUMINANCE, IL_UNSIGNED_BYTE);
    }
    else if (ilPixelFormat == IL_RGB) {
        // Convert to RGBA, as GL_TEXTURE_RECTANGLE_ARB seems to have issues loading RGB textures with 
        // dimensions that are not a multiple of four...
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    }
    else if (ilPixelFormat == IL_RGBA) {
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    }
    else {
        // Something ain't right...
        wxLogMessage("Engine::LoadImage() : Invalid number of components");

        // Remove
        ilDeleteImages(1, &image);                 
             
        return false;
    }

    return true;
}

void Engine::LoadTexture(ILuint image, Texture& texture) {
    // Set the current image
    ilBindImage(image);

    // Get image info
    int width = ilGetInteger(IL_IMAGE_WIDTH);
    int height = ilGetInteger(IL_IMAGE_HEIGHT);
    int ilPixelFormat = ilGetInteger(IL_IMAGE_FORMAT);

    // Get the pixel format
    Image::PixelFormat pixelFormat;
    if (ilPixelFormat == IL_LUMINANCE) {
        pixelFormat = Image::LUMINANCE;
    }
    else if (ilPixelFormat == IL_RGB) {
        // Already converted to IL_RGBA in LoadImages, so we should never be here
        wxLogMessage("Engine::LoadTexture() : Error, trying to load IL_RGB");
        return;
    }
    else if (ilPixelFormat == IL_RGBA) {
        pixelFormat = Image::RGBA;
    }
    else {
        // Already caught this in LoadImages, so we should never be here
        wxLogMessage("Engine::LoadTexture() : Error, trying to load unknown pixel format");
        return;
    }

    
    // Create the texture
    texture.width = width;
    texture.height = height;
    texture.pixelFormat = pixelFormat;

    glGenTextures(1, &texture.texture);    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture.texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Create the texture
    if (pixelFormat == Image::LUMINANCE) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ilGetData());
    }   
    else if (pixelFormat == Image::RGBA) {
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
    }
}


void Engine::UpdateNormal() {
    // Check for loading images
    CheckAvatarsAndGuards();
    CheckQuadrant();
    CheckFragments();

    // Check for removing images
    CheckFadedOut();


    // Quadrant, fragment, and guard image behavior based on distance       
    int numToAverage = 3;
    for (int i = 0; i < (int)imagery.size(); i++) {
        float distance = tracking->GetAverageDistance(GraphicsToWalls(imagery[i]->GetPosition().X()), numToAverage);
        imagery[i]->UpdateDistance(distance);
    }

    // Violent image behavior based on distance
    if (violentImage) {
       float distance = tracking->GetAverageDistance(GraphicsToWalls(violentImage->GetPosition().X()), 1);
       violentImage->UpdateDistance(distance);
    }   


    // Update the media
    for (int i = 0; i < (int)connections.size(); i++) {
// XXX : Why is this necessary here?  Play() is already called in PlayVideo, but doesn't always work when
//       guard and quadrant videos are loaded at the same time...        
connections[i].GetCurrentVideo()->Play();
        if (!connections[i].Update()) {
            connections.erase(connections.begin() + i);
            i--;
        }
    }

    if (violentConnection) {
violentConnection->GetCurrentVideo()->Play();
        if (!violentConnection->Update()) {
            delete violentImage;
            delete violentConnection;
            violentImage = NULL;
            violentConnection = NULL;
        }
    }

    for (int i = 0; i < (int)imagery.size(); i++) {
        imagery[i]->Update();
        if (imagery[i]->TimedOut()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
            numberOfFragmentImages--;
        }
    }

    for (int i = 0; i < (int)avatars.size(); i++) {
        avatars[i]->Update();
    }

    if (violentImage) violentImage->Update();

    for (int i = 0; i < (int)fragmentSounds.size(); i++) {
        if (fragmentSounds[i]->Stopped()) {
            delete fragmentSounds[i];
            fragmentSounds.erase(fragmentSounds.begin() + i);
            i--;
        }
    }
}

void Engine::UpdateVictimize() {
    if (tracking->GetVictim() == NULL) {
        tracking->PickVictim();
    }


    if (victimizeCount == 0) {
        if (tracking->GetVictim()) {
            // Put a patch there
            int index = rand() % (int)patchTextures.size();
            float x = WallsToGraphics(tracking->GetVictim()->ProjectToWall());
            x += ((float)rand() / (float)RAND_MAX - 0.5) * 2.0;
            float y = (float)rand() / (float)RAND_MAX;
            Vec2 position = Vec2(x, y);

            imagery.push_back(new PatchImage());
            ShowTexture(patchTextures[index], imagery.back());
            imagery.back()->SetPosition(position);
            imagery.back()->SetDesiredPosition(position);
            float scale = 300.0f / 768.0f;
            imagery.back()->SetScale(scale);
            imagery.back()->SetDesiredScale(scale);

            victimizeCount = 2;
        }
    }


    // Update the media
    for (int i = 0; i < (int)imagery.size(); i++) {
        imagery[i]->Update();
        if (imagery[i]->TimedOut()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
            numberOfFragmentImages--;
        }
    }


    // Point the PosiTrack
    if (tracking->GetVictim()) {
        Vec3 headPosition = tracking->GetVictim()->GetPosition();
        headPosition.Z() += 0.2;
        posiTrack->PointAt(headPosition);
    }

    victimizeCount--;
}

void Engine::UpdateCoolDown1() {
    // Do nothing 
}


void Engine::UpdateCoolDown2() {
    CheckFadedOut();

    // Update the media
    for (int i = 0; i < (int)imagery.size(); i++) {
        imagery[i]->Update();
        if (imagery[i]->TimedOut()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
            numberOfFragmentImages--;
        }
    }
}



void Engine::CheckAvatarsAndGuards() {
    for (int i = 0; i < tracking->GetNumberOfViewers(); i++) {     
        // Put the avatar there
        Vec2 wallPosition = tracking->GetViewer(i)->ProjectToWall();
        float screenMin = 0.5f;
        float screenMax = 2.5f;
        float y = (tracking->GetViewer(i)->GetPosition().Z() - screenMin) / (screenMax - screenMin);
        Vec2 position = Vec2(WallsToGraphics(wallPosition), y);
//        avatars[i]->SetPosition(position);
        avatars[i]->SetDesiredPosition(position);       

        // Update the distance
        float distance = tracking->GetViewer(i)->GetPosition().Distance(wallPosition);
        avatars[i]->UpdateDistance(distance);


        // Check for guard
        if (canLoadGuard) {
            if (tracking->GetViewer(i)->GetOldQuadrant() == activeQuadrant &&
                tracking->GetViewer(i)->GetQuadrant() != activeQuadrant) {
                // Moved away from active quadrant
                imagery.push_back(new GuardImage());
                imagery.back()->SetQuadrant(OppositeQuadrant(activeQuadrant));
                PlayVideo(guardVideos[rand() % (int)guardVideos.size()], imagery.back());
                canLoadGuard = false;
            }
        }
    }
}


void Engine::CheckQuadrant() {
    // Check the number of viewers in the active quadrant
    int viewersInQuadrant = 0;
    for (int i = 0; i < tracking->GetNumberOfViewers(); i++) {
        if (tracking->GetViewer(i)->GetQuadrant() == activeQuadrant) viewersInQuadrant++;
    }

    bool addImage = false;

    if (numberOfQuadrantImages % 4 > 1) {
        if (viewersInQuadrant <= 1) {
            // Do nothing, there will already be an image there
        }
        else if (viewersInQuadrant <= 4) {
            if (numberOfQuadrantImages % 4 == 1) addImage = true;
        }
        else if (viewersInQuadrant <= 6) {
            if (numberOfQuadrantImages % 4 <= 2) addImage = true;
        }
        else if (viewersInQuadrant <= 8) {
            if (numberOfQuadrantImages % 4 <= 3) addImage = true;
        }
        else {
            if (numberOfQuadrantImages % 4 == 0) addImage = true;
        }
    }


    // Check the trigger
    if (triggerCount > numberOfQuadrantImages) addImage = true;


    // Add images       
    if (addImage) {            
        // Scale previous in the current quadrant
        const float scale = 0.75;
        for (int i = (int)imagery.size() - 1; i >= 0; i--) {
            if (imagery[i]->GetQuadrant() == activeQuadrant) {
                imagery[i]->SetDesiredScale(scale);
                break;
            }
        }


        // Check for new quadrant
        if (numberOfQuadrantImages > 0 && numberOfQuadrantImages % 4 == 0) {
            wxLogMessage("");
            DoNewQuadrant();
        }


        // Move previous images around
        for (int i = 0; i < (int)imagery.size(); i++) {
            if (imagery[i]->GetQuadrant() == activeQuadrant) {
                imagery[i]->GeneratePosition();
            }
        }


        // Load a random image at a random location in the active quadrant
        int index = rand() % (int)(chooseQuadrantImages.size() + 
                                   chooseQuadrantVideos.size() + 
                                   chooseTimelineVideos.size());

        if (index >= (int)(chooseQuadrantImages.size() + chooseQuadrantVideos.size())) {
            // Load timeline video
            index -= (int)(chooseQuadrantImages.size() + chooseQuadrantVideos.size());

            wxLogMessage("Engine::UpdateQuadrant() : Playing timeline video");

            imagery.push_back(new QuadrantImage());
            imagery.back()->SetQuadrant(activeQuadrant);
            PlayVideo(chooseTimelineVideos[index], imagery.back());
            imagery.back()->SetScale(0.8);
            imagery.back()->SetDesiredScale(0.8f);

            // Remove this video so it is not shown again
            chooseTimelineVideos.erase(chooseTimelineVideos.begin() + index);

            // Set up more videos to play in sequence
            while (rand() % 2 == 0 && (int)chooseTimelineVideos.size() > 0) {
                index = rand() % (int)chooseTimelineVideos.size();

                wxLogMessage("Engine::UpdateQuadrant() : \tAdding timeline video");

                connections.back().AddVideo(chooseTimelineVideos[index]);

                // Remove this timeline info so it is not shown again
                chooseTimelineVideos.erase(chooseTimelineVideos.begin() + index);
            }
        }
        else if (index >= (int)(chooseQuadrantImages.size())) {
            index -= (int)chooseQuadrantImages.size();

            // Load quadrant video
            wxLogMessage("Engine::UpdateQuadrant() : Playing quadrant video");

            imagery.push_back(new QuadrantImage());
            imagery.back()->SetQuadrant(activeQuadrant);
            PlayVideo(chooseQuadrantVideos[index], imagery.back());

            // Remove this video so it is not shown again
            chooseQuadrantVideos.erase(chooseQuadrantVideos.begin() + index);
        }
        else {
            // Show an image               
            wxLogMessage("Engine::UpdateQuadrant() : Showing image");

            imagery.push_back(new QuadrantImage());
            imagery.back()->SetQuadrant(activeQuadrant);
            ShowImage(chooseQuadrantImages[index], imagery.back());
            
            // Remove this image so it is not shown again
            chooseQuadrantImages.erase(chooseQuadrantImages.begin() + index);
        }

        numberOfQuadrantImages++;
    }
}


void Engine::CheckFragments() {
    // Image fragments
    bool showFragment = false;
    for (int i = 0; i < tracking->GetNumberOfViewers(); i++) {
        if (tracking->GetViewer(i)->TriggerFragment()) {
            showFragment = true;
            break;
        }
    }

    if (showFragment) {
        // Load fragments
        if (numberOfFragmentImages < 5) {
            int index = rand() % (int)fragmentTextures.size();

            // Show new fragment
            imagery.push_back(new FragmentImage());
            ShowTexture(fragmentTextures[index], imagery.back());

            numberOfFragmentImages++;

            // Random scale between 0.25 and 1.5
            float scale = (float)rand() / (float)RAND_MAX * 0.25 + 1.25;
            imagery.back()->SetScale(scale);
            imagery.back()->SetDesiredScale(scale);
            imagery.back()->SetTimer();
        }
    }


    // Video stutter
    float distanceTrigger = 1.5;
    for (int i = 0; i < (int)connections.size(); i++) {
        float distance = tracking->GetAverageDistance(GraphicsToWalls(connections[i].GetImage()->GetPosition().X()), 1);
        if (distance < distanceTrigger) {
            float jumpAmount = (float)rand() / (float)RAND_MAX * 0.5 + 0.25;
            connections[i].GetCurrentVideo()->Jump(-jumpAmount);     
        }
    }

    // Shift scanlines
    for (int i = numberOfOldImages; i < (int)imagery.size(); i++) {
        float distance = tracking->GetAverageDistance(GraphicsToWalls(imagery[i]->GetPosition().X()), 1);
        if (distance < distanceTrigger) {
            imagery[i]->Shift();    
        }
        else {
            imagery[i]->NoShift();
        }
    }


    // Audio fragments
    if (numberOfQuadrantImages % 4 != 1 && showFragment) {
        // Load audio fragments
        if ((int)fragmentSounds.size() < 5) {
            PlayFragment();
        }
    }
}


void Engine::CheckFadedOut() {
    // Pick an image to fade out
    if (numberOfOldImages > 0) {
        // Fade out a random image
        int index = rand() % numberOfOldImages;
        imagery[index]->FadeOut();
    }

    // Check for faded out
    for (int i = 0; i < numberOfOldImages; i++) {
        if (imagery[i]->FadedOut()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
            numberOfOldImages--;
        }
    }
}


void Engine::ShowImage(ILuint imageHandle, AzraelImage*& image) {
    // Set the current image
    ilBindImage(imageHandle);

    // Get image info
    int width = ilGetInteger(IL_IMAGE_WIDTH);
    int height = ilGetInteger(IL_IMAGE_HEIGHT);
    int ilPixelFormat = ilGetInteger(IL_IMAGE_FORMAT);

    // Get the pixel format
    Image::PixelFormat pixelFormat;
    if (ilPixelFormat == IL_LUMINANCE) {
        pixelFormat = Image::LUMINANCE;
    }
    else if (ilPixelFormat == IL_RGB) {
        // Already converted to IL_RGBA in LoadImages, so we should never be here
        wxLogMessage("Engine::ShowImage() : Error, trying to load IL_RGB");
    }
    else if (ilPixelFormat == IL_RGBA) {
        pixelFormat = Image::RGBA;
    }
    else {
        // Already caught this in LoadImages, so we should never be here
        wxLogMessage("Engine::ShowImage() : Error, trying to load unknown pixel format");
    }


    image->SetTextureInfo(width, height, pixelFormat);
    image->SetTextureData(ilGetData());
    image->SetViewExtents(0.0, graphics->GetViewWidth());
    image->SetScale(1.0);
    image->SetDesiredScale(1.0);
    image->SetFadeFragmentProgram(graphics->GetFadeFragmentProgram(), graphics->GetOpacityParameter(), graphics->GetShiftParameter());
    image->SetBlurFragmentPrograms(graphics->GetHorizontalBlurFragmentProgram(), graphics->GetHorizontalBlurParameter(),
                                   graphics->GetVerticalBlurFragmentProgram(), graphics->GetVerticalBlurParameter());
    image->SetAlignType(AzraelImage::None);
    image->SetAlignBottom(false);
    image->SetDontScale(false);
    image->GeneratePosition();
}

void Engine::ShowTexture(const Texture& texture, AzraelImage*& image) {
    image->SetTexture(texture.texture, texture.width, texture.height, texture.pixelFormat);
    image->SetViewExtents(0.0, graphics->GetViewWidth());
    image->SetScale(1.0);
    image->SetDesiredScale(1.0);
    image->SetFadeFragmentProgram(graphics->GetFadeFragmentProgram(), graphics->GetOpacityParameter(), graphics->GetShiftParameter());
    image->SetBlurFragmentPrograms(graphics->GetHorizontalBlurFragmentProgram(), graphics->GetHorizontalBlurParameter(),
                                   graphics->GetVerticalBlurFragmentProgram(), graphics->GetVerticalBlurParameter());
    image->SetAlignType(AzraelImage::None);
    image->SetAlignBottom(false);
    image->GeneratePosition();
}

void Engine::PlayVideo(AzraelVideo* video, AzraelImage*& image, bool violent) {
    // Get the pixel format
    VideoStream::VideoType videoType = video->GetVideoType();
    Image::PixelFormat pixelFormat = Image::BGR;
    if (videoType == VideoStream::RGBA) pixelFormat = Image::BGRA;

    image->SetTextureInfo(video->GetWidth(), video->GetHeight(), pixelFormat);
    image->SetViewExtents(0.0, graphics->GetViewWidth());
    image->SetScale(1.0);
    image->SetDesiredScale(1.0);
    image->SetFadeFragmentProgram(graphics->GetFadeFragmentProgram(), graphics->GetOpacityParameter(), graphics->GetShiftParameter());
    image->SetBlurFragmentPrograms(graphics->GetHorizontalBlurFragmentProgram(), graphics->GetHorizontalBlurParameter(),
                                   graphics->GetVerticalBlurFragmentProgram(), graphics->GetVerticalBlurParameter());
    image->SetAlignType(video->GetAlignType());
    image->SetAlignBottom(video->GetAlignBottom());
    image->SetDontScale(video->DontScale());
    image->GeneratePosition();


    // Create a connection for updating the image with the video data
    if (violent) {
        violentConnection = new VideoImageConnection(video, image);
    }
    else {
        VideoImageConnection connection(video, image);
        connections.push_back(connection);
    }


    // Play the video
    video->Play();
}


void Engine::PlayLongAudio(int channel) {
    wxLogMessage("Engine::PlayLongAudio() : Playing long audio");

    int index;
    do {
        index = rand() % (int)longSounds.size();
    }
    while (index == currentLongSound);
    currentLongSound = index;
    longSounds[currentLongSound]->SetChannel(channel);
    longSounds[currentLongSound]->Play();
}


void Engine::PlayFragment() {
    // Pick a quadrant that is not the active quadrant.
    int quadrant;
    do {
        quadrant = rand() % 4;
    }
    while (quadrant == activeQuadrant);
 

    // Load the audio
    bool success = false;
    do {
        int index = rand() % (int)fragmentSoundNames.size();

        fragmentSounds.push_back(new AudioStream());
        fragmentSounds.back()->SetFileName(fragmentSoundNames[index]);
        success = fragmentSounds.back()->Initialize(false, QuadrantToAudioChannel(quadrant));
        if (!success) {
            delete fragmentSounds.back();
            fragmentSounds.pop_back();

            fragmentSoundNames.erase(fragmentSoundNames.begin() + index);
        }
    }
    while (!success);

    fragmentSounds.back()->Play();
}


void Engine::DoNewQuadrant() {
    wxLogMessage("Engine::DoNewQuadrant()");

    // Fade current images and pin them to their current position
    for (int i = 0; i < (int)imagery.size(); i++) {
        if (imagery[i]->DeleteOnNewQuadrant()) {
            delete imagery[i];
            imagery.erase(imagery.begin() + i);
            i--;
        }
        else {
            imagery[i]->Fade();
            if (imagery[i]->GetScale() == 0.8) {
                imagery[i]->SetDesiredScale(0.6f); 
            }
            else {
                imagery[i]->SetDesiredScale(0.75);
            }
        }
    }

    // Stop current videos
    for (int i = 0; i < (int)connections.size(); i++) {
        connections[i].GetCurrentVideo()->Stop();
    }
    connections.clear();


    // Select new quadrant
    int q = activeQuadrant;
    do {
        activeQuadrant = GenerateQuadrant();
    }
    while (activeQuadrant == q);


    // Check violent video
    if (violentImage) {
        if (violentImage->GetQuadrant() == activeQuadrant) {
            delete violentImage;
            delete violentConnection;
            violentImage = NULL;
            violentConnection = NULL;
        }
    }


    // Reset ability to load images
    canLoadGuard = true;



    // Stop current fragment sounds
    for (int i = 0; i < (int)fragmentSounds.size(); i++) {
        delete fragmentSounds[i];
    }
    fragmentSounds.clear();


    // Pause current audio
    longSounds[currentLongSound]->Pause();
    
    // Play new audio in new quadrant
    PlayLongAudio(QuadrantToAudioChannel(activeQuadrant));
}

int Engine::GenerateQuadrant() const {
    return rand() % 4;
}

const Vec2 Engine::GraphicsToWalls(float position) const {
    float quadrantWidth = graphics->GetViewWidth() * 0.25;
    float wallWidth = tracking->GetRoomMax().X() - tracking->GetRoomMin().X();

    if (position >= 0 && position < quadrantWidth) {
        // Wall 1
        return Vec2(position * wallWidth / quadrantWidth, 
                    tracking->GetRoomMax().Y());
    }
    else if (position >= quadrantWidth && position < quadrantWidth * 2.0) {
        // Wall 2
        return Vec2(tracking->GetRoomMax().X(), 
                    tracking->GetRoomMax().Y() - (position - quadrantWidth) * wallWidth / quadrantWidth);
    }
    else if (position >= quadrantWidth * 2.0 && position < quadrantWidth * 3.0) {
        // Wall 3
        return Vec2(tracking->GetRoomMax().X() - (position - quadrantWidth * 2.0) * wallWidth / quadrantWidth,
                    tracking->GetRoomMin().Y());
    }
    else {
        // Wall 4
        return Vec2(tracking->GetRoomMin().X(),
                    (position - quadrantWidth * 3.0) * wallWidth / quadrantWidth);
    }
}

float Engine::WallsToGraphics(const Vec2& position) const {    
    float quadrantWidth = graphics->GetViewWidth() * 0.25;
    float wallWidth = tracking->GetRoomMax().X() - tracking->GetRoomMin().X();

    float x = position.X();
    float y = position.Y();

    float epsilon = 0.01f;
    if (y >= wallWidth - epsilon && y <= wallWidth + epsilon) {
        // Wall 0
        return x / wallWidth * quadrantWidth;
    }
    else if (x >= wallWidth - epsilon && x <= wallWidth + epsilon) {
        // Wall 1
        return quadrantWidth + (wallWidth - y) / wallWidth * quadrantWidth;
    }
    else if (y >= -epsilon && y <= epsilon) {
        // Wall 2
        return quadrantWidth * 2.0 + (wallWidth - x) / wallWidth * quadrantWidth;
    }
    else {
        // Wall 3
        return quadrantWidth * 3.0 + y / wallWidth * quadrantWidth;
    }
}

int Engine::QuadrantToAudioChannel(int quadrant) {
    if (quadrant == 0) {
        return 2;
    }
    else if (quadrant == 1) {
        return 1;
    }
    else if (quadrant == 2) {
        return 3;
    }
    else if (quadrant == 3) {
        return 4;
    }
    else {
        // Room speakers
        return 5;
    }
}

int Engine::OppositeQuadrant(int quadrant) {
    if (quadrant == 0) {
        return 2;
    }
    else if (quadrant == 1) {
        return 3;
    }
    else if (quadrant == 2) {
        return 0;
    }
    else if (quadrant == 3) {
        return 1;
    }

    // Shouldn't be here
    return 0;
}