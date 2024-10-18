// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RGBCamera.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRGBCamera, Log, All);

UCLASS()
class EAGLERL_API ARGBCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARGBCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// SceneCaptureComponent2D to capture the RGB data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	class USceneCaptureComponent2D* CaptureComponent2D;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	class UTextureRenderTarget2D* RenderTarget2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	float DesiredFps = 30.0f;

	// Function to capture and send data over the network
	void CaptureAndSendData();

private:
	void InitializeSocket();
	void SendRGBData(const TArray<FColor>& ImageData);
	void SendDepthData(const TArray<float>& DepthData);

	FSocket* Socket;
	FTimerHandle TimerHandle;
	FDateTime LastCaptureTime;

	class FImageSenderRunnable* ImageSenderRunnable;
	class FImageQueue* ImageQueue;
	
	bool bSocketInitialized;
}; 