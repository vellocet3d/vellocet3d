#include <iostream>

#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"


#include "vel/App.h"
#include "vel/AssetLoaderV2.h"
#include "vel/Vertex.h"
#include "vel/functions.h"
#include "vel/Log.h"


namespace vel
{
	AssetLoaderV2::AssetLoaderV2(AssetManager* assetManager, std::string assetFile) :
		assetManager(assetManager),
		currentAssetFile(assetFile),
		armatureTracker(nullptr),
		currentArmature(nullptr),
		existingArmature(false),
		currentMeshTextureId(0)
	{
		this->impScene = this->aiImporter.ReadFile(this->currentAssetFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

		if (!this->impScene || !this->impScene->mRootNode)
		{
			std::string errorMessage = this->aiImporter.GetErrorString();
			std::cout << "ERROR::ASSIMP::" << errorMessage << "\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		}
	}

	void AssetLoaderV2::load()
	{
		this->processNode(this->impScene->mRootNode);
	}

	std::pair<std::vector<MeshTracker*>, ArmatureTracker*> AssetLoaderV2::getTrackers()
	{
		return std::pair<std::vector<MeshTracker*>, ArmatureTracker*>(this->meshTrackers, this->armatureTracker);
	}

	void AssetLoaderV2::processAnimations()
	{
		for (unsigned int i = 0; i < this->impScene->mNumAnimations; i++)
		{
			std::string animationName = this->impScene->mAnimations[i]->mName.C_Str();

			// create a new animation
			auto a = Animation();
			a.name = this->impScene->mAnimations[i]->mName.C_Str();
			a.duration = this->impScene->mAnimations[i]->mDuration;
			a.tps = this->impScene->mAnimations[i]->mTicksPerSecond;
			// ok apparently this was reverted at some point...sweet, will leave below here just incase 
			//a.tps = this->impScene->mAnimations[i]->mTicksPerSecond * 33.3333333; // account for the weird assimp/fbx "update" that multiplies duration by 33.3333333

			// add all channels to animation

			for (unsigned int j = 0; j < this->impScene->mAnimations[i]->mNumChannels; j++)
			{
				auto c = Channel();

				// positions
				for (unsigned int k = 0; k < this->impScene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++)
				{
					auto position = this->impScene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue;
					auto time = (float)this->impScene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
					c.positionKeyTimes.push_back(time);
					c.positionKeyValues.push_back(glm::vec3(position.x, position.y, position.z));
				}

				// rotations
				for (unsigned int k = 0; k < this->impScene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++)
				{
					auto rotation = this->impScene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue;
					auto time = (float)this->impScene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
					c.rotationKeyTimes.push_back(time);
					c.rotationKeyValues.push_back(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
				}

				// scalings
				for (unsigned int k = 0; k < this->impScene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++)
				{
					auto scale = this->impScene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue;
					auto time = (float)this->impScene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
					c.scalingKeyTimes.push_back(time);
					c.scalingKeyValues.push_back(glm::vec3(scale.x, scale.y, scale.z));
				}

				// add channel to animation
				a.channels[this->impScene->mAnimations[i]->mChannels[j]->mNodeName.C_Str()] = c;
			}

			// add animation to scene's animations container, retrieving index
			auto aPtr = this->assetManager->addAnimation(a);

			// obtain this animation name relative to the armature
			auto name = explode_string(a.name, '|')[1];

			// add this animation name/index to the armature's animations vector
			this->currentArmature->addAnimation(name, aPtr);
		}
	}

	void AssetLoaderV2::processArmatureNode(aiNode* node)
	{
		std::string nodeName = node->mName.C_Str();

		if (nodeName != "RootNode" && !string_contains("_end", nodeName))
		{
			
			std::string boneName = nodeName;
			std::string nodeParentName = node->mParent->mName.C_Str();

			if (nodeParentName == "RootNode")
			{
				// check if armature with this name already exists within the asset manager
				auto armTracker = this->assetManager->getArmatureTracker(boneName);
				if(armTracker != nullptr)
				{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Armature, bypass reload: " + nodeName);
#endif
					armTracker->usageCount++;
					this->armatureTracker = armTracker;
					this->existingArmature = true;
					this->currentArmature = armTracker->ptr;
				}
				else
				{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading new Armature: " + nodeName);
#endif	
					this->armatureTracker = this->assetManager->addArmature(Armature(boneName));
					this->currentArmature = this->armatureTracker->ptr;
				}
			}
				
			if(!this->existingArmature)
			{
				ArmatureBone bone;
				bone.name = boneName;
				bone.parentName = nodeParentName == "RootNode" ? boneName : node->mParent->mName.C_Str();
				bone.parentArmature = this->currentArmature;

				this->currentArmature->addBone(bone);
			}
		}

		this->processedNodes.push_back(node);

		// Do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			this->processArmatureNode(node->mChildren[i]);
	}

	bool AssetLoaderV2::isRootArmatureNode(aiNode* node)
	{
		// if this node has children, and no mesh, assume that it is an armature
		// (this is very naive, but should work for the time being so we can move forward
		// with skeletal animation implementation. This method can be revised in the future
		// to be more accurate)
		std::string nodeName = node->mParent->mName.C_Str();
		if (nodeName == "RootNode" && node->mNumChildren > 0 && node->mNumMeshes == 0)
			return true;
		else
			return false;

	}

	void AssetLoaderV2::processNode(aiNode* node)
	{
		std::string nodeName = node->mName.C_Str();
		//std::cout << "processNode:" << nodeName << std::endl;


		if (nodeName == "RootNode")
			this->currentGlobalInverseMatrix = glm::inverse(this->aiMatrix4x4ToGlm(node->mTransformation));


		// If this is not the RootNode and this node has not already been processed
		if (nodeName != "RootNode" && !this->nodeHasBeenProcessed(node))
		{
			if (this->isRootArmatureNode(node))
			{
				this->processArmatureNode(node);
				
				if(!this->existingArmature)
				{
					this->processAnimations();

					// Obtain parent indexes for each bone using their
					// boneNames (done so that these indexes can be used at runtime instead
					// of loops and string comparisons)
					for (auto& b : this->currentArmature->getBones())
						b.parent = this->currentArmature->getBoneIndex(b.parentName);
				}
			}
			else
			{
				// use node name for mesh name

				// check if a mesh already exists with this node name and don't load if there is
				auto meshTracker = this->assetManager->getMeshTracker(nodeName);
				if (meshTracker != nullptr)
				{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Mesh, bypass reload: " + nodeName);
#endif
					meshTracker->usageCount++;
					this->meshTrackers.push_back(meshTracker);
					return;
				}

				// otherwise loop through all meshes for this node, creating a Mesh for each one
#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading new Mesh: " + nodeName);
#endif

				// add each generated mesh to a vector
				
				/* // this comment block was working code, commented to experiment
				std::vector<Mesh> subMeshes = {};

				auto meshCount = node->mNumMeshes;

				while (meshCount > 0)
				{
					std::string subMeshName = nodeName + "_" + std::to_string(meshCount - 1);
					
					subMeshes.push_back(Mesh(subMeshName));

					this->processMesh(this->impScene->mMeshes[node->mMeshes[(meshCount - 1)]], subMeshes.back());

					this->currentMeshTextureId++;
					meshCount--;
				}
				*/

				// after we've converted all assimp meshes into our mesh format, loop through all meshes
				// and join them into a single mesh object
				//Mesh finalMesh = Mesh(nodeName);
				//unsigned int indiceOffset = 0;

				//for (auto& m : subMeshes)
				//{
				//	//std::vector<Vertex>

				//	// bone indexes are screwing us right now, have to figure out how we'll handle these
				//	// when merging multiple meshes

				//}

				// trying something different, lets see if we can just create one single mesh from data that we generate
				// from all of the aiMeshes
				

				std::vector<Vertex> meshVertices = {};
				std::vector<unsigned int> meshIndices = {};
				std::vector<MeshBone> meshBones = {};

				auto meshCount = node->mNumMeshes;

				while (meshCount > 0)
				{
					this->processMesh(this->impScene->mMeshes[node->mMeshes[(meshCount - 1)]], meshVertices, meshIndices, meshBones);

					this->currentMeshTextureId++;
					meshCount--;
				}

				Mesh finalMesh = Mesh(nodeName);
				finalMesh.setVertices(meshVertices);
				finalMesh.setIndices(meshIndices);
				finalMesh.setBones(meshBones);
				finalMesh.setGlobalInverseMatrix(this->currentGlobalInverseMatrix);
				
				this->assetManager->addMesh(finalMesh);






				//mesh.setGlobalInverseMatrix(this->currentGlobalInverseMatrix);

				// then send to asset manager
				//this->assetManager->addMesh();
			}
		}

		// Do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			this->processNode(node->mChildren[i]);
	}

	void AssetLoaderV2::processMesh(aiMesh* aiMesh,
		std::vector<Vertex>& meshVertices,
		std::vector<unsigned int>& meshIndices,
		std::vector<MeshBone>& meshBones)
	{
		unsigned int indiceOffset = meshVertices.size();

		// walk through each of the aimesh's vertices
		for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			// position
			vector.x = aiMesh->mVertices[i].x;
			vector.y = aiMesh->mVertices[i].y;
			vector.z = aiMesh->mVertices[i].z;
			vertex.position = vector;

			// normal
			vector.x = aiMesh->mNormals[i].x;
			vector.y = aiMesh->mNormals[i].y;
			vector.z = aiMesh->mNormals[i].z;
			vertex.normal = vector;

			// texture coordinates
			if (aiMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = aiMesh->mTextureCoords[0][i].x;
				vec.y = aiMesh->mTextureCoords[0][i].y;
				vertex.textureCoordinates = vec;

				// assign this mesh's textureid to each vertex
				vertex.textureId = this->currentMeshTextureId;
			}
			else
			{
				vertex.textureCoordinates = glm::vec2(0.0f, 0.0f);
			}

			meshVertices.push_back(vertex);
		}


		// now walk through each of the mesh's faces (a face is a mesh's triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
		{
			aiFace face = aiMesh->mFaces[i];

			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				meshIndices.push_back(face.mIndices[j] + indiceOffset);
		}


		// if mesh has bones, process bones
		if (aiMesh->HasBones())
		{
			for (unsigned int i = 0; i < aiMesh->mNumBones; i++)
			{
				if (aiMesh->mBones[i]->mNumWeights == 0)
					continue;

				std::string aBoneName = aiMesh->mBones[i]->mName.C_Str();
				unsigned int boneIndex;
				bool foundExistingBone = false;
				for (unsigned int k = 0; k < meshBones.size(); k++)
				{
					if (meshBones.at(k).name == aBoneName)
					{
						boneIndex = k;
						foundExistingBone = true;
						break;
					}
				}

				if (!foundExistingBone)
				{
					auto b = MeshBone();
					b.name = aiMesh->mBones[i]->mName.C_Str();
					b.offsetMatrix = this->aiMatrix4x4ToGlm(aiMesh->mBones[i]->mOffsetMatrix);
					meshBones.push_back(b);
					boneIndex = meshBones.size() - 1;
				}

				for (unsigned int j = 0; j < aiMesh->mBones[i]->mNumWeights; j++)
				{
					unsigned int tmpVertInd = aiMesh->mBones[i]->mWeights[j].mVertexId + indiceOffset;
					for (unsigned int m = 0; m < (sizeof(meshVertices.at(tmpVertInd).weights.ids) / sizeof(meshVertices.at(tmpVertInd).weights.ids[0])); m++)
					{
						if (meshVertices.at(tmpVertInd).weights.weights[m] == 0.0f)
						{
							meshVertices.at(tmpVertInd).weights.ids[m] = boneIndex;
							meshVertices.at(tmpVertInd).weights.weights[m] = aiMesh->mBones[i]->mWeights[j].mWeight;
						}
					}
				}


				//for (unsigned int j = 0; j < aiMesh->mBones[i]->mNumWeights; j++)
				//	mesh.addVertexWeight(aiMesh->mBones[i]->mWeights[j].mVertexId, boneIndex, aiMesh->mBones[i]->mWeights[j].mWeight);
			}
		}
		//mesh.setBones(bones);

		//mesh.setGlobalInverseMatrix(this->currentGlobalInverseMatrix);
	}

	glm::mat4 AssetLoaderV2::aiMatrix4x4ToGlm(const aiMatrix4x4 &from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	aiMatrix4x4 AssetLoaderV2::glmToAssImpMat4(glm::mat4 mat)
	{
		const float* glmMat = (const float*)glm::value_ptr(mat);

		return aiMatrix4x4(
			glmMat[0], glmMat[1], glmMat[2], glmMat[3],
			glmMat[4], glmMat[5], glmMat[6], glmMat[7],
			glmMat[8], glmMat[9], glmMat[10], glmMat[11],
			glmMat[12], glmMat[13], glmMat[14], glmMat[15]
		);
	}

	bool AssetLoaderV2::nodeHasBeenProcessed(aiNode* in)
	{
		for (auto& pn : this->processedNodes)
			if (pn == in)
				return true;

		return false;
	}

}