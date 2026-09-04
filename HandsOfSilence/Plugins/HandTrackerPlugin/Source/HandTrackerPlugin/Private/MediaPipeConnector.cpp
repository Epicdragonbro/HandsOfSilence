//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//


#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
//#define WITH_CONSOLE

#include "MediaPipeConnector.h"
#include "Interfaces/IPluginManager.h"
#include "Components/LineBatchComponent.h"
#include "Runtime/Launch/Resources/Version.h"


#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows.h"
#include "Core.h"
#include <string>
#include "stdio.h"
#include <iostream>
#include "Windows/HideWindowsPlatformTypes.h"
#endif 

#include "Runtime/MediaAssets/Private/Misc/MediaTextureResource.h"
#include "MediaPipeHand.h"
#include "DeadLockFix.h"



extern "C" {
	typedef bool (*process_frame_ptr)(int rows, int cols, uint8* raw_frame, std::vector<std::vector<std::vector<double>>>& multi_hand_landmarks_vector, std::vector<std::string>& handedness, int& num_tracked_hands);
	typedef bool (*init_graph_ptr)(LPCWSTR base_dir);
	typedef bool (*shutdown_graph_ptr)();
}

#define LOCTEXT_NAMESPACE "FMediaPipePluginModule"
DEFINE_LOG_CATEGORY(MediaPipe);

FMediaPipeHandConnector::FMediaPipeHandConnector()
{
	//TODO: Set camera params from blueprint
	HFoV = 90.0f;
	VFoV = 59.0f;

	RefDistance = 270.0f;
	Distance = RefDistance;
	
	bIsInitiated = false;
	
	

	// Console windows for debug
#ifdef WITH_CONSOLE
	AllocConsole();
	
	// std::cout, std::clog, std::cerr, std::cin
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	// std::wcout, std::wclog, std::wcerr, std::wcin
	HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
	SetStdHandle(STD_ERROR_HANDLE, hConOut);
	SetStdHandle(STD_INPUT_HANDLE, hConIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();
#endif

	AspectRatio = 1.0f;

	//Init the vector with some empty values
	
	std::vector<std::vector<double>>hand_landmarks_vector;
	for (int i = 0; i < 21; i++) {
		hand_landmarks_vector.push_back(std::vector<double>{0, 0, 0, 0, 0});
	}

	multi_hand_landmarks_vector.push_back(hand_landmarks_vector);
	multi_hand_landmarks_vector.push_back(hand_landmarks_vector);

	handedness.push_back("None");
	handedness.push_back("None");

	num_tracked_hands = 0;
	

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("HandTrackerPlugin")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
	FString OpenCVLibraryPath;

#if PLATFORM_WINDOWS
	
	#if WITH_EDITOR
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/HandTrackerPluginLibrary/Win64/mediapipe_hand.dll"));
		OpenCVLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/HandTrackerPluginLibrary/Win64/opencv_world3410.dll"));
	#else
		LibraryPath = TEXT("mediapipe_hand.dll");
		OpenCVLibraryPath = TEXT("opencv_world3410.dll");
	#endif //WITH EDITOR

#endif // PLATFORM_WINDOWS

	FixDeadlock();

	OpenCVLibraryHandle = nullptr;
	OpenCVLibraryHandle = FPlatformProcess::GetDllHandle(*OpenCVLibraryPath);

	if (OpenCVLibraryHandle)
	{
		//Do something here on module loaded
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "OpenCV loaded"));
	}
	else {
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "OpenCV NOT loaded"));
	}
	
	MediaPipeLibraryHandle = nullptr;
	MediaPipeLibraryHandle = FPlatformProcess::GetDllHandle(*LibraryPath);


	if (MediaPipeLibraryHandle)
	{
		//Do something here on module loaded
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryMessage", "Third party library loaded"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load MediaPipe third party library"));
	}
	
	
	UE_LOG(MediaPipe, Log, TEXT("Connector created."));
}

FMediaPipeHandConnector::~FMediaPipeHandConnector()
{
#ifdef WITH_CONSOLE
	FreeConsole();
#endif;

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(MediaPipeLibraryHandle);
	MediaPipeLibraryHandle = nullptr;

	FPlatformProcess::FreeDllHandle(OpenCVLibraryHandle);
	OpenCVLibraryHandle = nullptr;

	UE_LOG(MediaPipe, Log, TEXT("Connector shutdown"));

	
}

uint8* FMediaPipeHandConnector::TextureRenderTarget2DToUint8(UTextureRenderTarget2D* CameraRenderTarget) {
	// Read the pixels from the RenderTarget and store them in a FColor array
	TArray<FColor> ColorData;
	FRenderTarget* RenderTarget = CameraRenderTarget->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(ColorData);
	return (uint8*)ColorData.GetData();
}


uint8* FMediaPipeHandConnector::Texture2DToUint8(UTexture2D* Texture) {
	uint8* raw = NULL;
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_ONLY);
	raw = (uint8*)Data;
	Mip.BulkData.Unlock();
	return raw;
}

uint8* FMediaPipeHandConnector::MediaTextureToUint8(UMediaTexture* Texture) {
	TArray<FColor> ColorData;
	FMediaTextureResource* TexResource = static_cast<FMediaTextureResource*>(Texture->GetResource());
	TexResource->ReadPixels(ColorData);
	return (uint8*)ColorData.GetData();
}

bool FMediaPipeHandConnector::InitGraph() {
	bool result = false;
	InitGraphFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("initGraph"));
	auto init_graph_func = reinterpret_cast<init_graph_ptr>(InitGraphFunctionHandle);

	if (InitGraphFunctionHandle != nullptr) {
		FString ProjectDir;
#if WITH_EDITOR

		ProjectDir = FPaths::ProjectDir()+FString("Binaries/Win64");
		UE_LOG(MediaPipe, Log, TEXT("Binaries: %s"), *ProjectDir);
#else
		ProjectDir = "";
#endif //WITH_EDITOR
		
		
		char* dir = TCHAR_TO_ANSI(*ProjectDir);
		wchar_t wtext[255];
		mbstowcs(wtext, dir, strlen(dir) + 1);//Plus null
		LPWSTR ptr = wtext;

		result = init_graph_func(wtext);

		if (result == true) {
			UE_LOG(MediaPipe, Log, TEXT("Graph initialized"));
			bIsInitiated = true;
			return 1;
		}
	}
	else {
		UE_LOG(MediaPipe, Error, TEXT("Init graph function not found"));
	}
	return 0;
}

bool FMediaPipeHandConnector::ShutdownGraph() {
	bIsInitiated = false;
	bool result = false;
	
	ShutdownGraphFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("shutdownGraph"));
	auto shutdown_graph_func = reinterpret_cast<shutdown_graph_ptr>(ShutdownGraphFunctionHandle);

	if (ShutdownGraphFunctionHandle != nullptr) {
		result = shutdown_graph_func();

		if (result == true) {
			UE_LOG(MediaPipe, Log, TEXT("Graph shutdown completed."));
			return 1;
		}
	}
	else {
		UE_LOG(MediaPipe, Error, TEXT("shutdown graph function not found"));
	}
	return 0;
}


void FMediaPipeHandConnector::ProcessFrame(int rows, int cols, uint8* rawFrame) {

		//compute aspect ratio
	    AspectRatio = (float)cols / (float)rows;

		bool result = false;
		ProcessFrameFunctionHandle = FPlatformProcess::GetDllExport(MediaPipeLibraryHandle, *FString("processFrame"));
		auto process_frame_func = reinterpret_cast<process_frame_ptr>(ProcessFrameFunctionHandle);


		if (ProcessFrameFunctionHandle != nullptr) {
			result = process_frame_func(rows, cols, rawFrame, multi_hand_landmarks_vector, handedness, num_tracked_hands);

			

		}
		else {
			UE_LOG(MediaPipe, Error, TEXT("Process frame function not found"));
		}

}



FVector FMediaPipeHandConnector::GetLandmarkPosition(int hand, int ID, bool bDepth) {
	
	if (multi_hand_landmarks_vector.size() > hand) {
		if (multi_hand_landmarks_vector.at(hand).size() != 0 && multi_hand_landmarks_vector.at(hand).size() > ID) {

			//Projecting from 0-1 to -1 , 1 


			float x = (this->multi_hand_landmarks_vector.at(hand).at(ID).at(0) - 0.5f) * 2.0f;
			float y = (this->multi_hand_landmarks_vector.at(hand).at(ID).at(1) - 0.5f) * 2.0f;
			float z = this->multi_hand_landmarks_vector.at(hand).at(ID).at(2);

			//Ignore z if requested
			if (!bDepth) z = 0;

			float MultiplierRefH = FMath::Tan(FMath::DegreesToRadians(HFoV / 2.0f)) * RefDistance / 2.0f;

			VFoV = FMath::RadiansToDegrees(2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HFoV) / 2) * 1.0f / AspectRatio));
			//UE_LOG(MediaPipe, Log, TEXT("VFoV: %f"), VFoV);
			//UE_LOG(MediaPipe, Log, TEXT("Aspect Ratio: %f"), AspectRatio);
			//UE_LOG(MediaPipe, Log, TEXT("Distance: %f"), Distance);


			float Multiplier = FMath::Tan(FMath::DegreesToRadians(HFoV / 2.0f)) * Distance;
			float MultiplierV = FMath::Tan(FMath::DegreesToRadians(VFoV / 2.0f)) * Distance;

			return FVector(x * Multiplier, -z * MultiplierV / 2.0f, -y * MultiplierV) + FVector(0, -Distance, 0);
		}
	}

	return FVector(0, 0, 0);

}

FVector2D FMediaPipeHandConnector::GetLandmarkPosition2D(int hand,int ID) {
	if (multi_hand_landmarks_vector.size() > hand) {
		if (multi_hand_landmarks_vector.at(hand).size() != 0 && multi_hand_landmarks_vector.at(hand).size() > ID) {

			//Range from 0-1
			float x = this->multi_hand_landmarks_vector.at(hand).at(ID).at(0);
			float y = this->multi_hand_landmarks_vector.at(hand).at(ID).at(1);


			//TO DO: Need to convert texture space to real 3D space based on camera fov, and other parameters.
			return FVector2D(x, y);

		}
	}
	return FVector2D(0, 0);
}

float FMediaPipeHandConnector::GetAspectRatio() {
	return this->AspectRatio;
}

FString FMediaPipeHandConnector::GetHandedness(int hand) {
	if (handedness.size() > hand) {
		FString handedness_trimmed = FString(handedness.at(hand).c_str()).TrimEnd();
		
		//Mirror hands
		if(handedness_trimmed == "Left") return "Right";
		if(handedness_trimmed == "Right") return "Left";

	}
	return "";
}

int FMediaPipeHandConnector::GetNumTrackedHands() {
	return num_tracked_hands;
}


bool FMediaPipeHandConnector::StartupConnector() {
	
	//return InitGraph();
	return 1;
}

void FMediaPipeHandConnector::ShutdownConnector() {
	
	//Need this otherwise the module won't close, and the editor remains in the memory. 
	ShutdownGraph();
}

void FMediaPipeHandConnector::FixDeadlock()
{
#if (ENGINE_MAJOR_VERSION == 5)

	const char* Variants[] = {
		// 5.0.0
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 B5 C6 B5 00", // dev editor (unrealeditor-core.dll)
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 CC CC CC CC 48 33 C4 48 89 45 17 4D 63 68 3C 33 C0", // dev game
	};
	const int NumVariants = sizeof(Variants) / sizeof(Variants[0]);

	auto Process = GetCurrentProcess();

	for (int i = 0; i < NumVariants; ++i)
	{
		auto Pattern = CkParseByteArray(Variants[i]);
		std::vector<uint8_t*> Loc;
		auto Status = CkFindPatternIntern<CkWildcardCC>(Process, Pattern, 2, Loc);
		
		if (Status == 0 && Loc.size() == 1)
		{
			auto Fix = CkParseByteArray("C3");
			Status = CkProtectWriteMemory(Process, Fix, Loc[0], 0);
			return;
		}
	}

	

#endif
}