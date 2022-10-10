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

		//int				attemptedVertexWeightAdditions = 0; // for debugging

		bool operator==(const Vertex &) const;
	};      
}