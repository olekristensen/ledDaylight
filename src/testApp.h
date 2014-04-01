#pragma once

#include "ofMain.h"
#include "pgCamera.h"
#include "CameraController.h"
#include "fullScreenDialogs.h"
#include "ofxUI.h"
#include "darknessFollowsScene.h"

class testApp : public ofBaseApp
{

public:

    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void setGUI();

protected:
    //pgCamera sourceCamera;
    //pgCamera destinationCamera;

    CameraController cameraController;

    //ola::DmxBuffer buffer;
    //ola::StreamingClient ola_client;

    vector<ledScene*> scenes;
    ledScene * activeScene;

    ofxUISuperCanvas *gui;
    bool hideGUI;
    bool bSaveSettings;
    bool bLoadSettings;
    void guiEvent(ofxUIEventArgs &e);
};
