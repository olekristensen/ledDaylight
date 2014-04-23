#ifndef CAMERACONTROLLER_H_INCLUDED
#define CAMERACONTROLLER_H_INCLUDED

#pragma once

#include "FlyCapture2.h"
using namespace FlyCapture2;

#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"

class CameraController
{
public:
    static const int camWidth = 1280;
    static const int camHeight = 720;

    static const float minBlobArea = .001;
    static const float maxBlobArea = .2;
    static const int maxBlobs = 8;
    static const int thresholdLevel = 128;

    const PixelFormat k_PixFmt = PIXEL_FORMAT_RAW8;

    const unsigned int k_HDRCtrl = 0x1800;

    const unsigned int k_HDRShutter1 = 0x1820;
    const unsigned int k_HDRShutter2 = 0x1840;
    const unsigned int k_HDRShutter3 = 0x1860;
    const unsigned int k_HDRShutter4 = 0x1880;

    const unsigned int k_HDRGain1 = 0x1824;
    const unsigned int k_HDRGain2 = 0x1844;
    const unsigned int k_HDRGain3 = 0x1864;
    const unsigned int k_HDRGain4 = 0x1884;

    const unsigned int k_HDROn = 0x82000000;
    const unsigned int k_HDROff = 0x80000000;

    float shutterWasSetToAbs = 0.0;

    vector<GigECamera*> cameras;
    vector<Image*> rawImages;
    vector<ImageMetadata> imagesMetadata;
    vector<ofxCvColorImage*> images;

    float getAbsShutterValueFromRegister(unsigned int value)
    {
        typedef union _AbsValueConversion
        {
            unsigned int ulValue;
            float fValue;
        } AbsValueConversion;

        AbsValueConversion converted;
        converted.ulValue = (value & 0xFFFFu);

        std::cout << converted.fValue << std::endl;

        return converted.fValue;

        value = (value & 0xFFFFu);
        float absValue = 0.0;
        std::cout << value << std::endl;
        float unitMillis = 0.01;
        if(value < 1025) {
            absValue = unitMillis * value;
            std::cout << absValue << std::endl;
            return absValue;
        }
        unsigned int exponent = (((value - 1024)/512)+1);
        absValue = 10.24;
        for(int i = 1; i < exponent; i++)
        {
            unitMillis = 0.01*pow(2,i);
            absValue += unitMillis * 512;
        }
        unitMillis = 0.01*pow(2,exponent);
        absValue += (value - (1024+((exponent-1)*512)))*unitMillis;
        return absValue;
    }

    CameraController()
    {
    }

    ~CameraController()
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


    bool IsHDRSupported( Camera* pCam )
    {
        Error error;
        unsigned int RegVal;
        error = pCam->ReadRegister( k_HDRCtrl, &RegVal );
        if (error != PGRERROR_OK)
        {
            catchError( error );
            return false;
        }

        return ( !0x80000000 & RegVal ) ? false : true;
    }

    void ToggleHDRMode( Camera* pCam, bool hdrOn )
    {
        Error error;
        error = pCam->WriteRegister( k_HDRCtrl, hdrOn ? k_HDROn : k_HDROff );
        if (error != PGRERROR_OK)
        {
            catchError( error );
        }

        printf( "HDR mode turned to %s\n", hdrOn ? "on" : "off" );
    }

    void InitializeHDRRegisters( Camera* pCam )
    {
        Error error;
        error = pCam->WriteRegister( k_HDRShutter1, 0x000 );
        error = pCam->WriteRegister( k_HDRShutter2, 0x120 );
        error = pCam->WriteRegister( k_HDRShutter3, 0x240 );
        error = pCam->WriteRegister( k_HDRShutter4, 0x360 );

        error = pCam->WriteRegister( k_HDRGain1, 0x000 );
        error = pCam->WriteRegister( k_HDRGain2, 0x0E3 );
        error = pCam->WriteRegister( k_HDRGain3, 0x1C6 );
        error = pCam->WriteRegister( k_HDRGain4, 0x2A9 );

        printf( "Initialized HDR registers...\n" );
    }


    void catchError(Error error)
    {
        if(error != PGRERROR_OK)
            error.PrintErrorTrace();
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

    bool PollForTriggerReady( GigECamera* pCam )
    {
        const unsigned int k_softwareTrigger = 0x62C;
        Error error;
        unsigned int regVal = 0;

        do
        {
            error = pCam->ReadRegister( k_softwareTrigger, &regVal );
            if (error != PGRERROR_OK)
            {
                catchError( error );
                return false;
            }

        }
        while ( (regVal >> 31) != 0 );

        return true;
    }

    bool FireSoftwareTrigger( GigECamera* pCam )
    {
        const unsigned int k_softwareTrigger = 0x62C;
        const unsigned int k_fireVal = 0x80000000;
        Error error;

        error = pCam->WriteRegister( k_softwareTrigger, k_fireVal );
        if (error != PGRERROR_OK)
        {
            catchError( error );
            return false;
        }

        return true;
    }


    void load(string filename)
    {
        ofxXmlSettings cameraSettings;
        cameraSettings.loadFile(filename);

        BusManager busMgr;
        catchError(busMgr.ForceAllIPAddressesAutomatically());
        unsigned int numCameras;
        catchError(busMgr.GetNumOfCameras(&numCameras));
        printf("found %u cameras", numCameras);
        if(numCameras < 1) exit(0);
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
                        for (unsigned int i=0; i < numStreamChannels; i++)
                        {
                            GigEStreamChannel streamChannel;
                            catchError(camera->GetGigEStreamChannelInfo( i, &streamChannel ));
                            printf( "\nPrinting stream channel information for channel %u:\n", i );
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

                        Property framerate(FRAME_RATE);
                        framerate.onOff = true;
                        framerate.absControl = true;
                        framerate.absValue = cameraSettings.getValue("framerate", 12.0); // 2076 is 12 fps
                        framerate.autoManualMode = false;
                        catchError(camera->SetProperty(&framerate));

                        /** STARTING HERE **/

                        // Get current trigger settings

                        TriggerMode triggerMode;
                        catchError(camera->GetTriggerMode( &triggerMode ));
                        // Set camera to trigger mode 0
                        triggerMode.onOff = false;
                        triggerMode.mode = 0;
                        triggerMode.parameter = 0;
                        triggerMode.source = 7;

                        catchError(camera->SetTriggerMode( &triggerMode ));

                        catchError(camera->StartCapture());

                        Property brightness(BRIGHTNESS);
                        brightness.autoManualMode = false;
                        brightness.absControl = true;
                        brightness.absValue = cameraSettings.getValue("brightness", 0.0);
                        catchError(camera->SetProperty(&brightness));

                        Property autoExposure(AUTO_EXPOSURE);
                        autoExposure.onOff = true;
                        autoExposure.autoManualMode = false;
                        autoExposure.absControl = true;
                        autoExposure.absValue = cameraSettings.getValue("exposure", 1.0);
                        catchError(camera->SetProperty(&autoExposure));

                        Property gamma(GAMMA);
                        gamma.onOff = true;
                        gamma.autoManualMode = false;
                        gamma.absControl = true;
                        gamma.absValue = 1.0;
                        catchError(camera->SetProperty(&gamma));

                        Property shutter(SHUTTER);
                        shutter.absControl = true;
                        shutter.absValue = cameraSettings.getValue("shutter", 1.0); // ms
                        shutter.autoManualMode = false;
                        shutter.onOff = true;
                        catchError(camera->SetProperty(&shutter));

                        Property gain(GAIN);
                        gain.absValue = cameraSettings.getValue("gain", 3.0);
                        gain.autoManualMode = false;
                        gain.absControl = true;
                        gain.onOff = true;
                        catchError(camera->SetProperty(&gain));

                        Property whitebalance(WHITE_BALANCE);
                        whitebalance.valueA = cameraSettings.getValue("whitebalanceRed", 512);
                        whitebalance.valueB = cameraSettings.getValue("whitebalanceBlue", 512);
                        whitebalance.autoManualMode = true;
                        whitebalance.onOff = true;
                        catchError(camera->SetProperty(&whitebalance));

                        cameras.push_back(camera);
                        rawImages.push_back(rawImage);

                        ofxCvColorImage* image = new ofxCvColorImage();
                        image->allocate(camWidth, camHeight);
                        images.push_back(image);


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
        }
    }

    void update()
    {
        imagesMetadata.clear();

        for(unsigned int i = 0; i < cameras.size(); i++)
        {
            GigECamera& camera = *(cameras[i]);

            shutterWasSetToAbs = pow(1000./(fmodf(ofGetElapsedTimef()*20, 2000.0)+5),2);
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
    }

    void draw()
    {
        ofPushMatrix();
        float scaleFactor = ofGetWidth()/(camWidth*2.0);
        float scaleFactorY = ofGetHeight()/(camHeight*1.0);

        ofScale(scaleFactor, scaleFactor, scaleFactor);
        for(unsigned int i = 0; i < cameras.size(); i++)
        {
            images[i]->draw(0, 0);

            ImageMetadata metadata = imagesMetadata[i];

            Property shutter(SHUTTER);
            catchError(cameras[i]->GetProperty(&shutter));

            ofDrawBitmapString("Image Metadata", 10, camHeight+200);
            ofDrawBitmapString(ofToString(metadata.embeddedShutter), 10, camHeight +40);
            ofDrawBitmapString(ofToString(getAbsShutterValueFromRegister(metadata.embeddedShutter)), 10, camHeight +80);
            ofDrawBitmapString(ofToString(shutterWasSetToAbs), 10, camHeight +120);
            ofDrawBitmapString(ofToString(metadata.embeddedShutter), 10, camHeight +160);

            ofDrawBitmapString("Camera Metadata", 300, camHeight+200);
            ofDrawBitmapString(ofToString(shutter.valueA), 300, camHeight +40);
            ofDrawBitmapString(ofToString(getAbsShutterValueFromRegister(shutter.valueA)), 300, camHeight +80);
            ofDrawBitmapString(ofToString(shutter.absValue), 300, camHeight +120);
            ofDrawBitmapString(ofToString(shutter.valueA), 300, camHeight +160);

            ofTranslate(camWidth, 0);
        }
        ofPopMatrix();
    }
};

#endif // CAMERACONTROLLER_H_INCLUDED
