#include <iostream>


#include "vel/Mesh.h"


namespace vel
{

    Mesh::Mesh(std::string name) :
        name(name),
		texture(nullptr),
		isOpaque(true)
    {}

	bool Mesh::getIsOpaque()
	{
		return this->isOpaque;
	}

	void Mesh::finalize()
	{
		for (auto& v : this->vertices)
			v.textureId = this->texture->dsaIdIndex;

		if (this->texture->alphaChannel)
			this->isOpaque = false;
	}

	void Mesh::setTexture(Texture* t)
	{
		this->texture = t;
	}

	Texture* Mesh::getTexture()
	{
		return this->texture;
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
		//this->vertices[vertexIndex].attemptedVertexWeightAdditions++;
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

    const std::vector<Vertex>& Mesh::getVertices() const
    {
        return this->vertices;
    }

    const std::vector<unsigned int>& Mesh::getIndices() const
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

    const std::optional<GpuMesh>& Mesh::getGpuMesh() const
    {
        return this->gpuMesh;
    }

}