#pragma once

#include "glm/glm.hpp"

#include "vel/VertexBoneData.h"


namespace vel
{
	struct Vertex
	{
		glm::vec3		position;
		glm::vec3		normal;
		glm::vec2		textureCoordinates;
		unsigned int	textureId;
		VertexBoneData	weights;

		bool operator==(const Vertex &) const;
	};      
}