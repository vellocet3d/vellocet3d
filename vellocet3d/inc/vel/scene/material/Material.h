#pragma once

#include <string>
#include <optional>


namespace vel
{
	struct Material
	{
		std::string name;
		std::optional<size_t> albedo;
		std::optional<size_t> normal;
		std::optional<size_t> metalness;
		std::optional<size_t> roughness;
		std::optional<size_t> ao; // ambient occlusion
		bool hasAlphaChannel = false;
	};
}