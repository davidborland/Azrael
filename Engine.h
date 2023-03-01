///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Engine.h
//
// Author:      David Borland
//
// Description: Main engine of Azrael.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef ENGINE_H
#define ENGINE_H


#include <vector>
#include <string>

#include <wx/log.h>     // This must be included before Video.h

#include <IL/il.h>
#include <IL/ilu.h>

#include <AudioStream.h>

#include "AzraelImage.h"
#include "AzraelVideo.h"
#include "AvatarImage.h"
#include "ViolentImage.h"
#include "Graphics.h"
#include "Tracking.h"
#include "ProjectorShutter.h"
#include "PosiTrack.h"
#include "VideoImageConnection.h"


struct Texture {
    GLuint texture;
    unsigned int width;
    unsigned int height;
    Image::PixelFormat pixelFormat;
};


class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize(HWND win, int windowWidth, int windowHeight);

    void Update();
    void Trigger();
    void Victimize();
    void CoolDown1();
    void CoolDown2();
    bool Reset();
    void DoViolence();

    void RenderLeft() const;
    void RenderRight() const;

    enum State {
        Normal,
        Victimizing,
        CoolingDown1,
        CoolingDown2
    };

    State GetState();

private:
    // Imagery currently being shown
    std::vector<AzraelImage*> imagery;
    std::vector<AzraelImage*> avatars;
    AzraelImage* violentImage;


    // Imagery to be shown
    std::vector<ILuint> quadrantImages;    
    std::vector<AzraelVideo*> quadrantVideos;
    std::vector<AzraelVideo*> timelineVideos;
    std::vector<ILuint> avatarImages;
    std::vector<AzraelVideo*> guardVideos;
    std::vector<AzraelVideo*> violentVideos;


    // Multiple copies of these might be shown at once, so store the textures instead
    // of creating a new one each time.
    std::vector<Texture> fragmentTextures;
    std::vector<Texture> patchTextures;


    // Images and videos to be play in quadrants that have not been shown
    std::vector<ILuint> chooseQuadrantImages;
    std::vector<AzraelVideo*> chooseQuadrantVideos;
    std::vector<AzraelVideo*> chooseTimelineVideos;


    // Connections between videos and images used to render them
    std::vector<VideoImageConnection> connections;
    VideoImageConnection* violentConnection;


    // Sounds 
    AudioStream* ambientSound;
    AudioStream* victimRoomSound;
    AudioStream* victimCenterSound;

    int currentLongSound;
    std::vector<AudioStream*> longSounds;

    std::vector<std::string> fragmentSoundNames;
    std::vector<AudioStream*> fragmentSounds;


    // Handle rendering and tracking
    Graphics* graphics;
    Tracking* tracking;

    ProjectorShutter* projectorShutter;
    PosiTrack* posiTrack;


    // Various state variables
    int numberOfQuadrantImages;
    int numberOfFragmentImages;
    int numberOfOldImages;

    int triggerCount;

    int victimizeCount;

    int activeQuadrant;

    bool canLoadGuard;

    State state;


    // Load Media
    bool LoadImages();
    bool LoadVideos();
    bool LoadAudio();

    bool LoadImage(const std::string& fileName, ILuint& image);
    void LoadTexture(ILuint image, Texture& texture);

    void CopyChoose();


    // Updates for different states
    void UpdateNormal();
    void UpdateVictimize();
    void UpdateCoolDown1();
    void UpdateCoolDown2();

    void CheckAvatarsAndGuards();
    void CheckQuadrant();
    void CheckFragments();
    void CheckFadedOut();


    // Play Media
    void ShowImage(ILuint imageHandle, AzraelImage*& image);
    void ShowTexture(const Texture& texture, AzraelImage*& image);
    void PlayVideo(AzraelVideo* video, AzraelImage*& image, bool violent = false);
    void PlayLongAudio(int channel);
    void PlayFragment();

    void DoNewQuadrant();


    // Utility functions
    int GenerateQuadrant() const;
    const Vec2 GraphicsToWalls(float position) const;
    float WallsToGraphics(const Vec2& position) const;
    int QuadrantToAudioChannel(int quadrant);
    int OppositeQuadrant(int quadrant);
};


#endif