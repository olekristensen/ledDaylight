#pragma once
#include "ledScene.h"
#include "ofxOlaShaderLight.h"

#define LIGHT_POS_Z_CHEAT 4.0

class ChromaWhiteSpot : public ofxOlaShaderLight
{
public:

    void setup(unsigned int startAddress = 0)
    {
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_COLOR_TEMPERATURE, false));
            DMXchannels.push_back(new DMXchannel(startAddress+1, DMXchannel::DMX_CHANNEL_BRIGHTNESS, false));
        }
        ofLight::setSpotlight();
        setAttenuation(1./.1);
        temperatureRangeColdKelvin = 6500;
        temperatureRangeWarmKelvin = 2700;
        setNormalisedBrightness(1.0);
        selected = false;
    };

    void draw()
    {
        ofPushStyle();
        ofPushMatrix();
        ofTranslate(0,0,getPosition().z*(LIGHT_POS_Z_CHEAT-1));
        ofSetColor(ofLight::getDiffuseColor());
        ofLight::draw();
        ofPopMatrix();
        ofPopStyle();
    };
    bool selected;
};


class darknessFollowsScene : public ledScene
{
public:
    darknessFollowsScene();
    void setup();
    void setGUI(ofxUISuperCanvas* gui);
    void update(ola::DmxBuffer * buffer);
    void draw();

    void mouseMoved(int x, int y);

    vector<ChromaWhiteSpot*> spotlights;

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

    ofEasyCam cam;

    float timeOffset = 100.0;
    float lastFrameSeconds;

    float lightZposCheat = LIGHT_POS_Z_CHEAT;

    ofTrueTypeFont font;

protected:
private:
};
