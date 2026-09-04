//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//


#include "MediaPipeHandTrackerComponent.h"
#include "MediaPipeConnector.h"


#include "IHandTrackerPlugin.h"


UMediaPipeHandTrackerComponent::UMediaPipeHandTrackerComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Make sure this component ticks
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bAutoActivate = true;
	bTickInEditor = true;
	bAutoActivate = true;
	
}

void UMediaPipeHandTrackerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction){
	IHandTrackerPlugin::GetMediaPipeConnectorSafe()->DeltaTime = DeltaTime;
}


void UMediaPipeHandTrackerComponent::GetHandLandmarksFromRenderTarget2D(UTextureRenderTarget2D* TextureRenderTarget) {
	if (TextureRenderTarget==nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IHandTrackerPlugin::GetMediaPipeConnectorSafe()->TextureRenderTarget2DToUint8(TextureRenderTarget);

	//Process the raw frame
	int32 SizeX = TextureRenderTarget->SizeX;
	int32 SizeY = TextureRenderTarget->SizeY;

	IHandTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}

void UMediaPipeHandTrackerComponent::GetHandLandmarksFromTexture2D(UTexture2D* Texture) {
	if (Texture == nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IHandTrackerPlugin::GetMediaPipeConnectorSafe()->Texture2DToUint8(Texture);

	//Process the raw frame
	int32 SizeX = Texture->GetSizeX();
	int32 SizeY = Texture->GetSizeY();

	IHandTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}

void UMediaPipeHandTrackerComponent::GetHandLandmarksFromMediaTexture(UMediaTexture* Texture) {
	if (Texture == nullptr) return;

	//Get raw pixel data from the input texture
	uint8* RawFrame = IHandTrackerPlugin::GetMediaPipeConnectorSafe()->MediaTextureToUint8(Texture);

	//Process the raw frame
	int32 SizeX = Texture->GetWidth();
	int32 SizeY = Texture->GetHeight();

	IHandTrackerPlugin::GetMediaPipeConnectorSafe()->ProcessFrame(SizeY, SizeX, RawFrame);

}




bool UMediaPipeHandTrackerComponent::InitGraph() {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->InitGraph();
}

bool UMediaPipeHandTrackerComponent::ShutdownGraph() {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->ShutdownGraph();
}

FVector UMediaPipeHandTrackerComponent::GetLandmarkPosition(int hand, int ID, bool bDepth) {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkPosition(hand, ID, bDepth);
}

FVector2D UMediaPipeHandTrackerComponent::GetLandmarkPosition2D(int hand, int ID) {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->GetLandmarkPosition2D(hand,ID);
}

FString UMediaPipeHandTrackerComponent::GetHandedness(int hand) {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->GetHandedness(hand);
}

int UMediaPipeHandTrackerComponent::GetNumTrackedHands() {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->GetNumTrackedHands();
}

void UMediaPipeHandTrackerComponent::DrawLine(FVector StartLocation, FVector EndLocation, FLinearColor Color, uint8 DepthPriority, float Thickness, float LifeTime) {
	GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::Foreground)->DrawLine(StartLocation, EndLocation, Color, 1, Thickness, LifeTime);
}

void UMediaPipeHandTrackerComponent::DrawPoint(FVector Position, FLinearColor Color, float PointSize, uint8 DepthPriority, float LifeTime) {
	GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::Foreground)->DrawPoint(Position, Color, PointSize, DepthPriority, LifeTime);
}

void UMediaPipeHandTrackerComponent::SetHFoV(float Degrees) {
	IHandTrackerPlugin::GetMediaPipeConnectorSafe()->HFoV = Degrees;
}

float UMediaPipeHandTrackerComponent::GetHFoV(float Degrees) {
	return IHandTrackerPlugin::GetMediaPipeConnectorSafe()->HFoV;
}


void UMediaPipeHandTrackerComponent::DrawCylinder(FVector const& Start, FVector const& End, float Radius, int32 Segments, FLinearColor const& Color, float LifeTime, uint8 DepthPriority, float Thickness) {
	// From "/Engine/Source/Runtime/Engine/Private/DrawDebugHelpers.cpp"

	if (ULineBatchComponent* const LineBatcher = GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::Foreground))
	{
		// Need at least 4 segments
		Segments = FMath::Max(Segments, 4);

		// Rotate a point around axis to form cylinder segments
		FVector Segment;
		FVector P1, P2, P3, P4;
		const float AngleInc = 360.f / Segments;
		float Angle = AngleInc;

		// Default for Axis is up
		FVector Axis = (End - Start).GetSafeNormal();
		if (Axis.IsZero())
		{
			Axis = FVector(0.f, 0.f, 1.f);
		}

		FVector Perpendicular;
		FVector Dummy;

		Axis.FindBestAxisVectors(Perpendicular, Dummy);

		Segment = Perpendicular.RotateAngleAxis(0, Axis) * Radius;
		P1 = Segment + Start;
		P3 = Segment + End;

		while (Segments--)
		{
			Segment = Perpendicular.RotateAngleAxis(Angle, Axis) * Radius;
			P2 = Segment + Start;
			P4 = Segment + End;

			LineBatcher->DrawLine(P2, P4, Color, DepthPriority, Thickness, LifeTime);
			LineBatcher->DrawLine(P1, P2, Color, DepthPriority, Thickness, LifeTime);
			LineBatcher->DrawLine(P3, P4, Color, DepthPriority, Thickness, LifeTime);

			P1 = P2;
			P3 = P4;
			Angle += AngleInc;
		}
	}
}

bool UMediaPipeHandTrackerComponent::IsWithEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

