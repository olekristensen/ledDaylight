#ifndef CAMERACONTROLLER_H_INCLUDED
#define CAMERACONTROLLER_H_INCLUDED

#pragma once

#include "FlyCapture2.h"
#include "ofxXmlSettings.h"
#include "CvUtilities.h"
#include <opencv2/photo.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace ofxCv;
using namespace std;

#include "ofMain.h"

using namespace FlyCapture2;

class blackflyThreadedCamera : public ofThread
{
public:
    static const int width = 1280;
    static const int height = 720;

    ofTrueTypeFont font;

    const PixelFormat k_PixFmt = PIXEL_FORMAT_RAW12;

    GigECamera* camera;
    CameraInfo cameraInfo;
    Image* rawImageCurrent;
    Image* rawImageNext;
    Image* rawImageBuffers[2];

    ofxXmlSettings settings;
    ofImage colorImage;
    ofImage fusionImage;
    ofImage tonemappedImage;
    FlyCapture2::Image convertedImage;

    bool updateHdrOnMainThread;

    Mat hdrImage;

    vector<Mat> ldrImages;
    vector<float> times;

    float absShutterVals[4096];
    unsigned int absShutterValsRead;

    enum CameraState { NEW, SETUP, CALIBRATING, RUNNING, CLOSING };

    CameraState state = NEW;

    blackflyThreadedCamera()
    {
        state = NEW;
        updateHdrOnMainThread = false;
        font.loadFont("GUI/DroidSans.ttf", 42, true, true, true);
        rawImageBuffers[0] = new Image();
        rawImageBuffers[1] = new Image();
        rawImageCurrent = rawImageBuffers[0];
        rawImageNext = rawImageBuffers[1];
        absShutterValsRead = 0;
    }

    int catchError(FlyCapture2::Error error)
    {
        if(error == PGRERROR_OK)
        {
            return 0;
        }
        else
        {
            error.PrintErrorTrace();
            return -1;
        }
    }

    float getAbsShutterFromEmbeddedShutter(unsigned int embeddedShutter)
    {
        if(embeddedShutter > 0)
            return absShutterVals[(embeddedShutter & 0x0000FFFF)-1];
        else
            return 0;
    }

    void printInfo(const CameraInfo& cameraInfo)
    {
        char macAddress[64];
        sprintf(
            macAddress,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            cameraInfo.macAddress.octets[0],
            cameraInfo.macAddress.octets[1],
            cameraInfo.macAddress.octets[2],
            cameraInfo.macAddress.octets[3],
            cameraInfo.macAddress.octets[4],
            cameraInfo.macAddress.octets[5]);

        char ipAddress[32];
        sprintf(
            ipAddress,
            "%u.%u.%u.%u",
            cameraInfo.ipAddress.octets[0],
            cameraInfo.ipAddress.octets[1],
            cameraInfo.ipAddress.octets[2],
            cameraInfo.ipAddress.octets[3]);

        char subnetMask[32];
        sprintf(
            subnetMask,
            "%u.%u.%u.%u",
            cameraInfo.subnetMask.octets[0],
            cameraInfo.subnetMask.octets[1],
            cameraInfo.subnetMask.octets[2],
            cameraInfo.subnetMask.octets[3]);

        char defaultGateway[32];
        sprintf(
            defaultGateway,
            "%u.%u.%u.%u",
            cameraInfo.defaultGateway.octets[0],
            cameraInfo.defaultGateway.octets[1],
            cameraInfo.defaultGateway.octets[2],
            cameraInfo.defaultGateway.octets[3]);

        printf(
            "\n*** CAMERA INFORMATION ***\n"
            "Serial number - %u\n"
            "Camera model - %s\n"
            "Camera vendor - %s\n"
            "Sensor - %s\n"
            "Resolution - %s\n"
            "Firmware version - %s\n"
            "Firmware build time - %s\n"
            "GigE version - %u.%u\n"
            "User defined name - %s\n"
            "XML URL 1 - %s\n"
            "XML URL 2 - %s\n"
            "MAC address - %s\n"
            "IP address - %s\n"
            "Subnet mask - %s\n"
            "Default gateway - %s\n\n",
            cameraInfo.serialNumber,
            cameraInfo.modelName,
            cameraInfo.vendorName,
            cameraInfo.sensorInfo,
            cameraInfo.sensorResolution,
            cameraInfo.firmwareVersion,
            cameraInfo.firmwareBuildTime,
            cameraInfo.gigEMajorVersion,
            cameraInfo.gigEMinorVersion,
            cameraInfo.userDefinedName,
            cameraInfo.xmlURL1,
            cameraInfo.xmlURL2,
            macAddress,
            ipAddress,
            subnetMask,
            defaultGateway );
    }

    void PrintStreamChannelInfo( GigEStreamChannel* pStreamChannel )
    {
        char ipAddress[32];
        sprintf(
            ipAddress,
            "%u.%u.%u.%u",
            pStreamChannel->destinationIpAddress.octets[0],
            pStreamChannel->destinationIpAddress.octets[1],
            pStreamChannel->destinationIpAddress.octets[2],
            pStreamChannel->destinationIpAddress.octets[3]);

        printf(
            "Network interface: %u\n"
            "Host post: %u\n"
            "Do not fragment bit: %s\n"
            "Packet size: %u\n"
            "Inter packet delay: %u\n"
            "Destination IP address: %s\n"
            "Source port (on camera): %u\n\n",
            pStreamChannel->networkInterfaceIndex,
            pStreamChannel->hostPost,
            pStreamChannel->doNotFragment == true ? "Enabled" : "Disabled",
            pStreamChannel->packetSize,
            pStreamChannel->interPacketDelay,
            ipAddress,
            pStreamChannel->sourcePort );
    }

    int setup(unsigned int serialNumber)
    {

        state = SETUP;

        camera = new GigECamera();

        BusManager busMgr;
        PGRGuid guid;
        FlyCapture2::Error error;

        error = busMgr.GetCameraFromSerialNumber ( serialNumber, &guid );
        if(error == PGRERROR_OK)
        {
            error = camera->Connect(&guid);
            if(error == PGRERROR_OK)
            {
                error = camera->GetCameraInfo(&cameraInfo);
                if(error == PGRERROR_OK)
                {
                    printInfo(cameraInfo);

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
                    imageSettings.offsetX = (imageSettingsInfo.maxWidth-width)/2;
                    imageSettings.offsetY = (imageSettingsInfo.maxHeight-height)/2;
                    imageSettings.height = height;
                    imageSettings.width = width;
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

                    /** STARTING CAPTURE HERE **/

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

                }
                else
                {
                    catchError(error);
                    return -1;
                }
            }
            else
            {
                catchError(error);
                return -1;
            }
        }
        else
        {
            catchError(error);
            return -1;
        }

        colorImage.allocate(width, height, OF_IMAGE_COLOR);
        fusionImage.allocate(width, height, OF_IMAGE_COLOR);
        tonemappedImage.allocate(width, height, OF_IMAGE_COLOR);

        startThread(true, false);

        return 0;

    }

    void draw()
    {
        this->lock();
        if(updateHdrOnMainThread){
                fusionImage.update();
                updateHdrOnMainThread = false;
        }
        ofPushStyle();
        ofDisableDepthTest();
        ofSetColor(127,255);
        ofRect(0,0,width, height);

        string str;
        float alpha = (.5+(sinf(ofGetElapsedTimef()*2.0)*.5));

        switch (state)
        {
        case NEW:
            break;
        case SETUP:
            ofSetColor(255.0, 255.0);
            font.drawString("Setting Up Camera...",40,80);
            break;
        case CALIBRATING:
            ofSetColor(255,255,0,255.0);
            ofRect(0,0,width*(absShutterValsRead/4096.0), height);
            ofSetColor(255,255,0,127.0);
            //ofRect(width*(absShutterValsRead/4096.0),0,width-(width*(absShutterValsRead/4096.0)), height);
            ofSetColor(0.0, 255.0);
            font.drawString("Calibrating Camera ... ",40,80);
            break;
        case RUNNING:

            //TODO: Why are the images not drawn?

            ofSetColor(255, 255);
            colorImage.update();
            colorImage.draw(0,0);
            str = "s/n: " + ofToString(cameraInfo.serialNumber);
            if(rawImageCurrent)
            {
                ImageMetadata metaData = rawImageCurrent->GetMetadata();
                str += "\nshutter: " + ofToString(getAbsShutterFromEmbeddedShutter(metaData.embeddedShutter));
            }
            fusionImage.draw(0, colorImage.getHeight());
            font.drawString(str,40,80);

            break;
        case CLOSING:
            ofSetColor(255,0,0,(.5+(sinf(ofGetElapsedTimef()*2.0)*.5))*255.0);
            ofRect(0,0,width, height);
            break;
        default:
            break;
        }
        ofPopStyle();
        this->unlock();
    }

    void threadedFunction()
    {

        state = CALIBRATING;

        string fileName = ofToDataPath("Camera-" + ofToString(cameraInfo.serialNumber) + "-absShutterVals.csv");

        if(ofFile::doesFileExist(fileName))
        {
            ofLogNotice( "Reading absolute exposure table from " + fileName + " ...\n" );
            ofFile file;
            file.open(fileName, ofFile::ReadOnly, false);
            ofBuffer buff = file.readToBuffer();
            int lineNumber = 0;
            while(!buff.isLastLine() && lineNumber <= 4096)
            {
                lock();
                absShutterVals[lineNumber++] = ofToFloat(buff.getNextLine());
                absShutterValsRead = lineNumber;
                unlock();
                yield();
            }
            file.close();
        }
        else
        {
            printf( "Building absolute exposure table ...\n" );

            Property prop;
            prop.type = SHUTTER;

            prop.autoManualMode = false;
            prop.absControl = true;

            // Read register for relative shutter

            const unsigned int shutter_rel = 0x81C;
            unsigned int shutter_Rel = 0;

            camera->ReadRegister(shutter_rel, &shutter_Rel);

            ofFile file;
            file.open(fileName, ofFile::WriteOnly, true);

            FlyCapture2::Error error;

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

                lock();
                absShutterVals[(shutter_Rel & 0x0000FFFF)-1] = prop.absValue;
                absShutterValsRead = shutter_count;
                unlock();

                file << ofToString(prop.absValue, 16) << endl;
            }
            file.close();
        }

        float lastEmbeddedAbsShutter = 0.0;

        while (isThreadRunning())
        {

            state = RUNNING;

            // SHUTTER MODULATION

            float duration = 1.5;
            float normalisedPosition = fmodf(ofGetElapsedTimef()/duration, 1.0);
            float quadPosition = pow(normalisedPosition, 4);
            float minShutterDenominator = 2000;
            float maxShutterDenominator = 12;
            float shutterAbs = ofMap(quadPosition, 0.0, 1.0, 1000./minShutterDenominator, 1000./maxShutterDenominator);
            Property shutter(SHUTTER);
            shutter.absValue = shutterAbs;
            shutter.autoManualMode = false;
            shutter.absControl = true;
            shutter.onOff = true;
            catchError(camera->SetProperty(&shutter));

            /** SOFTWARE TRIGGER

            // PollForTriggerReady(&camera);

            // Fire software trigger
            bool retVal = FireSoftwareTrigger( &camera );
            if ( !retVal )
            {
                printf("\nError firing software trigger!\n");
            }

            **/

            FlyCapture2::Error error = camera->RetrieveBuffer(rawImageNext);
            if(error == PGRERROR_OK)
            {
                //ofLogNotice() << "Frame received";
                PixelFormat pixFormat;
                unsigned int rows, cols, stride;
                rawImageNext->GetDimensions( &rows, &cols, &stride, &pixFormat );
                rawImageNext->SetColorProcessing(NEAREST_NEIGHBOR);

                // Convert the raw image
                error = rawImageNext->Convert( PIXEL_FORMAT_RGB8, &convertedImage );

                unsigned int pRows;
                unsigned int pCols;
                unsigned int pStride;

                convertedImage.GetDimensions ( &pRows, &pCols, &pStride );

                ofPixels pix;
                pix.setFromPixels(convertedImage.GetData(), width, height, OF_IMAGE_COLOR);
                pix.setImageType(OF_IMAGE_COLOR);

                ofImage newColorImage;
                newColorImage.setUseTexture(false);
                newColorImage.setFromPixels(pix);

                lock();
                copy(newColorImage, colorImage);
                std::swap(rawImageCurrent, rawImageNext);
                unlock();

                Mat newImage;
                copy(colorImage, newImage);
                ImageMetadata metaData = rawImageCurrent->GetMetadata();
                float embeddedAbsShutter = getAbsShutterFromEmbeddedShutter(metaData.embeddedShutter);

                if((lastEmbeddedAbsShutter-embeddedAbsShutter) > 40 && ldrImages.size() > 5) {

                ofLogNotice() << "Making HDR images" << endl;

/*
                Mat response;
                Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
                calibrate->process(ldrImages, response, times);
                ofLogNotice() << "Calculated Response Curve" << endl;

                Mat hdr;
                Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
                merge_debevec->process(ldrImages, hdr, times, response);
                hdrImage = hdr;
                ofLogNotice() << "Made HDR image" << endl;

                Mat tonemapped;
                Ptr<TonemapDurand> tonemap = createTonemapDurand(2.2f);
                tonemap->process(hdr, tonemapped);
                toOf(tonemapped, tonemappedImage);
                ofLogNotice() << "Made Tonemapped image" << endl;
*/
                Mat fusion;
                Ptr<MergeMertens> merge_mertens = createMergeMertens();
                merge_mertens->process(ldrImages, fusion);
                lock();
                copy(fusion, fusionImage);
                unlock();
                ofLogNotice() << "Made Fusion image with " << ldrImages.size() << " LDR images" << endl;

                //imwrite("fusion.png", fusion * 255);
                //imwrite("ldr.png", ldr * 255);
                //imwrite("hdr.hdr", hdr);


                    ldrImages.clear();
                    times.clear();
                    lock();
                    updateHdrOnMainThread = true;
                    unlock();
                }
                lock();
                ldrImages.push_back(newImage);
                times.push_back(embeddedAbsShutter);
                unlock();
                lastEmbeddedAbsShutter = embeddedAbsShutter;
            }
            else
            {
                catchError(error);
            }

        }

        state = CLOSING;

    }

    ~blackflyThreadedCamera()
    {
        stopThread();
        if(camera)
        {
            camera->StopCapture();
            camera->Disconnect();
            delete camera;
        }
        if(rawImageBuffers)
        {
            delete rawImageBuffers[0];
            delete rawImageBuffers[1];
        }
    }

};

#endif // CAMERACONTROLLER_H_INCLUDED
