#pragma once

#include <string>
#include <optional>


namespace vel
{
	struct Texture;

	struct Material
	{
		std::string name;
		glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Texture* diffuse = nullptr;
		Texture* albedo = nullptr;
		Texture* normal = nullptr;
		Texture* metallic = nullptr;
		Texture* roughness = nullptr;
		Texture* ao = nullptr; // ambient occlusion
		Texture* height = nullptr;
		float heightScale = 0.0f;
		bool hasAlphaChannel = false;
	};
}