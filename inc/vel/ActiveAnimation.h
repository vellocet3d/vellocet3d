#pragma once

#include <string>

#include "vel/Animation.h"


namespace vel
{
	struct ActiveAnimation
	{
		Animation*							animation; // pointer to animation held by current scene
		std::string							animationName; // name relative to armature, no armature prefix
		double								blendTime; // in ms
		bool								repeat; // whether or not this animation should loop

		double								animationTime; // total time animation has been running
		float								animationKeyTime; // what time value we need to use to find key index
		float								lastAnimationKeyTime; // last key time of animation used to determine how many cycles have passed
		unsigned int						currentAnimationCycle; // how many times has this animation completely cycled through all key data
		float								blendPercentage; // the interpolation alpha for blending between active animations
	};

}