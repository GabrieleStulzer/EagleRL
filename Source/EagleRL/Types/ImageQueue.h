#pragma once

class EAGLERL_API FImageQueue
{
public:
	FImageQueue();
	~FImageQueue();

	/** Enqueue image data */
	void Enqueue(const TArray<FColor>& ImageData);

	/** Dequeue image data. Returns false if the queue is empty */
	bool Dequeue(TArray<FColor>& OutImageData);

	/** Check if the queue is empty */
	bool IsEmpty();

	/** Get the event to wait on for new images */
	FEvent* GetEvent();

private:
	TQueue<TArray<FColor>> ImageQueue;
	FCriticalSection QueueLock;
	FEvent* NewImageEvent;
};

