#include "FImageSenderRunnable.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Sockets.h"
#include "Engine/World.h"

FImageSenderRunnable::FImageSenderRunnable(FImageQueue* ImageQueue, FSocket* Socket)
	: ImageQueue(ImageQueue), Socket(Socket), Thread(nullptr)
{
	Thread = FRunnableThread::Create(this, TEXT("FImageSenderRunnable"), 0, TPri_BelowNormal);
}

FImageSenderRunnable::~FImageSenderRunnable()
{
	if(Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}
}

bool FImageSenderRunnable::Init()
{
	return ImageQueue != nullptr && Socket != nullptr;
}

uint32 FImageSenderRunnable::Run()
{
	FDateTime LastCaptureTime = FDateTime::Now();
	
	while(StopTaskCounter.GetValue() == 0)
	{
		FEvent* NewImageEvent = ImageQueue->GetEvent();
		if (NewImageEvent)
		{
			uint32 WaitResult = NewImageEvent->Wait();
			if (WaitResult == 0)
			{
				UE_LOG(
					LogTemp,
					Error,
					TEXT("Failed to wait on new image event")
				);
			}

			TArray<FColor> ImageData;
			while (ImageQueue->Dequeue(ImageData))
			{
				SendImage(ImageData);
			}
		}

		FPlatformProcess::Sleep(0.001f);
	}
	
	return 0;
}

void FImageSenderRunnable::Stop()
{
	StopTaskCounter.Increment();
	if(ImageQueue && ImageQueue->GetEvent())
	{
		ImageQueue->GetEvent()->Trigger();
	}
}

void FImageSenderRunnable::SendImage(const TArray<FColor>& ImageData)
{
	if(!Socket)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Socket is not initialized")
		);
		return;
	}
	
	if (!Socket) return;

	TArray<uint8> ByteArray;
	for (const FColor& Pixel : ImageData) {
		ByteArray.Add(Pixel.R);
		ByteArray.Add(Pixel.G);
		ByteArray.Add(Pixel.B);
	}

	int32 BytesSent = 0;
	Socket->Send(ByteArray.GetData(), ByteArray.Num(), BytesSent);

	// int32 ImageSize = ImageData.Num() * sizeof(FColor);
	// int32 BytesSent = 0;
	// int32 BytesRemaining = ImageSize;
	// uint8* Data = (uint8*)ImageData.GetData();
	//
	// while(BytesRemaining > 0)
	// {
	// 	int32 BytesSentThisIteration = 0;
	// 	bool bSendSuccess = Socket->Send(Data, BytesRemaining, BytesSentThisIteration);
	// 	if (!bSendSuccess)
	// 	{
	// 		UE_LOG(
	// 			LogTemp,
	// 			Error,
	// 			TEXT("Failed to send image data")
	// 		);
	// 		break;
	// 	}
	//
	// 	BytesSent += BytesSentThisIteration;
	// 	BytesRemaining -= BytesSentThisIteration;
	// 	Data += BytesSentThisIteration;
	// }
}
