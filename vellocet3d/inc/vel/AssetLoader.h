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

	class AssetLoader
	{
	private:
		Assimp::Importer					aiImporter;
		const aiScene*						aiScene;
		Scene*								stageParentScene;
		Stage*								currentStage;
		std::string							currentAssetFile;
		std::string							currentAssetDirectory;
		Transform							currentTransform;
		std::optional<size_t>				currentMeshIndex;
		std::optional<size_t>				currentMeshTextureIndex;
		Armature*							currentArmature;
		bool								currentIsDynamic;
		std::vector<aiNode*>				processedNodes;
		std::vector<VertexBoneData>			currentMeshBones;
		glm::mat4							currentGlobalInverseMatrix;
		glm::mat4							blenderRotationCorrectionMatrix;
		glm::mat4							blenderRotationCorrectionPerVertexMatrix;
		void								processAnimations();
		void								processArmatureNode(aiNode* node);
		void								processActorNode(aiNode* node);
		void								processTransformable(aiNode* node);
		void								processMesh(aiNode* node);
		bool								isRootArmatureNode(aiNode* node);
		std::string							generateUniqueActorName(std::string name);
		glm::mat4							aiMatrix4x4ToGlm(const aiMatrix4x4 &from);
		std::optional<size_t>				getExistingAnimationIndex(std::string animationName);

		std::pair<std::string, std::string> get_texture_path_and_file(std::string in);

		aiMatrix4x4							glmToAssImpMat4(glm::mat4 mat);

		std::vector<size_t>					addedActorIndexes;

		bool								nodeHasBeenProcessed(aiNode* in);

	public:
																	AssetLoader(Scene* stageParentScene, Stage* stage, std::string assetFile, bool dynamic);
		std::optional<std::function<int(std::string actorName)>>	findShaderId;
		std::vector<size_t>											loadActors();

	};
	

}