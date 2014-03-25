#pragma once
#include "ledScene.h"

class TesselationSquare : public ofPlanePrimitive, public LedFixture
{
public:

    void setup(unsigned int startAddress = 0)
    {
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_WW, true));
            DMXchannels.push_back(new DMXchannel(startAddress+2, DMXchannel::DMX_CHANNEL_CW, true));
        }
        kelvinCold = 6500;
        kelvinWarm = 4200;
        brightness = 1.0;
    };

    void update()
    {
        drawColor = color * brightness;
        if(DMXstartAddress > 0)
        {
            drawColor *= temperatureToColor(temperature);
        }
        else
        {
            drawColor *= temperatureToColor(round(ofMap(0.25, 0,1, kelvinCold, kelvinWarm)));
            drawColor *= 0.75;
        }
    };

    void draw()
    {
        //ofPushStyle();
        ofSetColor(drawColor);
        ofPlanePrimitive::draw();
        //ofPopStyle();
    };

    void setColor(ofColor color)
    {
        this->color = color;
    }

    ofColor getColor()
    {
        return color;
    }

    void setTemperature(unsigned int degreesKelvin)
    {
        temperature = degreesKelvin;
    }

    unsigned int getTemperature()
    {
        return temperature;
    }

    void setBrightness(float brightness)
    {
        this->brightness = brightness;
    }

    float getBrightness()
    {
        return brightness;
    }

    float brightness;
    unsigned int temperature;

    ofColor color;
    ofColor drawColor;

};

class tesselationScene : public ledScene
{
public:
    tesselationScene();
    void setup();
    void setGUI(ofxUISuperCanvas* gui);
    void update(ola::DmxBuffer * buffer);
    void draw();

protected:
private:

    ofVec3f addTesselation(ofVec3f _origin, int _size, int _width, int _height, ofVec3f** _tesselationMap);

    vector<TesselationSquare*> tesselation;

    ofEasyCam cam;

    ofImage perlinNoiseImage;
    ofRectangle tesselationRect;

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

    float timeOffset = 100.0;
    float lastFrameSeconds;

};
