#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/Shader.h"
#include "vel/scene/stage/Camera.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/material/Texture.h"
#include "vel/scene/mesh/GpuMesh.h"
#include "vel/scene/mesh/MeshBone.h"


namespace vel
{
	class Mesh
	{

	private:
		std::string                         name;
		std::vector<Vertex>					vertices;
		std::vector<unsigned int>           indices;
		std::vector<MeshBone>				bones;
		std::optional<GpuMesh>              gpuMesh;
		glm::mat4							globalInverseMatrix;


	public:
											Mesh(std::string name);
		void								addVertexWeight(unsigned int vertexIndex, unsigned int boneIndex, float weight);
		void                                setGpuMesh(GpuMesh gm);
		void								setVertices(std::vector<Vertex>& vertices);
		void								setIndices(std::vector<unsigned int>& indices);
		void								setBones(std::vector<MeshBone>& bones);
		const std::optional<GpuMesh>&       getGpuMesh() const;
		const std::string                   getName() const;
		const std::vector<Vertex>&			getVertices() const;
		const std::vector<unsigned int>&	getIndices() const;
		const bool                          isRenderable() const;
		const bool                          hasBones() const;
		void								setGlobalInverseMatrix(glm::mat4 gim);
		glm::mat4							getGlobalInverseMatrix();
		MeshBone&							getBone(size_t index);
		MeshBone*							getBone(std::string boneName);
		const std::vector<MeshBone>&		getBones() const;

	};
    
}