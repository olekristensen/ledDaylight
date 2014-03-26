#include "darknessFollowsScene.h"

darknessFollowsScene::darknessFollowsScene()
{
    //ctor
}

void darknessFollowsScene::setup()
{

    ofTrueTypeFont::setGlobalDpi(72);

    font.loadFont("verdana.ttf", 24, true, true);

    name = "Darkness Follows";

    kelvinCold = 6500;
    kelvinWarm = 2700;

    kelvinColdRange = kelvinCold;
    kelvinWarmRange = kelvinWarm;
    temperatureSpeed = 0.5;
    temperatureSpread = 0.15;
    temperatureTime = 0;

    brightnessRangeFrom = 0.0;
    brightnessRangeTo = 1.0;
    brightnessSpeed = 0.4;
    brightnessSpread = 0.25;
    brightnessTime = 0;

    int numberSpotlightsX = 3;
    int numberSpotlightsY = 3;
    float spotlightsDistX = 110;
    float spotlightsDistY = 85;
    float spotlightsPosZ = -285/lightZposCheat;

    floor.set(285,300,100,100);

    white.diffuseColor = ofVec4f(255,255,255,255);

    int addressMap[] = {1,3,5,
                        7,9,11,
                        13,15,17
                       };

    int addressIndex = 0;

    ofVec3f posOffsetFromCenter(spotlightsDistX*(numberSpotlightsX-1)*.5, spotlightsDistY*(numberSpotlightsY-1)*.5, spotlightsPosZ);

    for(int x = 0; x < numberSpotlightsX; x++)
    {
        for(int y = numberSpotlightsY-1; y >=0; y--)
        {
            ChromaWhiteSpot* cws = new ChromaWhiteSpot();
            cws->setup(addressMap[addressIndex++]);
            cws->setParent(floor);
            ofVec3f pos(x*spotlightsDistX, y*spotlightsDistY);
            pos -= posOffsetFromCenter;
            cws->setPosition(pos);
            spotlights.push_back(cws);
        }
    }

    cam.setTranslationKey(' ');

}


void darknessFollowsScene::setGUI(ofxUISuperCanvas* gui)
{
    this->gui = gui;
    guiWidgets.push_back(gui->addLabel("Temperature", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Range", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rTemp = gui->addRangeSlider("tRange", kelvinWarm, kelvinCold, &kelvinWarmRange, &kelvinColdRange, gui->getRect()->getWidth()-8, 30);
    rTemp->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rTemp);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpeed = gui->addSlider("tSpeed",0,1,&temperatureSpeed, gui->getRect()->getWidth()-8, 30);
    sTempSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpeed);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpread = gui->addSlider("tSpread",0,0.33,&temperatureSpread, gui->getRect()->getWidth()-8, 30);
    sTempSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel(""));
    guiWidgets.push_back(gui->addLabel("Brightness", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Range", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rBrightness = gui->addRangeSlider("bRange", 0, 1, &brightnessRangeFrom, &brightnessRangeTo, gui->getRect()->getWidth()-8, 30);
    rBrightness->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rBrightness);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpeed = gui->addSlider("bSpeed",0,1,&brightnessSpeed, gui->getRect()->getWidth()-8, 30);
    sBrightnessSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpeed);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpread = gui->addSlider("bSpread",0,0.33,&brightnessSpread, gui->getRect()->getWidth()-8, 30);
    sBrightnessSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel(""));
}

void darknessFollowsScene::update(ola::DmxBuffer * buffer)
{

    double temperatureSpreadCubic = powf(temperatureSpread, 3);
    double brightnessSpreadCubic = powf(brightnessSpread, 3);

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); it++)
    {
        ChromaWhiteSpot * cws = *(it);
        float tempNoise = ofNoise(cws->getPosition().x*temperatureSpreadCubic, cws->getPosition().y*temperatureSpreadCubic, temperatureTime);
        unsigned int temperature = round(ofMap(tempNoise, 0, 1, fmaxf(cws->temperatureRangeWarmKelvin, kelvinWarmRange), fminf(cws->temperatureRangeColdKelvin, kelvinColdRange)));

        float brightness = ofNoise(cws->getPosition().x*brightnessSpreadCubic, cws->getPosition().y*brightnessSpreadCubic, brightnessTime);
        brightness = ofMap(brightness, 0, 1, brightnessRangeFrom, brightnessRangeTo);

        cws->setTemperature(temperature);
        cws->setNormalisedBrightness(brightness);
    }
    ofxOlaShaderLight::update();

    float now = ofGetElapsedTimef() + timeOffset;
    temperatureTime += powf(temperatureSpeed,8) * ( ( now - lastFrameSeconds ) / (1./60));
    brightnessTime += powf(brightnessSpeed,8) * ( ( now - lastFrameSeconds ) / (1./60));
    lastFrameSeconds = now;
}

void darknessFollowsScene::draw()
{
//    ofPushMatrix();
//    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    cam.begin();

    ofPushStyle();
    ofxOlaShaderLight::begin();
    ofxOlaShaderLight::setMaterial(white);
    floor.draw();
    ofxOlaShaderLight::end();
    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        if(cws->selected)
        {
            ofPushStyle();
            ofEnableDepthTest();
            glDepthMask(GL_FALSE);
            ofPushMatrix();
            ofSetColor(255,0,0, 255);
            ofVec3f v = cws->getGlobalPosition();
            v.z *= lightZposCheat;
            v = cam.worldToScreen(v);
            v+= ofVec3f(0,0,-0.0025);
            v = cam.screenToWorld(v);
            v-= cws->getGlobalPosition();
            ofTranslate(v);
            cws->ofLight::draw();
            ofPopMatrix();
            glDepthMask(GL_TRUE);
            ofPopStyle();
        }

        cws->draw();
    }
    ofDisableLighting();
    ofPopStyle();
    cam.end();
    ofSetColor(255, 255, 255);
    ofDisableDepthTest();
    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(0.99,1.,lightZposCheat*1.))+ofVec3f(12,-6,0));
        v.x -= ofGetWidth()/3.;
        v.x *= ofGetWidth()/(ofGetWidth()*2./3.);
        font.drawString(ofToString(cws->DMXstartAddress),v.x,v.y+60);
    }


//    ofPopMatrix();
}

void darknessFollowsScene::mouseMoved(int x, int y)
{

    //TODO: Make mouse find a light WHAT IS UP WITH VIEWPORTS!?!?!
    ofVec3f mouseVec = ofVec3f(x,y,0);
    cout << mouseVec << endl;
    mouseVec.x = ofMap(mouseVec.x, gui->getRect()->getWidth(), ofGetWidth(), gui->getRect()->getWidth(), ofGetWidth()-gui->getRect()->getWidth());
    cout << mouseVec << endl;

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen(cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.));
        if(v.distance(mouseVec) < 20){
            cws->selected = true;
        } else {
            cws->selected = false;
        }
    }
}
