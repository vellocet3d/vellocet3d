#pragma once

#include <vector>

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "vel/CustomTriangleMeshData.h"

namespace vel 
{
	class CustomTriangleMesh : public btTriangleMesh
	{
	private:
		std::vector<CustomTriangleMeshData> m_customMeshData;

	public:
		void addTriangle(const btVector3& vertex0, const btVector3& vertex1, const btVector3& vertex2, const CustomTriangleMeshData& data);
		CustomTriangleMeshData& getCustomVertexData(int triangleIndex, int vertexIndex);
	};
}