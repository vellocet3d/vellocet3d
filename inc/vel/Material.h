#pragma once

#include <string>
#include <optional>


namespace vel
{
	struct Texture;

	struct Material
	{
		std::string name;
		Texture* albedo = nullptr;
		Texture* normal = nullptr;
		Texture* metallic = nullptr;
		Texture* roughness = nullptr;
		Texture* ao = nullptr; // ambient occlusion
		bool hasAlphaChannel = false;
	};
}