#ifndef CAMERACONTROLLER_H_INCLUDED
#define CAMERACONTROLLER_H_INCLUDED

#pragma once

#include "FlyCapture2.h"
#include "ofxXmlSettings.h"
#include "CvUtilities.h"
#include <opencv2/photo.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace ofxCv;
using namespace std;

#include "ofMain.h"

using namespace FlyCapture2;

class HdrImageThread : public ofThread
{

public:

    int width;
    int height;

    int reduction = 1;

    std::queue<FlyCapture2::Image> pImageQueue;
    std::queue<float> pImageQueueExposureTimes;

    std::mutex queueMutex;

    FlyCapture2::Image convertedImage;

    vector<Mat> ldrImages;
    vector<float> ldrExposureTimes;

    ofImage colorImage;
    ofImage fusionImage;
    ofImage tonemappedImage;

    bool updateHdrImage;
    bool updateColorImage;

    int numberExposures;

    void setup(int width, int height, int numberExposures)
    {
        this->width = width;
        this->height = height;
        this->numberExposures = numberExposures;
        updateHdrImage = false;
        updateColorImage = false;

        colorImage.allocate(width, height, OF_IMAGE_COLOR);
        fusionImage.allocate(width/reduction, height/reduction, OF_IMAGE_COLOR);
        tonemappedImage.allocate(width/reduction, height/reduction, OF_IMAGE_COLOR);
    }

    void addImage(FlyCapture2::Image pImage, float exposureTime)
    {

        queueMutex.lock();
        pImageQueue.push(pImage);
        pImageQueueExposureTimes.push(exposureTime);
        queueMutex.unlock();
        if(!isThreadRunning())
        {
            startThread(true);
        }
    }

    void clearQueues()
    {
        lock();
        cout << "cleared ldr vectors" << endl;
        ldrImages.clear();
        ldrExposureTimes.clear();
        unlock();
    }

    void draw()
    {
        ofSetColor(255,255,255,255);
        if(updateHdrImage)
        {
            lock();
            //fusionImage.update();
            tonemappedImage.update();
            updateHdrImage = false;
            unlock();
        }
        if(updateColorImage)
        {
            lock();
            colorImage.update();
            updateColorImage = false;
            unlock();
        }
        colorImage.draw(0,0);
        //fusionImage.draw(0,height);
        tonemappedImage.draw(0,height);
        /*
        lock();
        for(int i = 0; i < ldrExposureTimes.size(); i++)
        {
            float columnWidth = width*1.0/numberExposures;
            ofRect(i*columnWidth, height*2.5, columnWidth, -ldrExposureTimes[i]*3.0);
        }
        unlock();
        */
    }

    void threadedFunction()
    {
        queueMutex.lock();
        int queueSize = pImageQueue.size();
        queueMutex.unlock();
        while(queueSize > 0)
        {
            queueMutex.lock();
            FlyCapture2::Image pImage = pImageQueue.front();
            float pImageExposureTime = pImageQueueExposureTimes.front();
            queueMutex.unlock();

            float ldrImageExposureTime = 0;
            lock();
            if(ldrExposureTimes.size() > 0)
            {
                ldrImageExposureTime = ldrExposureTimes.back();
            }
            unlock();

            if(pImageExposureTime != ldrImageExposureTime)
            {

                pImage.SetColorProcessing(IPP);

                // Convert the raw image
                FlyCapture2::Error error = pImage.Convert( PIXEL_FORMAT_RGB8, &convertedImage );
                if(error == PGRERROR_OK)
                {
                    unsigned int rows, cols, stride;
                    convertedImage.GetDimensions ( &rows, &cols, &stride );

                    ofPixels pix;
                    pix.setFromPixels(convertedImage.GetData(), cols, rows, OF_IMAGE_COLOR);
                    pix.setImageType(OF_IMAGE_COLOR);

                    ofImage newColorImage;
                    newColorImage.setUseTexture(false);
                    newColorImage.setFromPixels(pix);

                    Mat newImage;
                    copy(newColorImage, newImage);

/*                    Mat smallNewImage;
                    resize(newImage, smallNewImage, Size(width/reduction, height/reduction));
*/
                    lock();
                    ldrImages.push_back(newImage);
                    ldrExposureTimes.push_back(pImageExposureTime);
                    unlock();
                }
                else
                {
                    //clearQueues();
                }
            }
            queueMutex.lock();
            pImageQueue.pop();
            pImageQueueExposureTimes.pop();
            queueSize = pImageQueue.size();
            queueMutex.unlock();
            if(ldrImages.size() >= numberExposures)
            {
                /*
                cout << "making fusion" << endl;
                Mat fusion;
                Ptr<MergeMertens> merge_mertens = createMergeMertens();
                merge_mertens->process(ldrImages, fusion);
                */

/*
                cout << "making response" << endl;
                Mat response;
                Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
                calibrate->process(ldrImages, response, ldrExposureTimes);
*/
/*              DOES NOT WORK
                cout << "making color image" << endl;
                Mat combinedLdrs(width*(ldrImages.size()+1), height*(ldrImages.size()+1), CV_8UC3);
                for(int i = 0; i < ldrImages.size(); i++){
                    Mat roi(combinedLdrs, Rect(i*width, 0, width, height));
                    ldrImages[i].copyTo(roi);
                }
                Mat newColorImage;
                resize(combinedLdrs, newColorImage, Size(width, height));
                */
                Mat newColorImage(ldrImages.back());

                cout << "making hdr" << endl;
                Mat hdr;
                Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
                merge_debevec->process(ldrImages, hdr, ldrExposureTimes);

                cout << "making tonemapped" << endl;
                Mat tonemapped;
                Ptr<TonemapReinhard> tonemap = createTonemapReinhard(1.2);
//                Ptr<TonemapMantiuk> tonemap = createTonemapMantiuk(1.2);
                tonemap->process(hdr, tonemapped);

                lock();
                //copy(fusion, fusionImage);
                copy(newColorImage, colorImage);
                copy(tonemapped, tonemappedImage);
                updateColorImage = true;
                updateHdrImage = true;
                unlock();

                clearQueues();

            }
        }
        cout << "stopping thread" << endl;
    }
};


class blackflyThreadedCamera : public ofThread
{
public:
    ofTrueTypeFont font;

    static const int width = 1280;
    static const int height = 720;

    int numberExposures = 3;
    int numberExposuresGUI = 3;

    int divisor = 8;
    int divisorGUI = 8;

    float shutterDenominator = 1500.0;
    float shutterDenominatorGUI = 1500.0;

    const PixelFormat k_PixFmt = PIXEL_FORMAT_RAW8;
    const float k_frameRate = 10.0;

    GigECamera* camera;
    CameraInfo cameraInfo;

    ofxXmlSettings settings;

    FlyCapture2::Image * rawImageBuffers[2];
    FlyCapture2::Image * rawImageCurrent;
    FlyCapture2::Image * rawImageNext;

    HdrImageThread hdrImageThread;

    list<float> history;

    bool drawWasCalled;
    bool errorRetrieve;

    float absShutterVals[4096];
    unsigned int absShutterValsRead;

    enum CameraState { NEW, SETUP, READING_EXP, RUNNING, CLOSING };

    CameraState state = NEW;

    blackflyThreadedCamera()
    {
        state = NEW;
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

    bool PollForTriggerReady()
    {
        const unsigned int k_softwareTrigger = 0x62C;
        FlyCapture2::Error error;
        unsigned int regVal = 0;

        do
        {
            error = camera->ReadRegister( k_softwareTrigger, &regVal );
            if (error != PGRERROR_OK)
            {
                error.PrintErrorTrace();
                return false;
            }
        }
        while ( (regVal >> 31) != 0 );

        return true;
    }

    bool FireSoftwareTrigger()
    {
        const unsigned int k_softwareTrigger = 0x62C;
        const unsigned int k_fireVal = 0x80000000;
        FlyCapture2::Error error;

        error = camera->WriteRegister( k_softwareTrigger, k_fireVal );
        if (error != PGRERROR_OK)
        {
            error.PrintErrorTrace();
            return false;
        }

        return true;
    }


    int setup(unsigned int serialNumber)
    {

        font.loadFont("GUI/DroidSans.ttf", 42, true, true, true);

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
                    packetDelayProp.value = static_cast<unsigned int>(1200);
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
                    framerate.absValue = k_frameRate; // 2076 is 12 fps
                    framerate.autoManualMode = false;
                    catchError(camera->SetProperty(&framerate));

                    /** STARTING CAPTURE HERE **/

                    // Get the camera configuration
                    FC2Config config;
                    catchError(camera->GetConfiguration( &config ));
                    // Set the grab timeout to 1.5 seconds
                    config.grabTimeout = 1500;

                    // Set the camera configuration
                    catchError(camera->SetConfiguration( &config ));

                    catchError(camera->StartCapture());

                    printf( "Setting brightness...\n" );

                    Property brightness(BRIGHTNESS);
                    brightness.autoManualMode = false;
                    brightness.absControl = true;
                    brightness.absValue = 0.0;
                    catchError(camera->SetProperty(&brightness));

                    printf( "Setting exposure...\n" );

                    Property autoExposure(AUTO_EXPOSURE);
                    autoExposure.onOff = false;
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
                    gain.absValue = 1.0;
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

        hdrImageThread.setup(width, height, numberExposures);

        startThread(true);

        return 0;

    }

    void draw()
    {
        ofPushStyle();
        ofDisableDepthTest();
        ofSetColor(127,255);
        ofRect(0,0,width, height);

        string str;
        float alpha = (.5+(sinf(ofGetElapsedTimef()*2.0)*.5));

        str = "s/n: " + ofToString(cameraInfo.serialNumber) + "\n";

        switch (state)
        {
        case NEW:
            break;
        case SETUP:
            ofSetColor(255.0, 255.0);
            str += "Setting Up Camera...";
            font.drawString(str,40,80);
            break;
        case READING_EXP:
            /*            ofSetColor(255, 255);
                        hdrImageThread.colorImage.update();
                        hdrImageThread.draw();
            */
            ofSetColor(255,255,255,63.0);
            ofRect(0,0,width*(absShutterValsRead/4096.0), height);
            ofSetColor(0,0,0,63.0);
            ofRect(width*(absShutterValsRead/4096.0),0,width-(width*(absShutterValsRead/4096.0)), height);
            ofSetColor(255.0, 255.0);
            str += "Building Exposure Table ...";
            if(absShutterValsRead > 0)
                str += "\n" +  ofToString(absShutterValsRead) + ": " + ofToString(absShutterVals[absShutterValsRead-1]);
            font.drawString(str,40,80);
            break;
        case RUNNING:
            ofSetColor(255, 255);
            hdrImageThread.draw();
            if(rawImageCurrent)
            {
                ImageMetadata metaData = rawImageCurrent->GetMetadata();
                str += "shutter: " + ofToString(getAbsShutterFromEmbeddedShutter(metaData.embeddedShutter));
            }
            font.drawString(str,40,80);
            ofSetColor(255,0,0);
            if(errorRetrieve) ofRect(width-30,0, 30, 30);
            {
                lock();
                int historyWidth = 5;
                int historyI = 0;
                while(history.size()*historyWidth > width) history.pop_front();
                for(std::list<float>::iterator it = history.begin(); it != history.end(); it++)
                {
                    float historyItem = *(it);
                    if(historyItem < 0)
                    {
                        ofSetColor(255,0,0,255);
                    }
                    else
                    {
                        ofSetColor(0,255,historyItem*8);
                    }
                    ofRect(historyI*historyWidth, 0, historyWidth, historyWidth);
                    historyI++;
                }
                unlock();
            }
            break;
        case CLOSING:
            ofSetColor(255,0,0,(.5+(sinf(ofGetElapsedTimef()*2.0)*.5))*255.0);
            ofRect(0,0,width, height);
            break;
        default:
            break;
        }
        ofPopStyle();
        drawWasCalled = true;
    }

    void threadedFunction()
    {

        string fileName = ofToDataPath("Camera-" + ofToString(cameraInfo.serialNumber) + "-absShutterVals.csv");

        state = READING_EXP;

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
            }
            file.close();
        }

        if(absShutterValsRead < 4096)
        {
            if(ofFile::doesFileExist(fileName))
            {
                ofFile::removeFile(fileName);
            }
            lock();
            absShutterValsRead = 0;
            unlock();
            printf( "Building absolute exposure table ...\n" );

            printf( "Setting trigger...\n" );

            TriggerMode triggerMode;
            catchError(camera->GetTriggerMode( &triggerMode ));
            triggerMode.onOff = true;
            // Set camera to trigger mode 0
            triggerMode.mode = 0;
            triggerMode.parameter = 0;
            // Trigger source 7 = software, 0 = external
            triggerMode.source = 7;

            catchError(camera->SetTriggerMode( &triggerMode ));


            Property prop;
            prop.type = SHUTTER;

            prop.autoManualMode = false;
            prop.absControl = true;

            // Read register for relative shutter

            const unsigned int shutter_register = 0x81C;
            unsigned int shutterRelative = 0;

            camera->ReadRegister(shutter_register, &shutterRelative);

            ofFile file;
            file.open(fileName, ofFile::WriteOnly, true);

            FlyCapture2::Error error;

            // Loop through all possible relative shutter values. Save relative and corresponding absolute shutter

            for(unsigned int shutter_count = 1; shutter_count <= 4096; shutter_count ++)
            {

                camera->WriteRegister(shutter_register,shutter_count);

                PollForTriggerReady();

                bool retVal = FireSoftwareTrigger();
                if ( !retVal )
                {
                    printf("\nError firing software trigger!\n");
                }

                error = camera->GetProperty( &prop );
                if (error != PGRERROR_OK)
                {
                    catchError( error );
                }

                camera->ReadRegister( shutter_register, &shutterRelative );

                file << ofToString(prop.absValue, 16) << endl;

                lock();
                absShutterVals[(shutterRelative & 0x0000FFFF)-1] = prop.absValue;
                absShutterValsRead = shutter_count;
                unlock();

            }
            file.close();
        }

        printf( "Setting trigger...\n" );

        TriggerMode triggerMode;
        catchError(camera->GetTriggerMode( &triggerMode ));
        triggerMode.onOff = false;
        // Set camera to trigger mode 0
        triggerMode.mode = 0;
        triggerMode.parameter = 0;
        // Trigger source 7 = software, 0 = external
        triggerMode.source = 7;

        catchError(camera->SetTriggerMode( &triggerMode ));



        while (isThreadRunning())
        {
            state = RUNNING;
            shutterDenominator = shutterDenominatorGUI;
            float lastEmbeddedShutter = 0;

            for (int i = 0; i < numberExposures; )
            {

                float shutterAbs = 1000.0/shutterDenominator;
                Property shutter(SHUTTER);
                shutter.absValue = shutterAbs;
                shutter.autoManualMode = false;
                shutter.absControl = true;
                shutter.onOff = true;
                catchError(camera->SetProperty(&shutter));

                if(i == 0) sleep(floor(shutterDenominator/20.0));

                FlyCapture2::Error error = camera->RetrieveBuffer(rawImageNext);
                if(error == PGRERROR_OK)
                {
                    ImageMetadata metaData = rawImageNext->GetMetadata();
                    float embeddedAbsShutter = getAbsShutterFromEmbeddedShutter(metaData.embeddedShutter);
                    history.push_back(embeddedAbsShutter);
                    if(embeddedAbsShutter != lastEmbeddedShutter)
                    {
                        //FlyCapture2::Image * newImage;
                        //rawImageNext->DeepCopy(newImage);
                        hdrImageThread.addImage(*(rawImageNext), embeddedAbsShutter);
                        shutterDenominator /= divisor;
                        i++;
                        lock();
                        std::swap(rawImageCurrent, rawImageNext);
                        unlock();
                        errorRetrieve = false;
                    }
                    lastEmbeddedShutter = embeddedAbsShutter;
                }
                else
                {
                    history.push_back(-1.0);
                    catchError(error);
                    errorRetrieve = true;
                }
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
