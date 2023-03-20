#include <iostream>
#include "vel/CustomTriangleMesh.h"

namespace vel 
{
	void CustomTriangleMesh::addTriangle(const btVector3& vertex0, const btVector3& vertex1, const btVector3& vertex2, 
		const CustomTriangleMeshData& vertex0CD, const CustomTriangleMeshData& vertex1CD, const CustomTriangleMeshData& vertex2CD)
	{
		btTriangleMesh::addTriangle(vertex0, vertex1, vertex2);
		this->m_customMeshData.push_back(vertex0CD);
		this->m_customMeshData.push_back(vertex1CD);
		this->m_customMeshData.push_back(vertex2CD);
	}

	CustomTriangleMeshData& CustomTriangleMesh::getCustomVertexData(int triangleIndex, int vertexIndex)
	{
		// Calculate the index of the custom data associated with the specified vertex
		//int dataIndex = triangleIndex * 3 + vertexIndex;
		int dataIndex = vertexIndex;

		//std::cout << dataIndex << std::endl; 24 but max is 21 or 20 if 0 indexed

		// Return the custom data associated with the vertex
		return this->m_customMeshData.at(dataIndex);
	}

	size_t CustomTriangleMesh::getCustomMeshDataSize()
	{
		return this->m_customMeshData.size();
	}
}
