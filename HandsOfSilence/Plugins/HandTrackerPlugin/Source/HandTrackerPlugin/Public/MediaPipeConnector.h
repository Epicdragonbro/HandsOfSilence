//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "MediaTexture.h"




//MediaPipe log
DECLARE_LOG_CATEGORY_EXTERN(MediaPipe, Log, All);


class FMediaPipeHandConnector
{

	
public:
	FMediaPipeHandConnector();
	virtual ~FMediaPipeHandConnector();

	/** Startup the device, and do any initialization that may be needed */
	bool StartupConnector();

	/** Tear down the device */
	void ShutdownConnector();


	uint8* Texture2DToUint8(UTexture2D* Texture);
	uint8* MediaTextureToUint8(UMediaTexture* Texture);
	uint8* TextureRenderTarget2DToUint8(UTextureRenderTarget2D* CameraRenderTarget);
	void ProcessFrame(int rows, int cols, uint8* rawFrame);
	bool InitGraph();
	bool ShutdownGraph();
	float GetAspectRatio();
	
	FVector GetLandmarkPosition(int hand, int ID, bool bDepth=true);
	FVector2D GetLandmarkPosition2D(int hand, int ID);
	FString GetHandedness(int hand);
	int GetNumTrackedHands();
	
	
bool bIsInitiated;

	//Camera parameters
	float HFoV;		
	float VFoV;
	float RefDistance; 
	float Distance;
	
	//Need for fInterpTo
	float DeltaTime;
	
	

private:
	void FixDeadlock();
	
	/** Handle to the test dll we will load */
	void* MediaPipeLibraryHandle;
	void* OpenCVLibraryHandle;
	void* ProcessFrameFunctionHandle;
	void* InitGraphFunctionHandle;
	void* ShutdownGraphFunctionHandle;

	float AspectRatio;

	std::vector<std::vector<std::vector<double>>> multi_hand_landmarks_vector;
	std::vector<std::string> handedness;
	int num_tracked_hands;
	

};
