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
#include "vel/scene/animation/Animation.h"




namespace vel
{

	class AssetLoaderV2
	{
	private:
		Assimp::Importer					aiImporter;
		const aiScene*						aiScene;
		Scene*								currentScene;

		std::string							currentAssetFile;
		std::optional<size_t>				currentMeshIndex;
		Armature*							currentArmature;
		std::vector<aiNode*>				processedNodes;
		std::vector<VertexBoneData>			currentMeshBones;
		glm::mat4							currentGlobalInverseMatrix;
		void								processAnimations();
		void								processArmatureNode(aiNode* node);
		void								processNode(aiNode* node);
		void								processMesh(aiMesh* aiMesh);
		bool								isRootArmatureNode(aiNode* node);
		glm::mat4							aiMatrix4x4ToGlm(const aiMatrix4x4 &from);
		std::optional<Animation*>			getExistingAnimation(std::string animationName);
		aiMatrix4x4							glmToAssImpMat4(glm::mat4 mat);
		bool								nodeHasBeenProcessed(aiNode* in);

	public:
		AssetLoaderV2(Scene* currentScene, std::string assetFile);
		void								load();

	};


}