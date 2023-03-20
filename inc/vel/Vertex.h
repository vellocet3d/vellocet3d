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
		glm::vec2		lightmapCoordinates;
		unsigned int	textureId;
		VertexBoneData	weights;

		bool operator==(const Vertex &) const;

		static glm::vec2 calculateLightmapUVForWorldPoint(
			const glm::vec3& worldPosA, const glm::vec3& worldPosB, const glm::vec3& worldPosC, 
			const glm::vec2& uvCoordsA, const glm::vec2& uvCoordsB, const glm::vec2& uvCoordsC,
			const glm::vec3& worldIntersectionPoint);
	};      
}