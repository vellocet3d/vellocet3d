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
#include "vel/scene/Shader.h"
#include "vel/scene/stage/Transform.h"
#include "vel/scene/armature/Armature.h"
#include "vel/scene/mesh/VertexBoneData.h"

using namespace vel::scene::stage;


namespace vel::scene
{

	class AssetLoader
	{
	private:
		Assimp::Importer					aiImporter;
		const aiScene*						aiScene;
		bool								headless;
		Stage*								currentStage;
		std::string							currentAssetFile;
		std::string							currentAssetDirectory;
		Transform							currentTransform;
		std::optional<size_t>				currentMeshIndex;
		std::optional<size_t>				currentMeshTextureIndex;
		std::optional<armature::Armature>	currentArmature;
		bool								currentIsDynamic;
		std::vector<std::string>			processedNodes;
		std::vector<mesh::VertexBoneData>	currentMeshBones;
		glm::mat4							currentGlobalInverseMatrix;
		void								processAnimations();
		void								processArmatureNode(aiNode* node);
		void								processActorNode(aiNode* node);
		void								processTransformable(aiNode* node);
		void								processMesh(aiNode* node);
		bool								isRootArmatureNode(aiNode* node);
		std::string							generateUniqueActorName(std::string name);
		glm::mat4							aiMatrix4x4ToGlm(const aiMatrix4x4 &from);
		std::optional<size_t>				getExistingAnimationIndex(std::string animationName);

	public:
																	AssetLoader(Stage* stage, std::string assetFile, bool dynamic);
		std::optional<std::function<int(std::string actorName)>>	findShaderId;
		void														loadActors();

	};
	

}