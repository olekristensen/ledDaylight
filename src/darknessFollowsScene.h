#pragma once
#include "ledScene.h"
#include "LedFixture.h"

class darknessFollowsScene : public ledScene
{
public:
    darknessFollowsScene();
    void setup();
    void setGUI(ofxUISuperCanvas* gui);
    void update();
    void draw();

    void mouseMoved(int x, int y);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);

    void saveSettings(ofFile f);
    void loadSettings(ofFile f);

    vector<LedFixture*> lights;

    ofPlanePrimitive floor;
    ofxOlaShaderLight::Material white;
    ofxOlaShaderLight::Material ballMaterial;

    unsigned int kelvinCold;
    unsigned int kelvinWarm;

    float kelvinWarmRange;
    float kelvinColdRange;
    float temperatureSpeed;
    float temperatureTime;
    float temperatureSpread;

    float brightnessRangeFrom;
    float brightnessRangeTo;
    float brightnessSpeed;
    float brightnessTime;
    float brightnessSpread;

    float manualBalance;

    float kelvinWarmRangePos;
    float kelvinColdRangePos;

    float brightnessRangeFromPos;
    float brightnessRangeToPos;

    float kelvinWarmRangeDir;
    float kelvinColdRangeDir;

    float brightnessRangeFromDir;
    float brightnessRangeToDir;

    float posSize;

    ofEasyCam cam;

    float timeOffset = 100.0;
    float lastFrameSeconds;

    float lightZposCheat = LIGHT_POS_Z_CHEAT;

    ofTrueTypeFont font;
    float colorPickerRadius;
    int millisLastClick;

    bool wallsOrCorners;

    ofVec3f mouseVec;
    ofNode  floorPos;

protected:
private:
};
