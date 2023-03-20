#include "vel/CustomTriangleMesh.h"

namespace vel 
{
	void CustomTriangleMesh::addTriangle(const btVector3& vertex0, const btVector3& vertex1, const btVector3& vertex2, const CustomTriangleMeshData& data)
	{
		btTriangleMesh::addTriangle(vertex0, vertex1, vertex2);
		this->m_customMeshData.push_back(data);
	}

	CustomTriangleMeshData& CustomTriangleMesh::getCustomVertexData(int triangleIndex, int vertexIndex)
	{
		// Calculate the index of the custom data associated with the specified vertex
		int dataIndex = triangleIndex * 3 + vertexIndex;

		// Return the custom data associated with the vertex
		return this->m_customMeshData.at(dataIndex);
	}
}
