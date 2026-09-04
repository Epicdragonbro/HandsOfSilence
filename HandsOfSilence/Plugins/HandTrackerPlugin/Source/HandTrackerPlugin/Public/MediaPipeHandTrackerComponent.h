//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once
#include "Engine/Texture2D.h"
#include "Components/LineBatchComponent.h"
#include "MediaPipeHandTrackerComponent.generated.h"


UCLASS(ClassGroup = MediaPipe, BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UMediaPipeHandTrackerComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()


public:
	
		void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
		UFUNCTION(BlueprintCallable,  Category = "Hand tracker")
		bool InitGraph();

		UFUNCTION(BlueprintCallable,  Category = "Hand tracker")
		bool ShutdownGraph();
		
		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void GetHandLandmarksFromRenderTarget2D(UTextureRenderTarget2D* TextureRenderTarget);

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void GetHandLandmarksFromTexture2D(UTexture2D* Texture);

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void GetHandLandmarksFromMediaTexture(UMediaTexture* Texture);

		UFUNCTION(BlueprintCallable, BlueprintPure,  Category = "Hand tracker")
		FVector GetLandmarkPosition(int hand, int ID, bool bDepth=true);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hand tracker")
		FVector2D GetLandmarkPosition2D(int hand,int ID);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hand tracker")
		FString GetHandedness(int hand);

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hand tracker")
		int GetNumTrackedHands();

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void SetHFoV(float Degrees);

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		float GetHFoV(float Degrees);


		//DRAW HELPER Function TODO: Move to a separate component

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void DrawLine(FVector StartLocation, FVector EndLocation, FLinearColor Color, uint8 DepthPriority, float Thickness, float LifeTime);

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void DrawPoint(FVector Position, FLinearColor Color, float PointSize, uint8 DepthPriority, float LifeTime);

		UFUNCTION(BlueprintCallable, Category = "Hand tracker")
		void DrawCylinder(FVector const& Start, FVector const& End, float Radius, int32 Segments, FLinearColor const& Color, float LifeTime, uint8 DepthPriority, float Thickness);

		UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Hand tracker")
		bool IsWithEditor();

};