//
//  OpenNIDevice.cpp
//  KinectExtension
//
//  Created by Wouter Verweirder on 24/01/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <math.h>
#include "OpenNIDevice.h"

XnBool needPose = FALSE;
XnChar strPose[20] = "";

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE userExitCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE userReentryCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE poseDetectedCallback(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE outOfPoseCallback(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& rCapability, XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE calibrationCompleteCallback(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus calibrationError, void* pCookie);

OpenNIDevice::OpenNIDevice(int nr, xn::Context context)
{
    printf("OpenNIDevice::OpenNIDevice(%i)\n", nr);
    this->nr = nr;
    this->freContext = freContext;
    this->context = context;
    
    //initialize the mutexes
    /*
    pthread_mutex_init(&userMutex, NULL);
    pthread_mutex_init(&depthMutex, NULL);
    pthread_mutex_init(&rgbMutex, NULL);
    pthread_mutex_init(&userMaskMutex, NULL);
    pthread_mutex_init(&infraredMutex, NULL);
    pthread_mutex_init(&pointCloudMutex, NULL);
     */
    
    //set default values
    setDefaults();
}

void OpenNIDevice::setFreContext(FREContext pFreContext)
{
	freContext = pFreContext;
}

FREContext OpenNIDevice::getFreContext()
{
	return freContext;
}

void OpenNIDevice::setDefaults()
{
    //set some default values
    running = false;
    started = false;
    userCallbacksRegistered = false;
    
    asUserMirrored = false;
    asUserEnabled = false;
    asSkeletonMirrored = false;
    asSkeletonEnabled = false;
    
    asDepthWidth = 320;
    asDepthHeight = 240;
    asDepthPixelCount = asDepthWidth * asDepthHeight;
    asDepthMirrored = false;
    asDepthEnabled = false;
    asDepthShowUserColors = false;
    
    depthWidth = 640;
    depthHeight = 480;
    depthPixelCount = depthWidth * depthHeight;
    depthScale = depthWidth / asDepthWidth;
    
    depthGenerator = NULL;
    
    depthByteArray = 0;
    
    asRGBWidth = 320;
    asRGBHeight = 240;
    asRGBPixelCount = asRGBWidth * asRGBHeight;
    asRGBMirrored = false;
    asRGBEnabled = false;
    
    rgbWidth = 640;
    rgbHeight = 480;
    rgbPixelCount = rgbWidth * rgbHeight;
    rgbScale = rgbWidth / asRGBWidth;
    
    imageGenerator = NULL;
    
    RGBByteArray = 0;
    
    userGenerator = NULL;
    
    asUserMaskWidth = 320;
    asUserMaskHeight = 240;
    asUserMaskPixelCount = asUserMaskWidth * asUserMaskHeight;
    asUserMaskMirrored = false;
    asUserMaskEnabled = false;
    
    userMaskWidth = 640;
    userMaskHeight = 480;
    userMaskPixelCount = userMaskWidth * userMaskHeight;
    userMaskScale = userMaskWidth / asUserMaskWidth;
    
    userMaskByteArray = 0;
    
    asInfraredWidth = 320;
    asInfraredHeight = 240;
    asInfraredPixelCount = asInfraredWidth * asInfraredHeight;
    asInfraredMirrored = false;
    asInfraredEnabled = false;
    
    infraredWidth = 640;
    infraredHeight = 480;
    infraredPixelCount = infraredWidth * infraredHeight;
    infraredScale = infraredWidth / asInfraredWidth;
    
    infraredGenerator = NULL;
    
    infraredByteArray = 0;
    
    asPointCloudWidth = 320;
    asPointCloudHeight = 240;
    asPointCloudPixelCount = asPointCloudWidth * asPointCloudHeight;
    asPointCloudMirrored = false;
    asPointCloudEnabled = false;
    asPointCloudDensity = 1;
    asPointCloudIncludeRGB = false;
    
    pointCloudWidth = 640;
    pointCloudHeight = 480;
    pointCloudPixelCount = pointCloudWidth * pointCloudHeight;
    pointCloudScale = pointCloudWidth / asPointCloudWidth;
    
    pointCloudByteArray = 0;
    pointCloudRegions = 0;
    
    //player index coloring
    setUserColor(1, 0xff0000, 1);
	setUserColor(2, 0x00ff00, 1);
	setUserColor(3, 0x0000ff, 1);
	setUserColor(4, 0xffff00, 1);
	setUserColor(5, 0xff00ff, 1);
	setUserColor(6, 0x8080ff, 1);
    
    setUserColor(7, 0xff0000, 1);
	setUserColor(8, 0x00ff00, 1);
	setUserColor(9, 0x0000ff, 1);
	setUserColor(10, 0xffff00, 1);
	setUserColor(11, 0xff00ff, 1);
	setUserColor(12, 0x8080ff, 1);
    
    setUserColor(13, 0xff0000, 1);
	setUserColor(14, 0x00ff00, 1);
	setUserColor(15, 0x0000ff, 1);
}

kinectUserFrame OpenNIDevice::getUserFrameBuffer(){
	return userFrame;
}

int OpenNIDevice::getAsDepthWidth()
{
    return asDepthWidth;
}

int OpenNIDevice::getAsDepthHeight()
{
    return asDepthHeight;
}

uint32_t* OpenNIDevice::getAsDepthByteArray(){
	return depthByteArray;
}

int OpenNIDevice::getAsRGBWidth()
{
    return asRGBWidth;
}

uint32_t* OpenNIDevice::getAsRGBByteArray(){
	return RGBByteArray;
}

int OpenNIDevice::getAsRGBHeight()
{
    return asRGBHeight;
}

int OpenNIDevice::getAsInfraredWidth()
{
    return asInfraredWidth;
}

int OpenNIDevice::getAsInfraredHeight()
{
    return asInfraredHeight;
}

uint32_t* OpenNIDevice::getAsInfraredByteArray(){
	return infraredByteArray;
}

int OpenNIDevice::getAsUserMaskWidth()
{
    return asUserMaskWidth;
}

int OpenNIDevice::getAsUserMaskHeight()
{
    return asUserMaskHeight;
}

uint32_t* OpenNIDevice::getAsUserMaskByteArray(int userID){
	return userMaskByteArray[userID];
}

int OpenNIDevice::getAsPointCloudWidth()
{
    return asPointCloudWidth;
}

int OpenNIDevice::getAsPointCloudHeight()
{
    return asPointCloudHeight;
}

bool OpenNIDevice::getASPointCloudMirror(){
	return asPointCloudMirrored;
}

int OpenNIDevice::getASPointCloudDensity(){
	return asPointCloudDensity;
}

bool OpenNIDevice::getASPointCloudIncludeRGB(){
	return asPointCloudIncludeRGB;
}

int OpenNIDevice::getAsPointCloudByteArrayLength()
{
    if(asPointCloudIncludeRGB)
    {
        return (asPointCloudWidth * asPointCloudHeight * sizeof(short) * 6) / asPointCloudDensity;
    }
    else
    {
        return (asPointCloudWidth * asPointCloudHeight * sizeof(short) * 3) / asPointCloudDensity;
    }
}

short* OpenNIDevice::getAsPointCloudByteArray(){
	return pointCloudByteArray;
}

PointCloudRegion* OpenNIDevice::getPointCloudRegions(){
	return pointCloudRegions;
}

unsigned int OpenNIDevice::getNumRegions(){
	return numRegions;
}

void OpenNIDevice::lockUserMutex()
{
    userMutex.lock();
    //pthread_mutex_lock(&userMutex);
}

void OpenNIDevice::unlockUserMutex()
{
    userMutex.unlock();
    //pthread_mutex_unlock(&userMutex);
}

void OpenNIDevice::lockDepthMutex()
{
    depthMutex.lock();
    //pthread_mutex_lock(&depthMutex);
}

void OpenNIDevice::unlockDepthMutex()
{
    depthMutex.unlock();
    //pthread_mutex_unlock(&depthMutex);
}

void OpenNIDevice::lockRGBMutex()
{
    rgbMutex.lock();
    //pthread_mutex_lock(&rgbMutex);
}

void OpenNIDevice::unlockRGBMutex()
{
    rgbMutex.unlock();
    //pthread_mutex_unlock(&rgbMutex);
}

void OpenNIDevice::lockUserMaskMutex()
{
    userMaskMutex.lock();
    //pthread_mutex_lock(&userMaskMutex);
}

void OpenNIDevice::unlockUserMaskMutex()
{
    userMaskMutex.unlock();
    //pthread_mutex_unlock(&userMaskMutex);
}

void OpenNIDevice::lockInfraredMutex()
{
    infraredMutex.lock();
    //pthread_mutex_lock(&infraredMutex);
}

void OpenNIDevice::unlockInfraredMutex()
{
    infraredMutex.unlock();
    //pthread_mutex_unlock(&infraredMutex);
}

void OpenNIDevice::lockPointCloudMutex()
{
    pointCloudMutex.lock();
    //pthread_mutex_lock(&pointCloudMutex);
}

void OpenNIDevice::unlockPointCloudMutex()
{
    pointCloudMutex.unlock();
    //pthread_mutex_unlock(&pointCloudMutex);
}

void OpenNIDevice::start()
{
    printf("OpenNIDevice::start()\n");
    if(!running)
    {
        //returnVal = pthread_attr_init(&attr);
        //returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        
        running = true;
        
        //int threadError = pthread_create(&posixThreadID, &attr, deviceThread, (void *) this);
        //printf("thread create code: %i\n", threadError);
        mThread = boost::thread(&OpenNIDevice::deviceThread, this);
    }
}

void OpenNIDevice::stop()
{
    printf("OpenNIDevice::stop()\n");
    if(running)
    {
        running = false;
        //int threadError = pthread_join(posixThreadID, NULL);
        //printf("thread join code: %d\n", threadError);
        mThread.join();
    }
    if(depthGenerator.IsValid())
    {
        if(depthGenerator.IsGenerating())
        {
            depthGenerator.StopGenerating();
        }
        depthGenerator.Release();
        depthGenerator = NULL;
    }
    if(imageGenerator.IsValid())
    {
        if(imageGenerator.IsGenerating())
        {
            imageGenerator.StopGenerating();
        }
        imageGenerator.Release();
        imageGenerator = NULL;
    }
    if(userGenerator.IsValid())
    {
        if(userGenerator.IsGenerating())
        {
            //stop generating
            userGenerator.StopGenerating();
        }
        if(userCallbacksRegistered)
        {
            //unregister skeleton detection callbacks
            userGenerator.UnregisterUserCallbacks(userHandle);
            userGenerator.UnregisterFromUserExit(userExitHandle);
            userGenerator.UnregisterFromUserReEnter(userReentryHandle);
            userGenerator.GetSkeletonCap().UnregisterFromCalibrationStart(calibrationStartHandle);
            userGenerator.GetSkeletonCap().UnregisterFromCalibrationComplete(calibrationCompleteHandle);
            userGenerator.GetPoseDetectionCap().UnregisterFromPoseDetected(poseDetectedHandle);
            userGenerator.GetPoseDetectionCap().UnregisterFromOutOfPose(outOfPoseHandle);
        }
        userGenerator.Release();
        userGenerator = NULL;
    }
    if(infraredGenerator.IsValid())
    {
        if(infraredGenerator.IsGenerating())
        {
            infraredGenerator.StopGenerating();
        }
        infraredGenerator.Release();
        infraredGenerator = NULL;
    }
    //cleanup bytearrays
    if(depthByteArray != 0) delete [] depthByteArray;
    if(RGBByteArray != 0) delete [] RGBByteArray;
    if(infraredByteArray != 0) delete [] infraredByteArray;
    if(pointCloudByteArray != 0) delete [] pointCloudByteArray;
    if(userMaskByteArray != 0)
    {
        for(int i = 0; i < MAX_SKELETONS; i++)
        {
            delete [] userMaskByteArray[i];
        }
        delete [] userMaskByteArray;
    }
    if(pointCloudRegions != 0)
    {
        delete [] pointCloudRegions;
    }
    //reset defaults
    setDefaults();
    if(started)
    {
        started = false;
        //send stopped event
        FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "stopped");
    }
}

void * OpenNIDevice::deviceThread(void *self)
{
    OpenNIDevice *adapter = (OpenNIDevice *) self;
    adapter->run();
    return NULL;
}

void OpenNIDevice::run()
{
    printf("OpenNIDevice::run(), %s\n", (running) ? "true" : "false");
    if(running)
    {  
        XnStatus rc;
        
        XnMapOutputMode depthMode;
        depthMode.nXRes = depthWidth;
        depthMode.nYRes = depthHeight;
        depthMode.nFPS = 30;
        
        rc = depthGenerator.Create(context);
        if(rc != XN_STATUS_OK)
        {
            printf("OpenNIDevice create depthGenerator failed: %s\n", xnGetStatusString(rc));
            stop();
            return;
        }
        depthGenerator.SetMapOutputMode(depthMode);
        depthGenerator.GetMirrorCap().SetMirror(false);
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }  
        
        XnMapOutputMode rgbMode;
        rgbMode.nXRes = rgbWidth;
        rgbMode.nYRes = rgbHeight;
        rgbMode.nFPS = 30;
        
        rc = imageGenerator.Create(context);
        if(rc != XN_STATUS_OK)
        {
            printf("OpenNIDevice create imageGenerator failed: %s\n", xnGetStatusString(rc));
            stop();
            return;
        }
        imageGenerator.SetMapOutputMode(rgbMode);
        imageGenerator.GetMirrorCap().SetMirror(false);
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }
        
        XnMapOutputMode infraredMode;
        infraredMode.nXRes = infraredWidth;
        infraredMode.nYRes = infraredHeight;
        infraredMode.nFPS = 30;
        
        rc = infraredGenerator.Create(context);
        if(rc != XN_STATUS_OK)
        {
            printf("OpenNIDevice create infraredGenerator failed: %s\n", xnGetStatusString(rc));
            stop();
            return;
        }
        infraredGenerator.SetMapOutputMode(infraredMode);
        infraredGenerator.GetMirrorCap().SetMirror(false);
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }
        
        //map depth to rgb stream
        depthGenerator.GetAlternativeViewPointCap().SetViewPoint(imageGenerator);
        
        //initialize the user generator
        rc = userGenerator.Create(context);
        if(rc != XN_STATUS_OK)
        {
            printf("OpenNIDevice create userGenerator failed: %s\n", xnGetStatusString(rc));
            stop();
            return;
        }
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }
        
        //register skeleton detection callbacks
        userGenerator.RegisterUserCallbacks(newUserCallback, lostUserCallback, (xn::UserGenerator *)&userGenerator, userHandle);
        userGenerator.RegisterToUserExit(userExitCallback, (xn::UserGenerator *)&userGenerator, userExitHandle);
        userGenerator.RegisterToUserReEnter(userReentryCallback, (xn::UserGenerator *)&userGenerator, userReentryHandle);
        userGenerator.GetSkeletonCap().RegisterToCalibrationStart(calibrationStartCallback, (xn::UserGenerator *)&userGenerator, calibrationStartHandle);
        userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(calibrationCompleteCallback, (xn::UserGenerator *)&userGenerator, calibrationCompleteHandle);
        userGenerator.GetPoseDetectionCap().RegisterToPoseDetected(poseDetectedCallback, (xn::UserGenerator *)&userGenerator, poseDetectedHandle);
        userGenerator.GetPoseDetectionCap().RegisterToOutOfPose(outOfPoseCallback, (xn::UserGenerator *)&userGenerator, outOfPoseHandle);
        
        userCallbacksRegistered = true;
        
        if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
        {
            printf("Supplied user generator doesn't support skeleton\n");
        }
        
        if (userGenerator.GetSkeletonCap().NeedPoseForCalibration())
        {
            needPose = true;
            if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
            {
                printf("Pose required, but not supported\n");
            }
            else
            {
                userGenerator.GetSkeletonCap().GetCalibrationPose(strPose);
            }
        }
        else
        {
            needPose = false;
        }
        
        userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
        userGenerator.GetSkeletonCap().SetSmoothing(0.5f);
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }
        
        if(asDepthEnabled || asPointCloudEnabled)
        {
            rc = depthGenerator.StartGenerating();
            if(rc != XN_STATUS_OK)
            {
                printf("OpenNIDevice start depthGenerator failed: %s\n", xnGetStatusString(rc));
                stop();
                return;
            }
        }
        if(asRGBEnabled || asUserMaskEnabled || asPointCloudEnabled)
        {
            rc = imageGenerator.StartGenerating();
            if(rc != XN_STATUS_OK)
            {
                printf("OpenNIDevice start imageGenerator failed: %s\n", xnGetStatusString(rc));
                stop();
                return;
            }
        }
        if((asDepthEnabled && asDepthShowUserColors) || asUserMaskEnabled || asUserEnabled || asSkeletonEnabled)
        {
            rc = userGenerator.StartGenerating();
            if(rc != XN_STATUS_OK)
            {
                printf("OpenNIDevice start userGenerator failed: %s\n", xnGetStatusString(rc));
                stop();
                return;
            }
        }
        //you cant have both infrared and rgb
        if(asInfraredEnabled && !imageGenerator.IsGenerating())
        {
            rc = infraredGenerator.StartGenerating();
            if(rc != XN_STATUS_OK)
            {
                printf("OpenNIDevice start infraredGenerator failed: %s\n", xnGetStatusString(rc));
                stop();
                return;
            }
        }
        
        //check if running is still true, as OpenNi takes a while to initialize
        if(!running)
        {
            stop();
            return;
        }
        
        //send started event
        started = true;
        FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "started");
        while(running)
        {
            XnStatus rc = XN_STATUS_OK;
            
            if(imageGenerator.IsGenerating())
            {
                //read a new RGB frame
                rc = imageGenerator.WaitAndUpdateData();
                if (rc != XN_STATUS_OK)
                {
                    printf("RGB read failed: %s\n", xnGetStatusString(rc));
                    break;
                }
                
                //get the rgb metadata
                imageGenerator.GetMetaData(imageMetaData);
            }
            
            if(depthGenerator.IsGenerating())
            {
                //read a new Depth frame
                rc = depthGenerator.WaitAndUpdateData();
                if (rc != XN_STATUS_OK)
                {
                    printf("Depth read failed: %s\n", xnGetStatusString(rc));
                    break;
                }
                
                //get the depth metadata
                depthGenerator.GetMetaData(depthMetaData);
                //calculate the histogram
                calculateHistogram();
            }
            
            if(infraredGenerator.IsGenerating())
            {
                //read a new Infrared frame
                rc = infraredGenerator.WaitAndUpdateData();
                if (rc != XN_STATUS_OK)
                {
                    printf("IR read failed: %s\n", xnGetStatusString(rc));
                    break;
                }
                
                //get the depth metadata
                infraredGenerator.GetMetaData(infraredMetaData);
            }
            
            if(userGenerator.IsGenerating())
            {
                //read a new User frame
                userGenerator.WaitAndUpdateData();
                //get the user pixels
                userGenerator.GetUserPixels(0, sceneMetaData);
            }
            
            //rgb image
            if(asRGBEnabled)
            {
                lockRGBMutex();
                rgbFrameHandler();
                unlockRGBMutex();
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "RGBFrame");
            }
            
            //depth image
            if(asDepthEnabled)
            {
                lockDepthMutex();
                if(asDepthShowUserColors)
                {
                    depthFrameWithUserColorsHandler();
                }
                else
                {
                    depthFrameHandler();
                }
                unlockDepthMutex();
                //dispatch depth frame event
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "depthFrame");
            }
            
            //infrared image - not available when rgb is enabled
            if(asInfraredEnabled && !imageGenerator.IsGenerating())
            {
                lockInfraredMutex();
                infraredHandler();
                unlockInfraredMutex();
                //dispatch infrared frame event
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "infraredFrame");
            }
            
            //point cloud
            if(asPointCloudEnabled)
            {
                lockPointCloudMutex();
                if(asPointCloudIncludeRGB)
                {
                    pointCloudWithRGBHandler();
                }
                else
                {
                    pointCloudHandler();
                }
                unlockPointCloudMutex();
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "pointCloudFrame");
            }
            
            //user information
            if(asUserEnabled || asSkeletonEnabled || asUserMaskEnabled)
            {
                lockUserMutex();
                userHandler();
                unlockUserMutex();
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "userFrame");
            }
            
            //user mask image
            if(asUserMaskEnabled)
            {
                lockUserMaskMutex();
                userMaskHandler();
                unlockUserMaskMutex();
                //dispatch user mask frame event
                FREDispatchStatusEventAsync(freContext, (const uint8_t*) "status", (const uint8_t*) "userMaskFrame");
            }
        }
    }
}

void OpenNIDevice::rgbFrameHandler()
{
    RGBFrameBuffer = imageMetaData.RGB24Data();
    
    uint32_t *rgbRun = RGBByteArray;
    int direction = asRGBMirrored ? -1 : 1;
    int directionFactor = asRGBMirrored ? 1 : 0;
    
    for(uint32_t y = 0; y < asRGBHeight; y++)
    {
        const XnRGB24Pixel *pRGBBuffer = RGBFrameBuffer + ((y + directionFactor) * (rgbWidth * rgbScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asRGBWidth; x++)
        {
            *rgbRun = 0xff << 24 | ((*pRGBBuffer).nBlue + ((*pRGBBuffer).nGreen << 8) + ((*pRGBBuffer).nRed << 16));
            rgbRun++;
            pRGBBuffer += (rgbScale * direction);
        }
    }
}

void OpenNIDevice::depthFrameHandler()
{
    depthFrameBuffer = depthMetaData.Data();
    
    uint32_t *depthRun = depthByteArray;
    int direction = asDepthMirrored ? -1 : 1;
    int directionFactor = asDepthMirrored ? 1 : 0;
    
    unsigned int red, green, blue;
    float value;
    
    for(uint32_t y = 0; y < asDepthHeight; y++)
    {
        const XnDepthPixel *pDepthBuffer = depthFrameBuffer + ((y + directionFactor) * (depthWidth * depthScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asDepthWidth; x++)
        {
            //get histogram pixel
            value = 0;
            
            if (*pDepthBuffer != 0)
            {
                value = depthHistogram[*pDepthBuffer];
            }
            
            red = ((int) (value * 1)) << 16;
            green = ((int) (value * 1)) << 8;
            blue = ((int) (value * 1));
            
            *depthRun = 0xff << 24 | (red + green + blue);
            
            pDepthBuffer += (depthScale * direction);
            depthRun++;
        }
    }
}

void OpenNIDevice::depthFrameWithUserColorsHandler()
{
    depthFrameBuffer = depthMetaData.Data();
    sceneFrameBuffer = sceneMetaData.Data();
    
    uint32_t *depthRun = depthByteArray;
    int direction = asDepthMirrored ? -1 : 1;
    int directionFactor = asDepthMirrored ? 1 : 0;
    
    unsigned int red, green, blue;
    float value;
    
    for(uint32_t y = 0; y < asDepthHeight; y++)
    {
        const XnDepthPixel *pDepthBuffer = depthFrameBuffer + ((y + directionFactor) * (depthWidth * depthScale)) - directionFactor;
        
        const XnLabel *pSceneBuffer = sceneFrameBuffer + ((y + directionFactor) * (depthWidth * depthScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asDepthWidth; x++)
        {
            //get histogram pixel
            value = 0;
            
            if (*pDepthBuffer != 0)
            {
                value = depthHistogram[*pDepthBuffer];
            }
            
            short label = *pSceneBuffer;
            
            if(label == 0)
            {
                red = ((int) value) << 16;
                green = ((int) value) << 8;
                blue = ((int) value);
            }
            else
            {
                if(userIndexColors[label][3] == 1)
                {
                    red = ((int) (value * userIndexColors[label][0])) << 16;
                    green = ((int) (value * userIndexColors[label][1])) << 8;
                    blue = ((int) (value * userIndexColors[label][2]));
                }
                else
                {
                    red = ((int) (0xFF && userIndexColors[label][0])) << 16;
                    green = ((int) (0xFF && userIndexColors[label][1])) << 8;
                    blue = ((int) (0xFF && userIndexColors[label][2]));
                }
            }
            
            *depthRun = 0xff << 24 | (red + green + blue);
            
            pDepthBuffer += (depthScale * direction);
            pSceneBuffer += (depthScale * direction);
            depthRun++;
        }
    }
}

void OpenNIDevice::userMaskHandler()
{
    //we need depth, rgb & scene info
    RGBFrameBuffer = imageMetaData.RGB24Data();
    sceneFrameBuffer = sceneMetaData.Data();
    
    int direction = asUserMaskMirrored ? -1 : 1;
    int directionFactor = asUserMaskMirrored ? 1 : 0;
    
    int pixelNr = 0;
    for(uint32_t y = 0; y < asUserMaskHeight; y++)
    {
        const XnRGB24Pixel *pRGBBuffer = RGBFrameBuffer + ((y + directionFactor) * (rgbWidth * userMaskScale)) - directionFactor;
        const XnLabel *pSceneBuffer = sceneFrameBuffer + ((y + directionFactor) * (depthWidth * userMaskScale)) - directionFactor;
        for(uint32_t x = 0; x < asUserMaskWidth; x++)
        {
            XnLabel label = *pSceneBuffer;
            
            for(int i = 0; i < MAX_SKELETONS; i++)
            {
                userMaskByteArray[i][pixelNr] = 0;
            }
            if(label > 0)
            {
                userMaskByteArray[label - 1][pixelNr] = 0xff << 24 | ((*pRGBBuffer).nBlue + ((*pRGBBuffer).nGreen << 8) + ((*pRGBBuffer).nRed << 16));
            }
            
            pRGBBuffer += (userMaskScale * direction);
            pSceneBuffer += (userMaskScale * direction);
            pixelNr++;
        }
    }
    
}

void OpenNIDevice::infraredHandler()
{
    infraredFrameBuffer = infraredMetaData.Data();
    
    uint32_t *depthRun = infraredByteArray;
    int direction = asInfraredMirrored ? -1 : 1;
    int directionFactor = asInfraredMirrored ? 1 : 0;
    
    unsigned int red, green, blue;
    float value;
    
    for(uint32_t y = 0; y < asInfraredHeight; y++)
    {
        const XnIRPixel *pInfraredBuffer = infraredFrameBuffer + ((y + directionFactor) * (infraredWidth * infraredScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asInfraredWidth; x++)
        {
            value = *pInfraredBuffer;
            
            red = ((int) (value * 1)) << 16;
            green = ((int) (value * 1)) << 8;
            blue = ((int) (value * 1));
            
            *depthRun = 0xff << 24 | (red + green + blue);
            
            pInfraredBuffer += (infraredScale * direction);
            depthRun++;
        }
    }
}

void OpenNIDevice::pointCloudHandler()
{
    depthFrameBuffer = depthMetaData.Data();
    
    short *pointCloudRun = pointCloudByteArray;
    int direction = asPointCloudMirrored ? -1 : 1;
    int directionFactor = asPointCloudMirrored ? 1 : 0;
    
    if(pointCloudRegions != 0)
    {
        for(int i = 0; i < numRegions; i++)
        {
            pointCloudRegions[i].numPoints = 0;
        }
    }
    
    for(uint32_t y = 0; y < asPointCloudHeight; y+=asPointCloudDensity)
    {
        const XnDepthPixel *pDepthBuffer = depthFrameBuffer + ((y + directionFactor) * (pointCloudWidth * pointCloudScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asPointCloudWidth; x+=asPointCloudDensity)
        {
            //write to point cloud
            *pointCloudRun = x;
            pointCloudRun++;
            *pointCloudRun = y;
            pointCloudRun++;
            *pointCloudRun = *pDepthBuffer;
            pointCloudRun++;
            
            //check regions
            for(int i = 0; i < numRegions; i++)
            {
                if(
                   x >= pointCloudRegions[i].x && x <= pointCloudRegions[i].maxX &&
                   y >= pointCloudRegions[i].y && y <= pointCloudRegions[i].maxY &&
                   *pDepthBuffer >= pointCloudRegions[i].z && *pDepthBuffer <= pointCloudRegions[i].maxZ
                )
                {
                    pointCloudRegions[i].numPoints++;
                }
            }
            
            pDepthBuffer += (pointCloudScale * direction * asPointCloudDensity);
        }
    }
}

void OpenNIDevice::pointCloudWithRGBHandler()
{
    RGBFrameBuffer = imageMetaData.RGB24Data();
    depthFrameBuffer = depthMetaData.Data();
    
    short *pointCloudRun = pointCloudByteArray;
    int direction = asPointCloudMirrored ? -1 : 1;
    int directionFactor = asPointCloudMirrored ? 1 : 0;
    
    if(pointCloudRegions != 0)
    {
        for(int i = 0; i < numRegions; i++)
        {
            pointCloudRegions[i].numPoints = 0;
        }
    }
    
    for(uint32_t y = 0; y < asPointCloudHeight; y+=asPointCloudDensity)
    {
        const XnRGB24Pixel *pRGBBuffer = RGBFrameBuffer + ((y + directionFactor) * (rgbWidth * pointCloudScale)) - directionFactor;
        const XnDepthPixel *pDepthBuffer = depthFrameBuffer + ((y + directionFactor) * (pointCloudWidth * pointCloudScale)) - directionFactor;
        
        for(uint32_t x = 0; x < asPointCloudWidth; x+=asPointCloudDensity)
        {
            //write to point cloud
            *pointCloudRun = x;
            pointCloudRun++;
            *pointCloudRun = y;
            pointCloudRun++;
            *pointCloudRun = *pDepthBuffer;
            pointCloudRun++;
            *pointCloudRun = (*pRGBBuffer).nRed;
            pointCloudRun++;
            *pointCloudRun = (*pRGBBuffer).nGreen;
            pointCloudRun++;
            *pointCloudRun = (*pRGBBuffer).nBlue;
            pointCloudRun++;
            
            //check regions
            for(int i = 0; i < numRegions; i++)
            {
                if(
                   x >= pointCloudRegions[i].x && x <= pointCloudRegions[i].maxX &&
                   y >= pointCloudRegions[i].y && y <= pointCloudRegions[i].maxY &&
                   *pDepthBuffer >= pointCloudRegions[i].z && *pDepthBuffer <= pointCloudRegions[i].maxZ
                   )
                {
                    pointCloudRegions[i].numPoints++;
                }
            }
            
            pRGBBuffer += (pointCloudScale * direction * asPointCloudDensity);
            pDepthBuffer += (pointCloudScale * direction * asPointCloudDensity);
        }
    }
}

void OpenNIDevice::userHandler()
{
    //clear the current users
    memset(&userFrame.users[0], 0, sizeof(userFrame.users));
    
    XnUserID aUsers[MAX_SKELETONS];
    XnUInt16 nUsers = MAX_SKELETONS;
    XnUInt16 trackedUsers = userGenerator.GetNumberOfUsers();
    XnPoint3D position;
    XnStatus rc;
    
    userGenerator.GetUsers(aUsers, nUsers);
    
    userFrame.frameNumber = userGenerator.GetFrameID();
    userFrame.timeStamp = (int) (userGenerator.GetTimestamp() / 1000);
    
    for (int i = 0; i < MAX_SKELETONS; ++i)
    {
        if(i < trackedUsers)
        {
            rc = userGenerator.GetCoM(aUsers[i], position);
            userFrame.users[i].isTracking = true;
            userFrame.users[i].userID = aUsers[i];
            userFrame.users[i].trackingID = aUsers[i];
            userFrame.users[i].hasSkeleton = (asSkeletonEnabled && userGenerator.GetSkeletonCap().IsTracking(aUsers[i]));
            
            userFrame.users[i].worldX = position.X;
            userFrame.users[i].worldY = position.Y;
            userFrame.users[i].worldZ = position.Z;
            
            userFrame.users[i].worldRelativeX = (depthWidth - position.X) / (depthWidth * 2) - .5;
            //if(asSkeletonMirrored) skeleton.joints[targetIndex].x = .5 - skeleton.joints[targetIndex].x;
            userFrame.users[i].worldRelativeY = -1 * (((depthHeight - position.Y) / (depthHeight * 2)) - .5);
            userFrame.users[i].worldRelativeZ = (position.Z * 7.8125) / MAX_DEPTH;
            
            
            //depth & rgb space are the same, as we aligned the depth & image streams
            XnPoint3D pt[1];
            pt[0] = position;
            depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
            
            userFrame.users[i].rgbRelativeX = userFrame.users[i].depthRelativeX = pt[0].X / depthWidth;
            userFrame.users[i].rgbRelativeY = userFrame.users[i].depthRelativeY = pt[0].Y / depthHeight;
            
            //take mirrored rgb / depth images into account
            if(asRGBMirrored) userFrame.users[i].rgbRelativeX = 1 - userFrame.users[i].rgbRelativeX;
            if(asDepthMirrored) userFrame.users[i].depthRelativeX = 1 - userFrame.users[i].depthRelativeX;
            
            userFrame.users[i].rgbX = (int) (userFrame.users[i].rgbRelativeX * asRGBWidth);
            userFrame.users[i].rgbY = (int) (userFrame.users[i].rgbRelativeY * asRGBHeight);
            userFrame.users[i].depthX = (int) (userFrame.users[i].depthRelativeX * asDepthWidth);
            userFrame.users[i].depthY = (int) (userFrame.users[i].depthRelativeY * asDepthHeight);
            
            if (userFrame.users[i].hasSkeleton)
            {
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_HEAD, 0);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_NECK, 1);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_TORSO, 2);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_WAIST, 3);
                
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_COLLAR, 4);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_SHOULDER, 5);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_ELBOW, 6);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_WRIST, 7);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_HAND, 8);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_FINGERTIP, 9);
                
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_COLLAR, 10);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_SHOULDER, 11);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_ELBOW, 12);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_WRIST, 13);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_HAND, 14);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_FINGERTIP, 15);
                
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_HIP, 16);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_KNEE, 17);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_ANKLE, 18);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_LEFT_FOOT, 19);
                
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_HIP, 20);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_KNEE, 21);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_ANKLE, 22);
                addJointElement(userFrame.users[i], aUsers[i], XN_SKEL_RIGHT_FOOT, 23);
            }
        }
        else
        {
            userFrame.users[i].isTracking = false;
        }
    }
    
}

void OpenNIDevice::addJointElement(kinectUser &kUser, XnUserID user, XnSkeletonJoint eJoint, uint32_t targetIndex)
{
    float jointPositionX, jointPositionY, jointPositionZ, jointW, jointPositionConfidence;
    
    XnSkeletonJointPosition jointPosition;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user, eJoint, jointPosition);
    
    XnSkeletonJointOrientation orientation;
    userGenerator.GetSkeletonCap().GetSkeletonJointOrientation(user, eJoint, orientation);
    
    jointPositionX = jointPosition.position.X;
    jointPositionY = jointPosition.position.Y;
    jointPositionZ = jointPosition.position.Z;
    jointW = 0;
    jointPositionConfidence = jointPosition.fConfidence;
    
    kUser.joints[targetIndex].orientationConfidence = orientation.fConfidence;
    
    kUser.joints[targetIndex].orientationX = atan2f(orientation.orientation.elements[7], orientation.orientation.elements[8]);
    kUser.joints[targetIndex].orientationY = -asinf(orientation.orientation.elements[6]);
    kUser.joints[targetIndex].orientationZ = atan2f(orientation.orientation.elements[3], orientation.orientation.elements[0]);
    
    kUser.joints[targetIndex].positionConfidence = jointPositionConfidence;

    kUser.joints[targetIndex].worldX = jointPositionX;
    //if(asSkeletonMirrored) skeleton.joints[targetIndex].x = .5 - skeleton.joints[targetIndex].x;
    kUser.joints[targetIndex].worldY = jointPositionY;
    kUser.joints[targetIndex].worldZ = jointPositionZ;
    
    kUser.joints[targetIndex].worldRelativeX = (depthWidth - jointPositionX) / (depthWidth * 2) - .5;
    //if(asSkeletonMirrored) skeleton.joints[targetIndex].x = .5 - skeleton.joints[targetIndex].x;
    kUser.joints[targetIndex].worldRelativeY = -1 * (((depthHeight - jointPositionY) / (depthHeight * 2)) - .5);
    kUser.joints[targetIndex].worldRelativeZ = (jointPositionZ * 7.8125) / MAX_DEPTH;
    
    
    //depth & rgb space are the same, as we aligned the depth & image streams
    XnPoint3D pt[1];
    pt[0] = jointPosition.position;
    depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
    
    kUser.joints[targetIndex].rgbRelativeX = kUser.joints[targetIndex].depthRelativeX = pt[0].X / depthWidth;
    kUser.joints[targetIndex].rgbRelativeY = kUser.joints[targetIndex].depthRelativeY = pt[0].Y / depthHeight;
    
    //take mirrored rgb / depth images into account
    if(asRGBMirrored) kUser.joints[targetIndex].rgbRelativeX = 1 - kUser.joints[targetIndex].rgbRelativeX;
    if(asDepthMirrored) kUser.joints[targetIndex].depthRelativeX = 1 - kUser.joints[targetIndex].depthRelativeX;
    
    kUser.joints[targetIndex].rgbX = (int) (kUser.joints[targetIndex].rgbRelativeX * asRGBWidth);
    kUser.joints[targetIndex].rgbY = (int) (kUser.joints[targetIndex].rgbRelativeY * asRGBHeight);
    kUser.joints[targetIndex].depthX = (int) (kUser.joints[targetIndex].depthRelativeX * asDepthWidth);
    kUser.joints[targetIndex].depthY = (int) (kUser.joints[targetIndex].depthRelativeY * asDepthHeight);
}

void OpenNIDevice::calculateHistogram()
{
    depthFrameBuffer = depthMetaData.Data();
    
    xnOSMemSet(depthHistogram, 0, MAX_DEPTH*sizeof(float));
    
    unsigned int nNumberOfPoints = 0;
    for (XnUInt y = 0; y < depthHeight; ++y)
    {
        for (XnUInt x = 0; x < depthWidth; ++x, ++depthFrameBuffer)
        {
            
            if (*depthFrameBuffer != 0)
            {
                depthHistogram[*depthFrameBuffer]++;
                nNumberOfPoints++;
                
            }
        }
    }
    for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
    {
        depthHistogram[nIndex] += depthHistogram[nIndex-1];
    }
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
        {
            depthHistogram[nIndex] = (unsigned int)(256 * (1.0f - (depthHistogram[nIndex] / nNumberOfPoints)));
        }
    }
}

void OpenNIDevice::setUserMode(bool mirrored)
{
    //printf("OpenNIDevice::setUserMode(%s)\n", (mirrored) ? "true" : "false");
    asUserMirrored = mirrored;
}

void OpenNIDevice::setUserColor(int userID, int color, bool useIntensity)
{
	if(userID > MAX_SKELETONS) return;
	
    userIndexColors[userID - 1][0] = (0xFF & (color >> 16)) / 255.0f;
    userIndexColors[userID - 1][1] = (0xFF & (color >> 8)) / 255.0f;
    userIndexColors[userID - 1][2] = (0xFF & (color)) / 255.0f;
    userIndexColors[userID - 1][3] = useIntensity ? 1 : 0;
}

void OpenNIDevice::setUserEnabled(bool enabled)
{
    //printf("OpenNIDevice::setSkeletonEnabled(%s)\n", (enabled) ? "true" : "false");
    asUserEnabled = enabled;
}

void OpenNIDevice::setSkeletonMode(bool mirrored)
{
    //printf("OpenNIDevice::setSkeletonMode(%s)\n", (mirrored) ? "true" : "false");
    asSkeletonMirrored = mirrored;
}

void OpenNIDevice::setSkeletonEnabled(bool enabled)
{
    //printf("OpenNIDevice::setSkeletonEnabled(%s)\n", (enabled) ? "true" : "false");
    asSkeletonEnabled = enabled;
}

void OpenNIDevice::setDepthMode(unsigned int width, unsigned int height, bool mirrored)
{
    //printf("OpenNIDevice::setDepthMode(%i, %i, %s)\n", width, height, (mirrored) ? "true" : "false");
    
    lockDepthMutex();
    
    asDepthWidth = width;
    asDepthHeight = height;
    asDepthPixelCount = asDepthWidth * asDepthHeight;
    asDepthMirrored = mirrored;
    depthScale = depthWidth / asDepthWidth;
    
    //reset bytearray
    if(depthByteArray != 0) delete [] depthByteArray;
    depthByteArray = new uint32_t[asDepthPixelCount];
    
    unlockDepthMutex();
}

void OpenNIDevice::setDepthEnabled(bool enabled)
{
    //printf("OpenNIDevice::setDepthEnabled(%s)\n", (enabled) ? "true" : "false");
    asDepthEnabled = enabled;
}

void OpenNIDevice::setDepthShowUserColors(bool show)
{
    //printf("OpenNIDevice::setDepthShowUserColors(%s)\n", (show) ? "true" : "false");
    asDepthShowUserColors = show;
}

void OpenNIDevice::setRGBMode(unsigned int width, unsigned int height, bool mirrored)
{
    //printf("OpenNIDevice::setRGBMode(%i, %i, %s)\n", width, height, (mirrored) ? "true" : "false");
    
    lockRGBMutex();
    
    asRGBWidth = width;
    asRGBHeight = height;
    asRGBPixelCount = asRGBWidth * asRGBHeight;
    asRGBMirrored = mirrored;
    rgbScale = rgbWidth / asRGBWidth;
    
    //reset bytearray
    if(RGBByteArray != 0) delete [] RGBByteArray;
    RGBByteArray = new uint32_t[asRGBPixelCount];
    
    unlockRGBMutex();
}

void OpenNIDevice::setRGBEnabled(bool enabled)
{
    //printf("OpenNIDevice::setRGBEnabled(%s)\n", (enabled) ? "true" : "false");
    asRGBEnabled = enabled;
}

void OpenNIDevice::setUserMaskMode(unsigned int width, unsigned int height, bool mirrored)
{
    //printf("OpenNIDevice::setUserMaskMode(%i, %i, %s)\n", width, height, (mirrored) ? "true" : "false");
    
    lockUserMaskMutex();
    
    asUserMaskWidth = width;
    asUserMaskHeight = height;
    asUserMaskPixelCount = asUserMaskWidth * asUserMaskHeight;
    asUserMaskMirrored = mirrored;
    userMaskScale = userMaskWidth / asUserMaskWidth;
    
    //reset bytearray
    if(userMaskByteArray != 0)
    {
        for(int i = 0; i < MAX_SKELETONS; i++)
        {
            delete [] userMaskByteArray[i];
        }
        delete [] userMaskByteArray;
    }
    userMaskByteArray = new uint32_t*[MAX_SKELETONS];
    for(int i = 0; i < MAX_SKELETONS; i++)
    {
        userMaskByteArray[i] = new uint32_t[asUserMaskPixelCount];
    }
    
    unlockUserMaskMutex();
}

void OpenNIDevice::setUserMaskEnabled(bool enabled)
{
    //printf("OpenNIDevice::setUserMaskEnabled(%s)\n", (enabled) ? "true" : "false");
    asUserMaskEnabled = enabled;
}

void OpenNIDevice::setInfraredMode(unsigned int width, unsigned int height, bool mirrored)
{
    //printf("OpenNIDevice::setInfraredMode(%i, %i, %s)\n", width, height, (mirrored) ? "true" : "false");
    
    lockInfraredMutex();
    
    asInfraredWidth = width;
    asInfraredHeight = height;
    asInfraredPixelCount = asInfraredWidth * asInfraredHeight;
    asInfraredMirrored = mirrored;
    infraredScale = infraredWidth / asInfraredWidth;
    
    if(infraredByteArray != 0) delete [] infraredByteArray;
    infraredByteArray = new uint32_t[asInfraredPixelCount];
    
    unlockInfraredMutex();
}

void OpenNIDevice::setInfraredEnabled(bool enabled)
{
    //printf("OpenNIDevice::setInfraredEnabled(%s)\n", (enabled) ? "true" : "false");
    asInfraredEnabled = enabled;
}

void OpenNIDevice::setPointCloudMode(unsigned int width, unsigned int height, bool mirrored, unsigned int density, bool includeRGB)
{
    //printf("OpenNIDevice::setPointCloudMode(%i, %i, %s)\n", width, height, (mirrored) ? "true" : "false");
    
    lockPointCloudMutex();
    
    asPointCloudWidth = width;
    asPointCloudHeight = height;
    asPointCloudDensity = density;
    asPointCloudIncludeRGB = includeRGB;
    asPointCloudPixelCount = (asPointCloudWidth * asPointCloudHeight) / asPointCloudDensity;
    asPointCloudMirrored = mirrored;
    pointCloudScale = pointCloudWidth / asPointCloudWidth;
    
    if(pointCloudByteArray != 0) delete [] pointCloudByteArray;
    if(asPointCloudIncludeRGB)
    {
        pointCloudByteArray = new short[asPointCloudPixelCount * 6];
    }
    else
    {
        pointCloudByteArray = new short[asPointCloudPixelCount * 3];
    }
    
    unlockPointCloudMutex();
}

void OpenNIDevice::setPointCloudEnabled(bool enabled)
{
    //printf("OpenNIDevice::setPointCloudEnabled(%s)\n", (enabled) ? "true" : "false");
    asPointCloudEnabled = enabled;
}

void OpenNIDevice::setPointCloudRegions(PointCloudRegion *pointCloudRegions, unsigned int numRegions)
{
    this->pointCloudRegions = pointCloudRegions;
    this->numRegions = numRegions;
}

void OpenNIDevice::dispose()
{
    printf("OpenNIDevice::dispose()\n");
    //make sure threads are stopped
    stop();
    //the context of this instance will be destroyed, cleanup everything of this instance
    freContext = NULL;
}

//user tracking callbacks
void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
    if(needPose)
    {
        status = userGenerator->GetPoseDetectionCap().StartPoseDetection(strPose, nID);
        printf("New user %i, waiting for calibration pose... status: %s\n", nID, xnGetStatusString(status));
    }
    else
    {
        userGenerator->GetSkeletonCap().RequestCalibration(nID, TRUE);
    }
	
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().StopTracking(nID);
	status = userGenerator->GetSkeletonCap().Reset(nID);
	printf("Stopped tracking of user %i... status: %s\n", nID, xnGetStatusString(status));
}

void XN_CALLBACK_TYPE userExitCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().Reset(nID);
    printf("User %i exited - resetting skeleton data... status: %s\n", nID, xnGetStatusString(status));
}

void XN_CALLBACK_TYPE userReentryCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	XnChar calibrationPose[20];
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
	status = userGenerator->GetSkeletonCap().GetCalibrationPose(calibrationPose);
	status = userGenerator->GetPoseDetectionCap().StartPoseDetection(calibrationPose, nID);
    
    printf("User %i reentered, waiting for calibration pose... status: %s\n", nID, xnGetStatusString(status));
}

void XN_CALLBACK_TYPE poseDetectedCallback(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
	status = userGenerator->GetPoseDetectionCap().StopPoseDetection(nID);
	status = userGenerator->GetSkeletonCap().RequestCalibration(nID, true);
    
    printf("Pose detected for user %i, starting calibration... status: %s\n", nID, xnGetStatusString(status));
}

void XN_CALLBACK_TYPE outOfPoseCallback(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie)
{
	XnStatus status;
	XnChar calibrationPose[20];
	xn::UserGenerator* userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().GetCalibrationPose(calibrationPose);
	status = userGenerator->GetPoseDetectionCap().StartPoseDetection(calibrationPose, nID);
	printf("Out of pose, waiting for calibration pose of user %i... status: %s\n", nID, xnGetStatusString(status));
}

void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& rCapability, XnUserID nID, void* pCookie)
{
	printf("Starting calibration of user %i\n", nID);
}

void XN_CALLBACK_TYPE calibrationCompleteCallback(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus calibrationError, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator* userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
	if (calibrationError == XN_STATUS_OK)
	{
		status = userGenerator->GetSkeletonCap().StartTracking(nID);
		printf("Calibration success, beginning to track user %i now... status: %s\n", nID, xnGetStatusString(status));	}
	else
	{
		XnChar calibrationPose[20];
		status = userGenerator->GetSkeletonCap().GetCalibrationPose(calibrationPose);
		status = userGenerator->GetPoseDetectionCap().StartPoseDetection(calibrationPose, nID);
		printf("Calibration failure for user %i, waiting for calibration pose... status: %s\n", nID, xnGetStatusString(status));
	}
}