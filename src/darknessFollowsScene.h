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

    vector<StudioHDSpot*> spotlights;

    ofPlanePrimitive floor;
    ofxOlaShaderLight::Material white;

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

    ofEasyCam cam;

    float timeOffset = 100.0;
    float lastFrameSeconds;

    float lightZposCheat = LIGHT_POS_Z_CHEAT;

    ofTrueTypeFont font;
    float colorPickerRadius;
    int millisLastClick;

    ofVec3f mouseVec;
    ofVec3f mouseVecAfter;

protected:
private:
};
