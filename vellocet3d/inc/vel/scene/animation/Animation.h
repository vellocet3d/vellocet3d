#pragma once

#include <unordered_map>
#include <string>

#include "vel/scene/animation/Channel.h"


namespace vel::scene::animation
{
	struct Animation
	{
		std::string				name; //global name including armature prefix
		double					duration;
		double					tps;
		std::unordered_map<std::string, Channel> channels;
	};
	
}