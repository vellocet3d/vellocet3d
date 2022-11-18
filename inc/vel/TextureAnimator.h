#pragma once

namespace vel
{

	class TextureAnimator
	{
	private:
		float				frameCount;
		float				framesPerSecond;
		unsigned int		currentFrame;
		float				currentCycleTime;
		unsigned int		currentCycle;
		bool				paused;
		unsigned int		pauseAfterCycles;

	public:
		TextureAnimator(float frameCount, float fps);

		unsigned int		update(float frameTime);

		void				setFramesPerSecond(float fps);
		void				setPauseAfterCycles(unsigned int c);
		void				setPaused(bool p);

		unsigned int		getCurrentFrame();

	};
}