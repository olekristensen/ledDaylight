#include "darknessFollowsScene.h"
#include <math.h>       /* atan2 */

darknessFollowsScene::darknessFollowsScene()
{
    //ctor
}

void darknessFollowsScene::setup()
{

    ofTrueTypeFont::setGlobalDpi(72);

    font.loadFont("GUI/DroidSans.ttf", 14, true, true);

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

    brightnessRangeFromPos = brightnessRangeFrom;
    brightnessRangeToPos = brightnessRangeTo;

    kelvinColdRangePos = kelvinColdRange;
    kelvinWarmRangePos = kelvinWarmRange;

    brightnessRangeFromDir = brightnessRangeFrom;
    brightnessRangeToDir = brightnessRangeTo;

    kelvinColdRangeDir = kelvinColdRange;
    kelvinWarmRangeDir = kelvinWarmRange;

    posSize = 0.0;
    floorPos.setOrientation(ofQuaternion(-90,ofVec3f(0,0,1)));

    manualBalance = 1.0;

    wallsOrCorners = false;

    floor.set(285,300,100,100);

    white.diffuseColor = ofVec4f(255,255,255,255);
    ballMaterial.diffuseColor = ofVec4f(159,159,159,255);
    ballMaterial.specularColor = ofVec4f(255,255,255,255);
    ballMaterial.specularShininess = 0.33;

    // add StudioHDs

    int numberSpotlightsX = 4;
    int numberSpotlightsY = 3;
    float spotlightsDistX = 65;
    float spotlightsDistY = 85;
    float spotlightsPosZ = -285/lightZposCheat;

    int addressMap[] = {1,6,11,
                        16,21,26,
                        31,36,41,
                        46,51,56
                       };

    int addressIndex = 0;

    ofVec3f posOffsetFromCenter(spotlightsDistX*(numberSpotlightsX-1)*.5, spotlightsDistY*(numberSpotlightsY-1)*.5, spotlightsPosZ);

    for(int x = 0; x < numberSpotlightsX; x++)
    {
        for(int y = 0; y < numberSpotlightsY; y++)
        {
            StudioHDSpot* shd = new StudioHDSpot();
            shd->setup(addressMap[addressIndex++]);
            shd->setParent(floor);
            ofVec3f pos(x*spotlightsDistX, y*spotlightsDistY);
            pos -= posOffsetFromCenter;
            shd->setPosition(pos);
            lights.push_back(shd);
        }
    }

    // Add lupoleds

    LupoLed* softboxFront = new LupoLed();
    softboxFront->setup(201);
    softboxFront->setParent(floor);
    softboxFront->setPosition(-floor.getWidth()/2.0, -floor.getHeight()/2.0, 290/lightZposCheat);
    softboxFront->setOrientation(ofQuaternion(-45,ofVec3f(0,0,1)));
    lights.push_back(softboxFront);

    LupoLed* softboxLeft = new LupoLed();
    softboxLeft->setup(203);
    softboxLeft->setParent(floor);
    softboxLeft->setPosition(floor.getWidth()/2.0, -floor.getHeight()/2.0, 290/lightZposCheat);
    softboxLeft->setOrientation(ofQuaternion(45,ofVec3f(0,0,1)));
    lights.push_back(softboxLeft);

    LupoLed* softboxBack = new LupoLed();
    softboxBack->setup(205);
    softboxBack->setParent(floor);
    softboxBack->setPosition(floor.getWidth()/2.0, floor.getHeight()/2.0, 290/lightZposCheat);
    softboxBack->setOrientation(ofQuaternion(135,ofVec3f(0,0,1)));
    lights.push_back(softboxBack);

    LupoLed* softboxRight = new LupoLed();
    softboxRight->setup(207);
    softboxRight->setParent(floor);
    softboxRight->setPosition(-floor.getWidth()/2.0,floor.getHeight()/2.0, 290/lightZposCheat);
    softboxRight->setOrientation(ofQuaternion(225,ofVec3f(0,0,1)));
    lights.push_back(softboxRight);

    /*
        // Add balls

        ChromaWhiteBall * ballLeft = new ChromaWhiteBall();
        ballLeft->setup(301);
        ballLeft->setParent(floor);
        ballLeft->setPosition(-floor.getWidth()/4.0, floor.getHeight()/5.0, 260/lightZposCheat);
        lights.push_back(ballLeft);

        ChromaWhiteBall * ballRight = new ChromaWhiteBall();
        ballRight->setup(303);
        ballRight->setParent(floor);
        ballRight->setPosition(floor.getWidth()/4.0, floor.getHeight()/5.0, 260/lightZposCheat);
        lights.push_back(ballRight);
    */

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
    ofxUIRangeSlider * rTemp = gui->addRangeSlider("tRange", kelvinWarm, kelvinCold, &kelvinWarmRange, &kelvinColdRange, gui->getRect()->getWidth()-8, 15);
    rTemp->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rTemp);
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpeed = gui->addSlider("tSpeed",0,1,&temperatureSpeed, gui->getRect()->getWidth()-8, 15);
    sTempSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpeed);
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sTempSpread = gui->addSlider("tSpread",0,0.33,&temperatureSpread, gui->getRect()->getWidth()-8, 15);
    sTempSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sTempSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Brightness", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Range", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rBrightness = gui->addRangeSlider("bRange", 0, 1, &brightnessRangeFrom, &brightnessRangeTo, gui->getRect()->getWidth()-8, 15);
    rBrightness->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rBrightness);
    guiWidgets.push_back(gui->addLabel("Speed", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpeed = gui->addSlider("bSpeed",0,1,&brightnessSpeed, gui->getRect()->getWidth()-8, 15);
    sBrightnessSpeed->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpeed);
    guiWidgets.push_back(gui->addLabel("Spread", OFX_UI_FONT_SMALL));
    ofxUISlider * sBrightnessSpread = gui->addSlider("bSpread",0,0.33,&brightnessSpread, gui->getRect()->getWidth()-8, 15);
    sBrightnessSpread->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sBrightnessSpread);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Manual", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Balance", OFX_UI_FONT_SMALL));
    ofxUISlider * sManualBalance = gui->addSlider("mBalance",0.0,1.0,&manualBalance, gui->getRect()->getWidth()-8, 15);
    sManualBalance->setColorBack(ofColor(48,48,48));
    sManualBalance->setColorFillHighlight(ofColor(255,0,0));
    sManualBalance->setColorOutlineHighlight(ofColor(255,0,0));
    guiWidgets.push_back(sManualBalance);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Position", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Temperature", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rTempPos = gui->addRangeSlider("tRangePos", kelvinWarm, kelvinCold, &kelvinWarmRangePos, &kelvinColdRangePos, gui->getRect()->getWidth()-8, 15);
    rTempPos->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rTempPos);
    guiWidgets.push_back(gui->addLabel("Brightness", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rBrightnessPos = gui->addRangeSlider("bRangePos", 0, 1, &brightnessRangeFromPos, &brightnessRangeToPos, gui->getRect()->getWidth()-8, 15);
    rBrightnessPos->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rBrightnessPos);
    guiWidgets.push_back(gui->addLabel("Size", OFX_UI_FONT_SMALL));
    ofxUISlider * sPosSize = gui->addSlider("pSize",0.0,1.0,&posSize, gui->getRect()->getWidth()-8, 15);
    sPosSize->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(sPosSize);
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Direction", OFX_UI_FONT_LARGE));
    guiWidgets.push_back(gui->addSpacer());
    guiWidgets.push_back(gui->addLabel("Temperature", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rTempDir = gui->addRangeSlider("tRangeDir", kelvinWarm, kelvinCold, &kelvinWarmRangeDir, &kelvinColdRangeDir, gui->getRect()->getWidth()-8, 15);
    rTempPos->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rTempDir);
    guiWidgets.push_back(gui->addLabel("Brightness", OFX_UI_FONT_SMALL));
    ofxUIRangeSlider * rBrightnessDir = gui->addRangeSlider("bRangeDir", 0, 1, &brightnessRangeFromDir, &brightnessRangeToDir, gui->getRect()->getWidth()-8, 15);
    rBrightnessPos->setColorBack(ofColor(48,48,48));
    guiWidgets.push_back(rBrightnessDir);
}

void darknessFollowsScene::update()
{
    double temperatureSpreadCubic = powf(temperatureSpread, 3);
    double brightnessSpreadCubic = powf(brightnessSpread, 3);

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); it++)
    {
        LedFixture * shd = *(it);

        ofVec3f vecOnFloor(shd->getGlobalPosition().x,shd->getGlobalPosition().y,floor.getGlobalPosition().z);

        float distToFloorPos = vecOnFloor.distance(floorPos.getGlobalPosition());
        float distToFloorPosNormalised = distToFloorPos/floor.getWidth();

        float bRangeFrom = brightnessRangeFrom;
        float bRangeTo = brightnessRangeTo;

        float tRangeWarm = kelvinWarmRange;
        float tRangeCold = kelvinColdRange;

        if(distToFloorPosNormalised < posSize)
        {

            float posWeighing = ofClamp(ofMap(distToFloorPosNormalised, posSize/2.0, posSize, 1.0, 0.0) ,0.0,1.0);

            bRangeFrom *= (1.0-posWeighing);
            bRangeFrom += brightnessRangeFromPos*posWeighing;

            bRangeTo *= (1.0-posWeighing);
            bRangeTo += brightnessRangeToPos*posWeighing;

            tRangeWarm *= (1.0-posWeighing);
            tRangeWarm += kelvinWarmRangePos*posWeighing;

            tRangeCold *= (1.0-posWeighing);
            tRangeCold += kelvinColdRangePos*posWeighing;
        }
        if(shd->directional)
        {

            ofVec3f dirVecLight(0,1,0);
            ofVec3f dirVecPos(0,1,0);
            dirVecLight = dirVecLight * shd->getGlobalOrientation();
            dirVecPos = dirVecPos * floorPos.getGlobalOrientation();

            float angleA = atan2(dirVecLight.x, dirVecLight.y) * 180 / PI;
            float angleB = atan2(dirVecPos.x, dirVecPos.y) * 180 / PI;

            float angleDiff;

            if (angleA < 0){
                angleDiff = angleB - angleA;
            } else {
                angleDiff = angleA - angleB;
            }

            angleDiff = fmodf((angleDiff + 180.0), 360.0) - 180.0;

            float dirWeighing = ofMap(fabs(angleDiff),0.0,180.0,0.0,1.0);

            bRangeFrom *= (1.0-dirWeighing);
            bRangeFrom += brightnessRangeFromDir*dirWeighing;

            bRangeTo *= (1.0-dirWeighing);
            bRangeTo += brightnessRangeToDir*dirWeighing;

            tRangeWarm *= (1.0-dirWeighing);
            tRangeWarm += kelvinWarmRangeDir*dirWeighing;

            tRangeCold *= (1.0-dirWeighing);
            tRangeCold += kelvinColdRangeDir*dirWeighing;

        }

        float tempNoise = ofNoise(shd->getPosition().x*temperatureSpreadCubic, shd->getPosition().y*temperatureSpreadCubic, temperatureTime);
        unsigned int temperature = round(ofMap(tempNoise, 0, 1, fmaxf(shd->temperatureRangeWarmKelvin, tRangeWarm), fminf(shd->temperatureRangeColdKelvin, tRangeCold)));

        float brightness = ofNoise(shd->getPosition().x*brightnessSpreadCubic, shd->getPosition().y*brightnessSpreadCubic, brightnessTime);
        brightness = ofMap(brightness, 0, 1, bRangeFrom, bRangeTo);

        if(shd->manual || shd->selected )
        {
            int balancedTemperature((manualBalance*shd->manualTemperature)+(temperature*(1.0-manualBalance)));
            float balancedBrightness((manualBalance*shd->manualBrightness)+(brightness*(1.0-manualBalance)));
            shd->setTemperature(balancedTemperature);
            shd->setNormalisedBrightness(balancedBrightness);
        }
        else
        {
            shd->setTemperature(temperature);
            shd->setNormalisedBrightness(brightness);
        }
    }

    cout << endl;

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
    floorPos.transformGL();
    ofSetColor(255,255,0,127);
    ofDrawArrow(ofVec3f(0,0,0), ofVec3f(0,50,0), 20);
    ofDrawAxis(60);
    ofDisableDepthTest();
    ofSetColor(255,255,0,32);
    ofEllipse(0,0,160,160);
    ofEnableDepthTest();
    floorPos.restoreTransformGL();
    ofxOlaShaderLight::begin();
    ofxOlaShaderLight::setMaterial(ballMaterial);
    ofDrawSphere(floorPos.getGlobalPosition(), 30);
    ofxOlaShaderLight::end();
    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        if(shd->selected || shd->manual || shd->directional)
        {
            ofPushStyle();
            ofEnableDepthTest();
            glDepthMask(GL_FALSE);
            ofPushMatrix();
            if(shd->selected)
            {
                ofSetColor(0,255,0, 255);
            }
            if(shd->manual)
            {
                ofSetColor(255,0,0, 255);
            }
            if(shd->directional)
            {
                ofSetColor(0,0,255, 255);
            }
            if(shd->manual && shd->selected)
            {
                ofSetColor(255,255,0, 255);
            }
            if(shd->directional && shd->selected)
            {
                ofSetColor(0,255,255, 255);
            }
            ofVec3f v = shd->getGlobalPosition();
            v.z *= lightZposCheat;
            v = cam.worldToScreen(v);
            v+= ofVec3f(0,0,-0.0075);
            v = cam.screenToWorld(v);
            v-= shd->getGlobalPosition();
            ofTranslate(v);
            shd->drawForm();
            ofPopMatrix();
            glDepthMask(GL_TRUE);
            ofPopStyle();
        }

        shd->LedFixture::draw();

    }
    ofDisableLighting();
    ofPopStyle();
    cam.end();
    ofDisableDepthTest();
    ofViewport();

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        ofVec3f v = cam.worldToScreen((shd->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        string s(ofToString(shd->DMXstartAddress));

        ofSetColor(64, 64, 64,127);
        font.drawString(s,v.x-(font.stringWidth(s)/2.0),v.y+(font.stringHeight(s)/2.0));

        if(shd->selected && shd->manual)
        {
            ofFloatColor fromTemp(DMXfixture::temperatureToColor(shd->temperatureRangeWarmKelvin));
            ofFloatColor toTemp(DMXfixture::temperatureToColor(shd->temperatureRangeColdKelvin));
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
                ofMap(shd->getTemperature(), shd->temperatureRangeWarmKelvin, shd->temperatureRangeColdKelvin, colorPickerRadius, -colorPickerRadius),
                ofMap(shd->getNormalisedBrightness(),0, 1, -colorPickerRadius, colorPickerRadius),
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

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        ofVec3f v = cam.worldToScreen((shd->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(v.distance(mouseVec) < 30)
        {
            shd->selected = true;
            oneIsSelected = true;
            shd->manualTemperature = shd->getTemperature();
            shd->manualBrightness = shd->getNormalisedBrightness();
        }
        else
        {
            shd->selected = false;
        }
    }
    if(oneIsSelected)
    {
//        cam.disableMouseInput();
    }
    else
    {
//        cam.enableMouseInput();
    }

}

void darknessFollowsScene::mousePressed(int x, int y, int button)
{

    mouseVec = ofVec3f(x,y,0);
    millisLastClick = ofGetElapsedTimeMillis();

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        ofVec3f v = cam.worldToScreen((shd->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
    }

}

void darknessFollowsScene::mouseReleased(int x, int y, int button)
{

    mouseVec = ofVec3f(x,y,0);

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        ofVec3f v = cam.worldToScreen((shd->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(shd->selected)
        {
            if(button == 0)
            {
                if(shd->manual && ofGetElapsedTimeMillis() - millisLastClick < 500 )
                {
                    shd->manual = false;
                }
                else
                {
                    shd->manual = true;
                    shd->directional = false;
                }
            }
            if(button == 2)
            {
                if(shd->directional && ofGetElapsedTimeMillis() - millisLastClick < 500 )
                {
                    shd->directional = false;
                }
                else
                {
                    shd->directional = true;
                    shd->manual = false;
                }
            }
        }
    }

}

void darknessFollowsScene::mouseDragged(int x, int y, int button)
{

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * shd = *(it);
        ofVec3f v = cam.worldToScreen((shd->getGlobalPosition()*ofVec3f(1.,1.,lightZposCheat*1.)));
        v += ofVec3f(gui->getRect()->getWidth()/2.,0,0);
        if(shd->selected)
        {

            //TODO: make temperature navigation

            float manualTemperatureNormalised = ofMap(shd->manualTemperature, shd->temperatureRangeWarmKelvin, shd->temperatureRangeColdKelvin, 0.0, 1.0);
            manualTemperatureNormalised = ofClamp(manualTemperatureNormalised + ((x-mouseVec.x)/(colorPickerRadius*2.0)), 0.0, 1.0);

            shd->manualTemperature = ofMap(manualTemperatureNormalised,0.0,1.0,shd->temperatureRangeWarmKelvin, shd->temperatureRangeColdKelvin);
            shd->manualBrightness = ofClamp(shd->manualBrightness + ((mouseVec.y-y)/(colorPickerRadius*2.0)),0.0,1.0);
        }
    }
    mouseVec = ofVec3f(x,y,0);

    ofVec3f mouseWithDepth(mouseVec);
    mouseWithDepth.z = cam.worldToScreen(floor.getGlobalPosition()).z;
    mouseWithDepth.x -= gui->getRect()->getHalfWidth();
    ofVec3f mouseInCam(cam.screenToWorld(mouseWithDepth));

    float halfWidth = floor.getWidth()/2.0;
    float halfHeight = floor.getHeight()/2.0;

    ofVec3f P1(floor.getGlobalPosition().x-halfWidth, floor.getGlobalPosition().y-halfHeight, floor.getGlobalPosition().z);
    ofVec3f P2(floor.getGlobalPosition().x-halfWidth, floor.getGlobalPosition().y+halfHeight, floor.getGlobalPosition().z);
    ofVec3f P3(floor.getGlobalPosition().x+halfWidth, floor.getGlobalPosition().y+halfHeight, floor.getGlobalPosition().z);
    ofVec3f R1(cam.getGlobalPosition());
    ofVec3f R2(mouseInCam);

    // Returns in (fX, fY) the location on the plane (P1,P2,P3) of the intersection with the ray (R1, R2)
    // First compute the axes
    ofVec3f V1(P2 - P1);
    ofVec3f V2(P3 - P1);
    ofVec3f V3(V1.getCrossed(V2));

    // Project ray points R1 and R2 onto the axes of the plane. (This is equivalent to a rotation.)
    ofVec3f vRotRay1( V1.dot( R1-P1 ), V2.dot( R1-P1 ), V3.dot( R1-P1 ) );
    ofVec3f vRotRay2( V1.dot( R2-P1 ), V2.dot( R2-P1 ), V3.dot( R2-P1 ) );
    // Return now if ray will never intersect plane (they're parallel)
    if (vRotRay1.z != vRotRay2.z)
    {
        float fPercent = vRotRay1.z / (vRotRay2.z-vRotRay1.z);
        ofVec3f rayVec(R1 + (R1-R2) * fPercent);
        if(rayVec.distance(floorPos.getGlobalPosition()) < 20)
        {
            floorPos.setGlobalPosition(rayVec);

        }
        else if(rayVec.distance(floorPos.getGlobalPosition()) < 80)
        {
            ofNode n(floorPos);
            n.lookAt(rayVec, ofVec3f(0,0,1));
            n.tilt(-90);

            ofQuaternion q = floorPos.getGlobalOrientation();
            q.slerp(0.5, q, n.getGlobalOrientation());
            floorPos.setGlobalOrientation(q);
//            floorPos.setGlobalPosition(floorPos.lerp rayVec);
        }
    }

}

void darknessFollowsScene::saveSettings(ofFile f)
{
    settings.clear();
    settings.addTag("lights");
    settings.pushTag("lights");
    int i = 0;
    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
    {
        LedFixture * l = *(it);
        settings.addTag("light");
        settings.pushTag("light", i++);
        settings.addValue("DMXstartAddress", l->DMXstartAddress);
        settings.addValue("manual", l->manual);
        settings.addValue("directional", l->directional);
        settings.addValue("manualBrightness", l->manualBrightness);
        settings.addValue("manualTemperature", l->manualTemperature);
        settings.popTag(); // light
    }
    settings.popTag(); // lights
    settings.saveFile(f.getAbsolutePath());
}

void darknessFollowsScene::loadSettings(ofFile f)
{
    settings.loadFile(f.getAbsolutePath());
    settings.pushTag("lights");

    int numberOfSavedLights = settings.getNumTags("light");
    for(int i = 0; i < numberOfSavedLights; i++)
    {
        settings.pushTag("light", i);
        for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); ++it)
        {
            LedFixture * l = *(it);
            if(l->DMXstartAddress == settings.getValue("DMXstartAddress",0))
            {
                l->manual = settings.getValue("manual",0);
                l->directional = settings.getValue("directional",0);
                l->manualBrightness = settings.getValue("manualBrightness",0);
                l->manualTemperature = settings.getValue("manualTemperature",0);
            }
        }
        settings.popTag(); // light
    }

    settings.popTag(); // lights
}
