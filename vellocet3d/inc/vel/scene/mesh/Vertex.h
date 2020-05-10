#pragma once

#include "glm/glm.hpp"

#include "vel/scene/mesh/VertexBoneData.h"


namespace vel::scene::mesh
{
	struct Vertex
	{
		glm::vec3		position;
		glm::vec3		normal;
		glm::vec2		textureCoordinates;
		VertexBoneData	weights;

		bool operator==(const Vertex &) const;
	};      
}