#ifndef KinectExtension_AKMSSDKUser_h
#define KinectExtension_AKMSSDKUser_h

#include "../../ExtensionConfig.h"

#ifdef AIRKINECT_TARGET_MSSDK

#ifdef AIRKINECT_OS_WINDOWS
    #include "FlashRuntimeExtensions.h"
#else
    #include <Adobe AIR/Adobe AIR.h>
#endif

#include "../../Data/AKUser.h"
#include "AKMSSDKSkeletonJoint.h"
#include "AKMSSDKSkeletonBone.h"

#ifndef _AKMSSDKUser_
#define _AKMSSDKUser_
typedef struct _AKMSSDKUser
{
	AKUser* user;
	AKMSSDKSkeletonJoint* mssdkSkeletonJoints;
	AKMSSDKSkeletonBone* mssdkSkeletonBones;

	FREObject asFREObject()
	{
		FREObject freUser = user->asFREObjectWithoutJoints();
		
		FREObject freJoints;
		FREGetObjectProperty(freUser, (const uint8_t*) "skeletonJoints", &freJoints, NULL);
		for(int i = 0; i < this->user->numJoints; i++)
		{
			FREObject freJoint = this->mssdkSkeletonJoints[i].asFREObject();
			FRESetArrayElementAt(freJoints, i, freJoint);
		}
		FRESetObjectProperty(freUser, (const uint8_t*) "skeletonJoints", freJoints, NULL);

		FREObject freBones;
		FREGetObjectProperty(freUser, (const uint8_t*) "skeletonBones", &freBones, NULL);
		for(int i = 0; i < this->user->numBones; i++)
		{
			FREObject freBone = this->mssdkSkeletonBones[i].asFREObject();
			FRESetArrayElementAt(freBones, i, freBone);
		}
		FRESetObjectProperty(freUser, (const uint8_t*) "skeletonBones", freBones, NULL);

		return freUser;
	};

} AKMSSDKUser;
#endif _AKMSSDKUser_

#endif

#endif