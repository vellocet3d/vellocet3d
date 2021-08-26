#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/AssetManager.h"
#include "vel/Mesh.h"
#include "vel/Stage.h"
#include "vel/Shader.h"
#include "vel/Transform.h"
#include "vel/Armature.h"
#include "vel/VertexBoneData.h"
#include "vel/Animation.h"

#include "vel/AssetTrackers.h"


namespace vel
{

	class AssetLoaderV2
	{
	private:
		Assimp::Importer					aiImporter;
		const aiScene*						aiScene;
		AssetManager*						assetManager;
		
		std::vector<MeshTracker*>			meshTrackers;		
		ArmatureTracker*					armatureTracker; // currently only support one armature per .fbx file
		bool								existingArmature;
		

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
		aiMatrix4x4							glmToAssImpMat4(glm::mat4 mat);
		bool								nodeHasBeenProcessed(aiNode* in);

	public:
		AssetLoaderV2(AssetManager* currentScene, std::string assetFile);
		void								load();
		
		std::pair<std::vector<MeshTracker*>, ArmatureTracker*> getTrackers();

	};


}