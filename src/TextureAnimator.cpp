#include <iostream>

#include "vel\TextureAnimator.h"

namespace vel
{

	TextureAnimator::TextureAnimator(float frameCount, float fps) :
		frameCount(frameCount),
		framesPerSecond(fps),
		currentFrame(0),
		currentCycleTime(0.0f),
		currentCycle(1),
		paused(false),
		pauseAfterCycles(0),
		reverse(false)
	{}

	void TextureAnimator::setReverse(bool r)
	{
		this->reverse = r;
	}

	bool TextureAnimator::getIsPlayingReversed()
	{
		return this->reverse;
	}

	unsigned int TextureAnimator::getCurrentFrame()
	{
		return this->currentFrame;
	}

	void TextureAnimator::setPaused(bool p)
	{
		this->paused = p;
	}

	bool TextureAnimator::getPaused()
	{
		return this->paused;
	}

	void TextureAnimator::setFramesPerSecond(float fps)
	{
		this->framesPerSecond = fps;
	}

	void TextureAnimator::setPauseAfterCycles(unsigned int c)
	{
		this->pauseAfterCycles = c;
	}

	//unsigned int TextureAnimator::update(float frameTime)
	//{
	//	if (this->paused)
	//		return this->currentFrame;

	//	float numberOfFramesToPlayPerSecond = 1.0f / this->framesPerSecond;

	//	unsigned int nextFrame = (unsigned int)(this->currentCycleTime / numberOfFramesToPlayPerSecond);

	//	if (nextFrame == this->frameCount) // 0 based
	//	{
	//		if (this->pauseAfterCycles == this->currentCycle)
	//		{
	//			this->paused = true;
	//			this->currentCycle = 1;
	//		}
	//		else
	//		{
	//			this->currentCycle += 1;
	//		}
	//		
	//		this->currentCycleTime = 0.0f;

	//		return this->currentFrame;
	//	}

	//	this->currentCycleTime += frameTime;

	//	this->currentFrame = nextFrame;

	//	return this->currentFrame;
	//}

	unsigned int TextureAnimator::update(float frameTime)
	{
		if (this->paused)
			return this->currentFrame;

		float numberOfFramesToPlayPerSecond = 1.0f / this->framesPerSecond;

		unsigned int nextFrame = (unsigned int)(this->currentCycleTime / numberOfFramesToPlayPerSecond);
		unsigned int nextFrameReversed = (unsigned int)(this->frameCount - nextFrame);
		nextFrameReversed = nextFrameReversed - 1; // zero based index

		//std::cout << "nextFrame: " << nextFrame << "\n"
		//	<< "nextFrameReversed: " << nextFrameReversed << std::endl;

		if (nextFrame == this->frameCount) // 0 based
		{
			if (this->pauseAfterCycles == this->currentCycle)
			{
				this->paused = true;
				this->currentCycle = 1;
			}
			else
			{
				this->currentCycle += 1;
			}

			this->currentCycleTime = 0.0f;

			return this->currentFrame;
		}

		this->currentCycleTime += frameTime;

		if (!this->reverse)
			this->currentFrame = nextFrame;
		else
			this->currentFrame = nextFrameReversed;

		//std::cout << "this->currentFrame: " << this->currentFrame << std::endl;

		return this->currentFrame;
	}



}