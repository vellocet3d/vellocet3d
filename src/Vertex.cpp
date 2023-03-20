#include "vel/Vertex.h"

namespace vel
{
    bool Vertex::operator==(const Vertex &v) const
    {
        return position == v.position && normal == v.normal && textureCoordinates == v.textureCoordinates;
    }

	glm::vec2 Vertex::calculateLightmapUVForWorldPoint(const glm::vec3& worldPosA, const glm::vec3& worldPosB, const glm::vec3& worldPosC,
		const glm::vec2& uvCoordsA, const glm::vec2& uvCoordsB, const glm::vec2& uvCoordsC,
		const glm::vec3& worldIntersectionPoint)
	{
		// Calculate the barycentric coordinates of P
		glm::vec3 v0 = worldPosB - worldPosA;
		glm::vec3 v1 = worldPosC - worldPosA;
		glm::vec3 v2 = worldIntersectionPoint - worldPosA;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		// Interpolate the UV coordinates using the barycentric coordinates
		glm::vec2 UV;
		UV.x = u * uvCoordsA.x + v * uvCoordsB.x + w * uvCoordsC.x;
		UV.y = u * uvCoordsA.y + v * uvCoordsB.y + w * uvCoordsC.y;

		return UV;
	}
}