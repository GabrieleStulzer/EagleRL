#pragma once


#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtils.h"
#include <Interfaces/IPv4/IPv4Address.h>

#include "MovieSceneSequenceID.h"
#include "Common/TcpSocketBuilder.h"
#include "Types/ImageQueue.h"

class EAGLERL_API FImageSenderRunnable : public FRunnable
{
public:
	FImageSenderRunnable(FImageQueue*, FSocket*);
	virtual ~FImageSenderRunnable();

	// FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	FImageQueue* ImageQueue;
	FSocket* Socket;

	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	// bool CaptureImage(TArray<FColor>&) const;
	void SendImage(const TArray<FColor>& ImageData);
};
