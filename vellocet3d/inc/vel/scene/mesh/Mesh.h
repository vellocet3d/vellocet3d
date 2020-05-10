#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/scene/Shader.h"
#include "vel/scene/stage/Camera.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/mesh/Texture.h"
#include "vel/scene/mesh/Renderable.h"
#include "vel/scene/mesh/Bone.h"


namespace vel::scene::mesh
{
	class Mesh
	{

	private:
		std::string                         name;
		std::vector<Vertex>					vertices;
		std::vector<unsigned int>           indices;
		std::vector<Bone>					bones;
		std::optional<size_t>               meshRenderableIndex;
		glm::mat4							globalInverseMatrix;


	public:
											Mesh(std::string name);
		void								addVertexWeight(unsigned int vertexIndex, unsigned int boneIndex, float weight);
		void                                setMeshRenderableIndex(size_t index);
		void								setVertices(std::vector<Vertex>& vertices);
		void								setIndices(std::vector<unsigned int>& indices);
		void								setBones(std::vector<Bone>& bones);
		const std::optional<size_t>&        getMeshRenderableIndex() const;
		const std::string                   getName() const;
		const std::vector<Vertex>&			getVertices() const;
		const std::vector<unsigned int>&	getIndices() const;
		const bool                          isRenderable() const;
		const bool                          hasBones() const;
		void								setGlobalInverseMatrix(glm::mat4 gim);
		glm::mat4							getGlobalInverseMatrix();
		Bone&								getBone(size_t index);
		Bone*								getBone(std::string boneName);
		const std::vector<Bone>&			getBones() const;

	};
    
}