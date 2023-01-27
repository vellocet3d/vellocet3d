#include <iostream>


#include "vel/Mesh.h"


namespace vel
{

    Mesh::Mesh(std::string name) :
        name(name)
    {}

	AABB& Mesh::getAABB()
	{
		if (this->aabb.has_value())
			return this->aabb.value();

		this->aabb = AABB(this->getVertices());

		return this->aabb.value();
	}

	const std::vector<MeshBone>& Mesh::getBones() const
	{
		return this->bones;
	}

	MeshBone* Mesh::getBone(std::string boneName)
	{
		for (auto& b : this->bones)
		{
			if (b.name == boneName)
				return &b;
		}
		return nullptr;
	}

	MeshBone& Mesh::getBone(size_t index)
	{
		return this->bones.at(index);
	}

	glm::mat4 Mesh::getGlobalInverseMatrix()
	{
		return this->globalInverseMatrix;
	}

	void Mesh::setGlobalInverseMatrix(glm::mat4 gim)
	{
		this->globalInverseMatrix = gim;
	}

	const bool Mesh::hasBones() const
	{
		if (this->bones.size() > 0)
		{
			return true;
		}
		return false;
	}

	void Mesh::addVertexWeight(unsigned int vertexIndex, unsigned int boneIndex, float weight)
	{
		// wtff???...oh ok, this just looks weird, but it's looping through each index in the raw array
		for (unsigned int i = 0; i < (sizeof(this->vertices[vertexIndex].weights.ids) / sizeof(this->vertices[vertexIndex].weights.ids[0])); i++)
		{
			if (this->vertices[vertexIndex].weights.weights[i] == 0.0f)
			{
				this->vertices[vertexIndex].weights.ids[i] = boneIndex;
				this->vertices[vertexIndex].weights.weights[i] = weight;
				return;
			}
		}		
	}

	void Mesh::setVertices(std::vector<Vertex>& vertices)
	{
		this->vertices = vertices;
	}

	void Mesh::setIndices(std::vector<unsigned int>& indices)
	{
		this->indices = indices;
	}

	void Mesh::setBones(std::vector<MeshBone>& bones)
	{
		this->bones = bones;
	}

    void Mesh::setGpuMesh(GpuMesh gm)
    {
		this->gpuMesh = gm;
    }

    std::vector<Vertex>& Mesh::getVertices()
    {
        return this->vertices;
    }

    std::vector<unsigned int>& Mesh::getIndices()
    {
        return this->indices;
    }

    const std::string Mesh::getName() const
    {
        return this->name;
    }

    const bool Mesh::isRenderable() const
    {
        return this->gpuMesh.has_value();
    }

    std::optional<GpuMesh>& Mesh::getGpuMesh()
    {
        return this->gpuMesh;
    }

}