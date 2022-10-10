#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/Shader.h"
#include "vel/Camera.h"
#include "vel/Vertex.h"
#include "vel/Texture.h"
#include "vel/GpuMesh.h"
#include "vel/MeshBone.h"


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

		

		//Texture*							texture;
		//bool								isOpaque;


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

		//void								setTexture(Texture* t);
		//Texture*							getTexture();
		//void								finalize();

		//bool								getIsOpaque();

		std::vector<unsigned int>			textureIds;

	};
    
}