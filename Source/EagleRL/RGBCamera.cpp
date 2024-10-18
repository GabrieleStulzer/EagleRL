// Fill out your copyright notice in the Description page of Project Settings.


#include "RGBCamera.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtils.h"
#include <Interfaces/IPv4/IPv4Address.h>

#include "FImageSenderRunnable.h"
#include "MovieSceneSequenceID.h"
#include "Common/TcpSocketBuilder.h"

DEFINE_LOG_CATEGORY(LogRGBCamera)

// Sets default values
ARGBCamera::ARGBCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent2D"));
	CaptureComponent2D->SetupAttachment(RootComponent);


	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetObj(TEXT("/Game/RT_Capture.RT_Capture"));
    RenderTarget2D = RenderTargetObj.Object;
    CaptureComponent2D->TextureTarget = RenderTarget2D;

	bSocketInitialized = false;
}

// Called when the game starts or when spawned
void ARGBCamera::BeginPlay()
{
	Super::BeginPlay();
	InitializeSocket();
	ImageQueue = new FImageQueue();

	if(CaptureComponent2D && RenderTarget2D)
	{
		CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		CaptureComponent2D->bCaptureEveryFrame = false;
		CaptureComponent2D->bCaptureOnMovement = false;

		ImageSenderRunnable = new FImageSenderRunnable(ImageQueue, Socket);
	}

	float FrameInterval = 1.0f / DesiredFps;

	// GetWorldTimerManager().SetTimer(TimerHandle, this, &ARGBCamera::CaptureAndSendData, FrameInterval, true);
}

// Called every frame
void ARGBCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FDateTime CurrentTime = FDateTime::Now();
	float FrameInterval = 1.0f / DesiredFps;
	FTimespan TimeSinceLastCapture = CurrentTime - LastCaptureTime;
	if (TimeSinceLastCapture.GetTotalSeconds() >= FrameInterval)
	{
		CaptureAndSendData();
		LastCaptureTime = CurrentTime;
	}
	
}

void ARGBCamera::CaptureAndSendData()
{
	if (!bSocketInitialized || !Socket) return;

	if (!RenderTarget2D)
	{
		UE_LOG(LogRGBCamera, Error, TEXT("RenderTarget2D is not set"));
		return;
	}

	UE_LOG(LogRGBCamera, Log, TEXT("Capturing and sending data"));
	
	// Render target data from the SceneCaptureComponent2D
	FTextureRenderTargetResource* RenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
	TArray<FColor> Bitmap;
	bool bReadSuccess = RenderTargetResource->ReadPixels(Bitmap);

	if (!bReadSuccess)
    {
        UE_LOG(LogRGBCamera, Error, TEXT("Failed to read pixels from RenderTarget2D"));
        return;
    }

	ImageQueue->Enqueue(Bitmap);
	// SendRGB data to the server
	// SendRGBData(Bitmap);
}

void ARGBCamera::InitializeSocket()
{
	if (Socket) return;

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogRGBCamera, Error, TEXT("Failed to get socket subsystem"));
		return;
	}
	
	Socket = FTcpSocketBuilder(TEXT("RGBD Data Socket"));

	if (!Socket)
	{
		UE_LOG(LogRGBCamera, Error, TEXT("Failed to create socket"));
		return;
	}

	FString Host = TEXT("127.0.0.1");
	const int32 Port = 8080;

	// Resolve the host name
	TSharedPtr<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	bool bIsValid;
	Addr->SetIp(*Host, bIsValid);
	if (!bIsValid)
	{
		UE_LOG(LogRGBCamera, Error, TEXT("Invalid IP address"));
		return;
	}
	
	Addr->SetPort(Port);

	// Connect to the server
	bSocketInitialized = Socket->Connect(*Addr);
	if (!bSocketInitialized)
	{
		UE_LOG(LogRGBCamera, Error, TEXT("Failed to connect to server %s:%d"), *Host, Port);
		return;
	}

	UE_LOG(LogRGBCamera, Log, TEXT("Successfully connected to server %s:%d"), *Host, Port);
}

void ARGBCamera::SendRGBData(const TArray<FColor>& ImageData)
{
	if (!Socket) return;

	TArray<uint8> ByteArray;
	for (const FColor& Pixel : ImageData) {
		ByteArray.Add(Pixel.R);
		ByteArray.Add(Pixel.G);
		ByteArray.Add(Pixel.B);
	}

	int32 BytesSent = 0;
	Socket->Send(ByteArray.GetData(), ByteArray.Num(), BytesSent);
}

void ARGBCamera::SendDepthData(const TArray<float>& DepthData)
{
}

