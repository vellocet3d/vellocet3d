#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/stage/Stage.h"
#include "vel/Shader.h"
#include "vel/scene/stage/Transform.h"
#include "vel/scene/armature/Armature.h"
#include "vel/scene/mesh/VertexBoneData.h"
#include "vel/scene/Scene.h"




namespace vel
{

	class AssetLoaderV2
	{
	private:
		Assimp::Importer	aiImporter;
		const aiScene*		aiScene;
		Scene*				currentScene;
		std::string			currentAssetFile;

		void				processNodeForMeshes(aiNode* node);

		glm::mat4			currentGlobalInverseMatrix;
		glm::mat4			aiMatrix4x4ToGlm(const aiMatrix4x4 &from);

		bool				isRootArmatureNode(aiNode* node);
		void				processMesh(aiMesh* node);


	public:
		AssetLoaderV2(Scene* currentScene, std::string assetFile);

		void loadMeshes();

	};


}