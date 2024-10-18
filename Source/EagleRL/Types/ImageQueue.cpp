#include "ImageQueue.h"

FImageQueue::FImageQueue()
{
	// Create an event that is not initially signaled
	NewImageEvent = FPlatformProcess::GetSynchEventFromPool(false);
}

FImageQueue::~FImageQueue()
{
	if (NewImageEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(NewImageEvent);
		NewImageEvent = nullptr;
	}
}

void FImageQueue::Enqueue(const TArray<FColor>& ImageData)
{
	{
		FScopeLock Lock(&QueueLock);
		ImageQueue.Enqueue(ImageData);
	}

	// Signal that a new image is available
	NewImageEvent->Trigger();
}

bool FImageQueue::Dequeue(TArray<FColor>& OutImageData)
{
	FScopeLock Lock(&QueueLock);
	if (ImageQueue.Dequeue(OutImageData))
	{
		return true;
	}
	return false;
}

bool FImageQueue::IsEmpty()
{
	FScopeLock Lock(&QueueLock);
	return ImageQueue.IsEmpty();
}

FEvent* FImageQueue::GetEvent()
{
	return NewImageEvent;
}
