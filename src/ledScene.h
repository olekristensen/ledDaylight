#pragma once
#include "ofMain.h"
#include "LedFixture.h"
#include "ofxUI.h"

class ledScene
{
public:
    ledScene();
    ~ledScene();
    virtual void setup() {};
    virtual void draw() {};
    virtual void update() {};
    virtual void setGUI(ofxUISuperCanvas* gui)
    {
        this->gui = gui;
    };
    void hideGUI();
    void showGUI();
    virtual void mouseMoved(int x, int y ){};
    virtual void mouseDragged(int x, int y, int button){};
    virtual void mousePressed(int x, int y, int button){};
    virtual void mouseReleased(int x, int y, int button){};
    string name;
    vector<ofxUIWidget*> guiWidgets;
    ofxUISuperCanvas* gui;
protected:
private:
};
