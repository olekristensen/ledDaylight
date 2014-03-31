#include "darknessFollowsScene.h"

darknessFollowsScene::darknessFollowsScene()
{
    //ctor
}

void darknessFollowsScene::setup()
{

    ofTrueTypeFont::setGlobalDpi(72);

    font.loadFont("verdana.ttf", 18, true, true);

    name = "Darkness Follows";

    kelvinCold = 6500;
    kelvinWarm = 2700;

    colorPickerRadius = 50;

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

    manualBalance = 1.0;

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
    cam.setTarget(ofVec3f(0,0,140));
    cam.setOrientation(ofQuaternion(45,ofVec3f(1,0,0)));
    cam.setDistance(600);
    cam.disableMouseInput();

}


void darknessFollowsScene::setGUI(ofxUISuperCanvas* gui)
{
    this->gui = gui;
    guiWidgets.push_back(gui->addLabel("Temperature", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Range", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rTemp = gui->addRangeSlider("tRange", kelvinWarm, kelvinCold, &kelvinWarmRange, &kelvinColdRange, gui->getRect()->getWidth()-8, 20);
    rTemp->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rTemp);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpeed = gui->addSlider("tSpeed",0,1,&temperatureSpeed, gui->getRect()->getWidth()-8, 20);
    sTempSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpeed);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpread = gui->addSlider("tSpread",0,0.33,&temperatureSpread, gui->getRect()->getWidth()-8, 20);
    sTempSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel(""));
    guiWidgets.push_back(gui->addLabel("Brightness", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Range", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rBrightness = gui->addRangeSlider("bRange", 0, 1, &brightnessRangeFrom, &brightnessRangeTo, gui->getRect()->getWidth()-8, 20);
    rBrightness->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rBrightness);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpeed = gui->addSlider("bSpeed",0,1,&brightnessSpeed, gui->getRect()->getWidth()-8, 20);
    sBrightnessSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpeed);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpread = gui->addSlider("bSpread",0,0.33,&brightnessSpread, gui->getRect()->getWidth()-8, 20);
    sBrightnessSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel(""));
    guiWidgets.push_back(gui->addLabel("Manual", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Balance", OFX_UI_FONT_SMALL));
    ofxUISlider * sManualBalance = gui->addSlider("mBalance",0.0,1.0,&manualBalance, gui->getRect()->getWidth()-8, 20);
    sManualBalance->setColorBack(ofColor(48,48,48));
    sManualBalance->setColorFillHighlight(ofColor(255,0,0));
    sManualBalance->setColorOutlineHighlight(ofColor(255,0,0));
    guiWidgets.push_back(sManualBalance);
    guiWidgets.push_back(gui->addSpacer());

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

        if(cws->manual || cws->selected ){
            int balancedTemperature((manualBalance*cws->manualTemperature)+(temperature*(1.0-manualBalance)));
            float balancedBrightness((manualBalance*cws->manualBrightness)+(brightness*(1.0-manualBalance)));
            cws->setTemperature(balancedTemperature);
            cws->setNormalisedBrightness(balancedBrightness);
        } else {
            cws->setTemperature(temperature);
            cws->setNormalisedBrightness(brightness);
        }
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
        if(cws->selected || cws->manual)
        {
            ofPushStyle();
            ofEnableDepthTest();
            glDepthMask(GL_FALSE);
            ofPushMatrix();
            if(cws->selected){
                ofSetColor(0,255,0, 255);
            }
            if(cws->manual){
                ofSetColor(255,0,0, 255);
            }
            if(cws->manual && cws->selected){
                ofSetColor(255,255,0, 255);
            }
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
    ofSetColor(64, 64, 64,127);
    ofDisableDepthTest();
    ofViewport();

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        string s(ofToString(cws->DMXstartAddress));
        font.drawString(s,v.x-(font.stringWidth(s)/2.0),v.y+(font.stringHeight(s)/2.0));

        if(cws->selected){
        ofFloatColor fromTemp(DMXfixture::temperatureToColor(cws->temperatureRangeWarmKelvin));
        ofFloatColor toTemp(DMXfixture::temperatureToColor(cws->temperatureRangeColdKelvin));
        fromTemp.a = 0.5;
        toTemp.a = 0.5;
        ofMesh mesh;
        mesh.addVertex(ofPoint(-colorPickerRadius,-colorPickerRadius,0)); // make a new vertex
        mesh.addColor(fromTemp);  // add a color at that vertex
        mesh.addVertex(ofPoint(colorPickerRadius,-colorPickerRadius,0)); // make a new vertex
        mesh.addColor(toTemp);  // add a color at that vertex
        mesh.addVertex(ofPoint(colorPickerRadius,colorPickerRadius,0)); // make a new vertex
        mesh.addColor(fromTemp*0);  // add a color at that vertex
        mesh.addVertex(ofPoint(colorPickerRadius,colorPickerRadius,0)); // make a new vertex
        mesh.addColor(fromTemp*0);  // add a color at that vertex
        mesh.addVertex(ofPoint(-colorPickerRadius,colorPickerRadius,0)); // make a new vertex
        mesh.addColor(toTemp*0);  // add a color at that vertex
        mesh.addVertex(ofPoint(-colorPickerRadius,-colorPickerRadius,0)); // make a new vertex
        mesh.addColor(fromTemp);  // add a color at that vertex

        ofVec3f currentPos(
                           ofMap(cws->getTemperature(), cws->temperatureRangeWarmKelvin, cws->temperatureRangeColdKelvin, colorPickerRadius, -colorPickerRadius),
                           ofMap(cws->getNormalisedBrightness(),0, 1, -colorPickerRadius, colorPickerRadius),
                            0
                           );

        ofPushMatrix();
            ofTranslate(ofGetMouseX(), ofGetMouseY(), 0);
            ofTranslate(currentPos);
            ofSetColor(0,0,0,64);
            ofRect(-colorPickerRadius-3, -colorPickerRadius-3, 6+(colorPickerRadius*2),6+(colorPickerRadius*2));
            mesh.draw();
        ofPopMatrix();
        }
    }
//    ofPopMatrix();
}

void darknessFollowsScene::mouseMoved(int x, int y)
{

    mouseVec = ofVec3f(x,y,0);

    bool oneIsSelected = false;

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(v.distance(mouseVec) < 30){
            cws->selected = true;
            oneIsSelected = true;
            cws->manualTemperature = cws->getTemperature();
            cws->manualBrightness = cws->getNormalisedBrightness();
        } else {
            cws->selected = false;
        }
    }
    if(oneIsSelected){
//        cam.disableMouseInput();
    } else {
//        cam.enableMouseInput();
    }
}

void darknessFollowsScene::mousePressed(int x, int y, int button){

    mouseVec = ofVec3f(x,y,0);
    millisLastClick = ofGetElapsedTimeMillis();

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
    }

}

void darknessFollowsScene::mouseReleased(int x, int y, int button){

    mouseVec = ofVec3f(x,y,0);

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(cws->selected){
            if(cws->manual && ofGetElapsedTimeMillis() - millisLastClick < 500 ){
                cws->manual = false;
            } else {
                cws->manual = true;
            }
        }
    }

}

void darknessFollowsScene::mouseDragged(int x, int y, int button){

    for(vector<ChromaWhiteSpot*>::iterator it = spotlights.begin(); it != spotlights.end(); ++it)
    {
        ChromaWhiteSpot * cws = *(it);
        ofVec3f v = cam.worldToScreen((cws->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(cws->selected){

            //TODO: make temperature navigation

            float manualTemperatureNormalised = ofMap(cws->manualTemperature, cws->temperatureRangeWarmKelvin, cws->temperatureRangeColdKelvin, 0.0, 1.0);
            manualTemperatureNormalised = ofClamp(manualTemperatureNormalised + ((x-mouseVec.x)/(colorPickerRadius*2.0)), 0.0, 1.0);

            cws->manualTemperature = ofMap(manualTemperatureNormalised,0.0,1.0,cws->temperatureRangeWarmKelvin, cws->temperatureRangeColdKelvin);
            cws->manualBrightness = ofClamp(cws->manualBrightness + ((mouseVec.y-y)/(colorPickerRadius*2.0)),0.0,1.0);
        }
    }
    mouseVec = ofVec3f(x,y,0);
}
