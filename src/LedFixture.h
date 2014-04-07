#pragma once
#include "ofMain.h"

#define USE_OLA_LIB_AND_NOT_OSC 1
#include "ofxOlaShaderLight.h"

#define LIGHT_POS_Z_CHEAT 4.0

class LedFixture : public ofxOlaShaderLight
{
public:

    void setup()
    {
        selected = false;
        manual = false;
        directional = false;
        manualBrightness = 1.0;
        manualTemperature = 6500;
    };

    void draw()
    {
        ofPushStyle();
        ofPushMatrix();
        ofTranslate(0,0,getPosition().z*(LIGHT_POS_Z_CHEAT-1));
        ofSetColor(ofLight::getDiffuseColor());
        drawForm();
        ofPopMatrix();
        ofPopStyle();
    };

    virtual void drawForm(){};

    bool selected;
    bool manual;
    bool directional;
    float manualBrightness;
    int manualTemperature;
    int addressInterval = 1;

};

class ChromaWhiteSpot : public LedFixture
{
public:

    void setup(unsigned int startAddress = 0)
    {
        this->LedFixture::setup();
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
        addressInterval = 2;
    };

    void drawForm()
    {
        ofLight::draw();
    };

};

class ChromaWhiteBall : public LedFixture
{
public:

    void setup(unsigned int startAddress = 0)
    {
        this->LedFixture::setup();
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_COLOR_TEMPERATURE, false));
            DMXchannels.push_back(new DMXchannel(startAddress+1, DMXchannel::DMX_CHANNEL_BRIGHTNESS, false));
        }
        setAttenuation(1./.1);
        temperatureRangeColdKelvin = 6500;
        temperatureRangeWarmKelvin = 2700;
        setNormalisedBrightness(1.0);
        addressInterval = 2;
    };

    void drawForm()
    {
        ofLight::draw();
    };

};

class LupoLed : public LedFixture
{
public:

    void setup(unsigned int startAddress = 0)
    {
        this->LedFixture::setup();
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_BRIGHTNESS, false));
            DMXchannels.push_back(new DMXchannel(startAddress+1, DMXchannel::DMX_CHANNEL_COLOR_TEMPERATURE, false,true));
        }
        setAttenuation(1./.1);
        temperatureRangeColdKelvin = 6500;
        temperatureRangeWarmKelvin = 2700;
        setNormalisedBrightness(1.0);
        addressInterval = 2;
    };

    void drawForm()
    {
        ofPushMatrix();
        transformGL();
        ofDrawBox(ofVec3f(0,0,0), 35, 8, 20);
        ofPushStyle();
        if(ofGetStyle().color == ofLight::getDiffuseColor()){
            ofSetColor(0,255);
        }
        ofDrawBox(ofVec3f(0,-4,0), 33, 5, 18);
        ofDrawAxis(80);
        ofPopStyle();
        restoreTransformGL();
        ofPopMatrix();
    };

};

class StudioHDSpot : public LedFixture
{
public:

    void setup(unsigned int startAddress = 0)
    {
        this->LedFixture::setup();
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_BRIGHTNESS, false));
            DMXchannels.push_back(new DMXchannel(startAddress+1, DMXchannel::DMX_CHANNEL_COLOR_TEMPERATURE, false));
        }
        ofLight::setSpotlight();
        setAttenuation(1./.1);
        temperatureRangeColdKelvin = 6500;
        temperatureRangeWarmKelvin = 2700;
        setNormalisedBrightness(1.0);
        addressInterval = 5;
    };

    void drawForm()
    {
        ofLight::draw();
    };
};
