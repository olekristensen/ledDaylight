#include "daylightScene.h"
#include <math.h>       /* atan2 */

daylightScene::daylightScene()
{
    //ctor
}

daylightScene::~daylightScene()
{
    for(unsigned int i = 0; i < cameras.size(); i++)
    {
        cameras[i]->StopCapture();
        cameras[i]->Disconnect();
        delete rawImages[i];
        delete images[i];
        delete cameras[i];
    }
}


void daylightScene::setup()
{

    ofTrueTypeFont::setGlobalDpi(72);

    font.loadFont("GUI/DroidSans.ttf", 14, true, true);

    name = "Daylight";

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
    boxPos.setOrientation(ofQuaternion(-90,ofVec3f(0,0,1)));

    manualBalance = 1.0;

    box.set(285,300,100,100);

    white.diffuseColor = ofVec4f(255,255,255,255);

    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 4096; j++)
        {
            absShutterVals[i][j] = 0.0;
        }
    }

    setupCams();

}

int daylightScene::setupCams(){
    BusManager busMgr;
    //catchError(busMgr.ForceAllIPAddressesAutomatically());
    unsigned int numCameras;
    catchError(busMgr.GetNumOfCameras(&numCameras));
    printf("found %u cameras", numCameras);
    if(numCameras < 1) return 0;
    for(unsigned int i = 0; i < numCameras; i++)
    {
        GigECamera* camera = new GigECamera();
        Image* rawImage = new Image();

        PGRGuid guid;
        CameraInfo camInfo;

        Error error;

        error = busMgr.GetCameraFromIndex(i, &guid);
        if(error == PGRERROR_OK)
        {
            error = camera->Connect(&guid);
            if(error == PGRERROR_OK)
            {
                error = camera->GetCameraInfo(&camInfo);
                if(error == PGRERROR_OK)
                {
                    printInfo(camInfo);

                    unsigned int numStreamChannels = 0;
                    catchError(camera->GetNumStreamChannels( &numStreamChannels ));
                    for (unsigned int j=0; j < numStreamChannels; j++)
                    {
                        GigEStreamChannel streamChannel;
                        catchError(camera->GetGigEStreamChannelInfo( j, &streamChannel ));
                        printf( "\nPrinting stream channel information for channel %u:\n", j );
                        PrintStreamChannelInfo( &streamChannel );
                    }

                    printf( "Querying GigE image setting information...\n" );

                    GigEImageSettingsInfo imageSettingsInfo;
                    catchError(camera->GetGigEImageSettingsInfo( &imageSettingsInfo ));

                    GigEImageSettings imageSettings;
                    imageSettings.offsetX = (imageSettingsInfo.maxWidth-camWidth)/2;
                    imageSettings.offsetY = (imageSettingsInfo.maxHeight-camHeight)/2;
                    imageSettings.height = camHeight;
                    imageSettings.width = camWidth;
                    imageSettings.pixelFormat = k_PixFmt;

                    printf( "Setting GigE image settings...\n" );

                    catchError(camera->SetGigEImageSettings( &imageSettings ));

                    GigEProperty packetSizeProp;
                    packetSizeProp.propType = PACKET_SIZE;
                    packetSizeProp.value = static_cast<unsigned int>(9000);
                    catchError(camera->SetGigEProperty(&packetSizeProp));

                    GigEProperty packetDelayProp;
                    packetDelayProp.propType = PACKET_DELAY;
                    packetDelayProp.value = static_cast<unsigned int>(1800);
                    catchError(camera->SetGigEProperty(&packetDelayProp));

                    printf( "Setting embeddedInfo...\n" );

                    EmbeddedImageInfo embeddedInfo;
                    catchError(camera->GetEmbeddedImageInfo( &embeddedInfo ));

                    embeddedInfo.timestamp.onOff = true;
                    embeddedInfo.gain.onOff = true;
                    embeddedInfo.shutter.onOff = true;
                    embeddedInfo.brightness.onOff = true;
                    embeddedInfo.exposure.onOff = true;
                    embeddedInfo.whiteBalance.onOff = true;
                    embeddedInfo.frameCounter.onOff = true;
                    embeddedInfo.strobePattern.onOff = true;
                    embeddedInfo.GPIOPinState.onOff = true;
                    embeddedInfo.ROIPosition.onOff = true;
                    catchError(camera->SetEmbeddedImageInfo( &embeddedInfo ));

                    printf( "Setting framerate...\n" );

                    Property framerate(FRAME_RATE);
                    framerate.onOff = true;
                    framerate.absControl = true;
                    framerate.absValue = 12.0; // 2076 is 12 fps
                    framerate.autoManualMode = false;
                    catchError(camera->SetProperty(&framerate));

                    /** STARTING HERE **/

                    // Get current trigger settings

                    printf( "Setting trigger...\n" );

                    TriggerMode triggerMode;
                    catchError(camera->GetTriggerMode( &triggerMode ));
                    // Set camera to trigger mode 0
                    triggerMode.onOff = false;
                    triggerMode.mode = 0;
                    triggerMode.parameter = 0;
                    triggerMode.source = 7;

                    catchError(camera->SetTriggerMode( &triggerMode ));

                    catchError(camera->StartCapture());

                    printf( "Setting brightness...\n" );

                    Property brightness(BRIGHTNESS);
                    brightness.autoManualMode = false;
                    brightness.absControl = true;
                    brightness.absValue = 0.0;
                    catchError(camera->SetProperty(&brightness));

                    printf( "Setting exposure...\n" );

                    Property autoExposure(AUTO_EXPOSURE);
                    autoExposure.onOff = true;
                    autoExposure.autoManualMode = false;
                    autoExposure.absControl = true;
                    autoExposure.absValue = 1.0;
                    catchError(camera->SetProperty(&autoExposure));

                    printf( "Setting gamma...\n" );

                    Property gamma(GAMMA);
                    gamma.onOff = true;
                    gamma.autoManualMode = false;
                    gamma.absControl = true;
                    gamma.absValue = 1.0;
                    catchError(camera->SetProperty(&gamma));

                    printf( "Setting shutter...\n" );

                    Property shutter(SHUTTER);
                    shutter.absControl = false;
//                    shutter.absValue = cameraSettings.getValue("shutter", 1.0); // ms
                    shutter.autoManualMode = true;
                    shutter.onOff = true;
                    catchError(camera->SetProperty(&shutter));

                    printf( "Setting gain...\n" );

                    Property gain(GAIN);
                    gain.absValue = 3.0;
                    gain.autoManualMode = false;
                    gain.absControl = true;
                    gain.onOff = true;
                    catchError(camera->SetProperty(&gain));

                    printf( "Setting whitebalance...\n" );

                    Property whitebalance(WHITE_BALANCE);
                    whitebalance.valueA = 512;
                    whitebalance.valueB = 512;
                    whitebalance.autoManualMode = true;
                    whitebalance.onOff = true;
                    catchError(camera->SetProperty(&whitebalance));

                    cameras.push_back(camera);
                    rawImages.push_back(rawImage);

                    ofxCvColorImage* image = new ofxCvColorImage();
                    image->allocate(camWidth, camHeight);
                    images.push_back(image);


                    printf( "Building absolute exposure table ...\n" );

                    Property prop;
                    prop.type = SHUTTER;

                    prop.autoManualMode = false;
                    prop.absControl = true;

                    // Read register for relative shutter

                    const unsigned int shutter_rel = 0x81C;
                    unsigned int shutter_Rel = 0;

                    camera->ReadRegister(shutter_rel, &shutter_Rel);

                    // Loop through all possible relative shutter values. Save relative and corresponding absolute shutter

                    for(unsigned int shutter_count = 1; shutter_count <= 4096; shutter_count ++)
                    {

                        camera->WriteRegister(shutter_rel,shutter_count);

                        error = camera->GetProperty( &prop );
                        if (error != PGRERROR_OK)
                        {
                            catchError( error );
                        }

                        camera->ReadRegister( shutter_rel, &shutter_Rel );

                        // Print to Excel File
                        absShutterVals[i][(shutter_Rel & 0x0000FFFF)-1] = prop.absValue;
                        printf(" %d, %.2f\n", (shutter_Rel & 0x0000FFFF), prop.absValue);

                    }

                }
                else
                {
                    catchError(error);
                }
            }
            else
            {
                catchError(error);
            }

        }
        else
        {
            catchError(error);
        }
        return numCameras;
    }
}

void daylightScene::setGUI(ofxUISuperCanvas* gui)
{
    this->gui = gui;

    /*    guiWidgets.push_back(gui->addLabel("Temperature", OFX_UI_FONT_LARGE));
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
        ofxUILabelToggle * tWallsInsteadOfCorners = gui->addLabelToggle("Show Walls Instead of Corners", &bWallsInsteadOfCorners, gui->getRect()->getWidth()-8, 15);
        tWallsInsteadOfCorners->setColorBack(ofColor(48,48,48));
        guiWidgets.push_back(tWallsInsteadOfCorners);
    */
}

void daylightScene::update()
{

    if(cameras.size() < 2){
        setupCams();
    }

    imagesMetadata.clear();

    for(unsigned int i = 0; i < cameras.size(); i++)
    {
        GigECamera& camera = *(cameras[i]);

        shutterWasSetToAbs = pow(1000./(fmodf(ofGetElapsedTimef()*10, 2000.0)+5),2);
        Image tempImage;
        Property shutter(SHUTTER);
        shutter.absValue = shutterWasSetToAbs;
        shutter.autoManualMode = false;
        shutter.absControl = true;
        shutter.onOff = true;
        catchError(camera.SetProperty(&shutter));

//            PollForTriggerReady(&camera);

        // Fire software trigger
        /*            bool retVal = FireSoftwareTrigger( &camera );
                    if ( !retVal )
                    {
                        printf("\nError firing software trigger!\n");
                    }
        */
        Error error = camera.RetrieveBuffer(&tempImage);
        if(error == PGRERROR_OK)
        {
            imagesMetadata.push_back(tempImage.GetMetadata());
            rawImages[i] = &tempImage;

            PixelFormat pixFormat;
            unsigned int rows, cols, stride;
            tempImage.GetDimensions( &rows, &cols, &stride, &pixFormat );
            tempImage.SetColorProcessing(NEAREST_NEIGHBOR);
            // Create a converted image
            Image convertedImage;

            // Convert the raw image
            catchError(tempImage.Convert( PIXEL_FORMAT_RGB8, &convertedImage ));

            memcpy(images[i]->getPixels(), convertedImage.GetData(), camWidth * camHeight  * 3);
            images[i]->flagImageChanged();

        }
        else
        {
            catchError(error);
        }
    }


    double temperatureSpreadCubic = powf(temperatureSpread, 3);
    double brightnessSpreadCubic = powf(brightnessSpread, 3);

    for(vector<LedFixture*>::iterator it = lights.begin(); it != lights.end(); it++)
    {
        LedFixture * shd = *(it);

        ofVec3f vecOnBox(shd->getGlobalPosition().x,shd->getGlobalPosition().y,box.getGlobalPosition().z);

        float distToBoxPos = vecOnBox.distance(boxPos.getGlobalPosition());
        float distToBoxPosNormalised = distToBoxPos/box.getWidth();

        float bRangeFrom = brightnessRangeFrom;
        float bRangeTo = brightnessRangeTo;

        float tRangeWarm = kelvinWarmRange;
        float tRangeCold = kelvinColdRange;

        if(distToBoxPosNormalised < posSize)
        {

            float posWeighing = ofClamp(ofMap(distToBoxPosNormalised, posSize/2.0, posSize, 1.0, 0.0) ,0.0,1.0);

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
            dirVecPos = dirVecPos * boxPos.getGlobalOrientation();

            float angleA = atan2(dirVecLight.x, dirVecLight.y) * 180 / PI;
            float angleB = atan2(dirVecPos.x, dirVecPos.y) * 180 / PI;

            float angleDiff;

            if (angleA < 0)
            {
                angleDiff = angleB - angleA;
            }
            else
            {
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

    //ofxOlaShaderLight::update();

    float now = ofGetElapsedTimef() + timeOffset;
    temperatureTime += powf(temperatureSpeed,8) * ( ( now - lastFrameSeconds ) / (1./60));
    brightnessTime += powf(brightnessSpeed,8) * ( ( now - lastFrameSeconds ) / (1./60));
    lastFrameSeconds = now;
}

void daylightScene::draw()
{
//    ofPushMatrix();
//    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);


    ofPushMatrix();
    float scaleFactor = ofGetWidth()/(camWidth*2.0);
    float scaleFactorY = ofGetHeight()/(camHeight*1.0);

    ofScale(scaleFactor, scaleFactor, scaleFactor);

    ofSetColor(255,255);
    for(unsigned int i = 0; i < cameras.size(); i++)
    {
        images[i]->draw(0, 0);

        ImageMetadata metadata = imagesMetadata[i];

        Property shutter(SHUTTER);
        catchError(cameras[i]->GetProperty(&shutter));

        ofDrawBitmapString("Image Metadata", 10, camHeight+200);
        ofDrawBitmapString(ofToString(metadata.embeddedShutter), 10, camHeight +40);
        ofDrawBitmapString(ofToString(absShutterVals[i][(metadata.embeddedShutter & 0x0000FFFF)-i]), 10, camHeight +80);
        ofDrawBitmapString(ofToString(shutterWasSetToAbs), 10, camHeight +120);
        ofDrawBitmapString(ofToString(metadata.embeddedShutter), 10, camHeight +160);

        ofDrawBitmapString("Camera Metadata", 300, camHeight+200);
        ofDrawBitmapString(ofToString(shutter.valueA), 300, camHeight +40);
        ofDrawBitmapString(ofToString(absShutterVals[i][(shutter.valueA & 0x0000FFFF)-1]), 300, camHeight +80);
        ofDrawBitmapString(ofToString(shutter.absValue), 300, camHeight +120);
        ofDrawBitmapString(ofToString(shutter.valueA), 300, camHeight +160);

        ofTranslate(camWidth, 0);
    }

    if(cameras.size() < 2){
                ofSetColor(255,(255.0*sin(ofGetElapsedTimef()*0.5)+0.5));
                ofDrawBitmapString("Looking for cameras", 100, 100);
    }

    ofPopMatrix();




    ofPushStyle();
    ofxOlaShaderLight::begin();
    ofxOlaShaderLight::setMaterial(white);

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
}

void daylightScene::mouseMoved(int x, int y)
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

void daylightScene::mousePressed(int x, int y, int button)
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

void daylightScene::mouseReleased(int x, int y, int button)
{

    lerpBoxPosToDest = false;

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

void daylightScene::mouseDragged(int x, int y, int button)
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
    mouseWithDepth.z = cam.worldToScreen(box.getGlobalPosition()).z;
    mouseWithDepth.x -= gui->getRect()->getHalfWidth();
    ofVec3f mouseInCam(cam.screenToWorld(mouseWithDepth));

    float halfWidth = box.getWidth()/2.0;
    float halfHeight = box.getHeight()/2.0;

    ofVec3f P1(box.getGlobalPosition().x-halfWidth, box.getGlobalPosition().y-halfHeight, box.getGlobalPosition().z);
    ofVec3f P2(box.getGlobalPosition().x-halfWidth, box.getGlobalPosition().y+halfHeight, box.getGlobalPosition().z);
    ofVec3f P3(box.getGlobalPosition().x+halfWidth, box.getGlobalPosition().y+halfHeight, box.getGlobalPosition().z);
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
        if(rayVec.distance(boxPos.getGlobalPosition()) < 30)
        {
            boxPos.setGlobalPosition(rayVec);

        }
        else if(rayVec.distance(boxPos.getGlobalPosition()) < 80)
        {
            ofNode n(boxPos);
            n.lookAt(rayVec, ofVec3f(0,0,1));
            n.tilt(-90);
            lerpBoxPosToDest = true;
            boxDest = n;
//            boxPos.setGlobalPosition(BoxPos.lerp(0.1, rayVec));
        }
    }

}

void daylightScene::saveSettings(ofFile f)
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

void daylightScene::loadSettings(ofFile f)
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
                l->manualBrightness = settings.getValue("manualBrightness",0.0);
                cout << l->manualBrightness << " from " << settings.getValue("manualBrightness", 0.0) << endl;
                l->manualTemperature = settings.getValue("manualTemperature",0);
            }
        }
        settings.popTag(); // light
    }

    settings.popTag(); // lights
}
