#pragma once

#include <unordered_map>
#include <string>

#include "vel/Channel.h"


namespace vel
{
	struct Animation
	{
		std::string				name; //global name including armature prefix
		double					duration;
		double					tps;
		std::unordered_map<std::string, Channel> channels;
	};
	
}