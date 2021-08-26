#include <iostream>
#include <algorithm>

#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/gtx/compatibility.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/string_cast.hpp"

#include "vel/Log.h"
#include "vel/Armature.h"



namespace vel
{
	Armature::Armature(std::string name) :
		name(name),
		runTime(0.0),
		previousRunTime(0.0)
	{}

	const std::vector<std::pair<std::string, Animation*>>&	Armature::getAnimations()
	{
		return this->animations;
	}

	glm::vec3 Armature::calcTranslation(const float& time, size_t currentKeyIndex, Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->positionKeyTimes[nextKeyIndex] - channel->positionKeyTimes[currentKeyIndex];
		float factor = ((time - channel->positionKeyTimes[currentKeyIndex]) / deltaTime);

		return channel->positionKeyValues[currentKeyIndex] + factor * (channel->positionKeyValues[nextKeyIndex] - channel->positionKeyValues[currentKeyIndex]);
	}

	glm::quat Armature::calcRotation(const float& time, size_t currentKeyIndex, Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->rotationKeyTimes[nextKeyIndex] - channel->rotationKeyTimes[currentKeyIndex];
		float factor = ((time - channel->rotationKeyTimes[currentKeyIndex]) / deltaTime);

		return glm::normalize(glm::slerp(channel->rotationKeyValues[currentKeyIndex], channel->rotationKeyValues[nextKeyIndex], factor));
	}

	glm::vec3 Armature::calcScale(const float& time, size_t currentKeyIndex, Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->scalingKeyTimes[nextKeyIndex] - channel->scalingKeyTimes[currentKeyIndex];
		float factor = ((time - channel->scalingKeyTimes[currentKeyIndex]) / deltaTime);

		return channel->scalingKeyValues[currentKeyIndex] + factor * (channel->scalingKeyValues[nextKeyIndex] - channel->scalingKeyValues[currentKeyIndex]);
	}

	void Armature::updateBone(size_t index, glm::mat4 parentMatrix)
	{
		// we assume there are always at least 2 keys of animation data

		// get current bone and update it's previous TRS values with current
		auto& bone = this->bones[index];
		bone.previousTranslation = bone.translation;
		bone.previousRotation = bone.rotation;
		bone.previousScale = bone.scale;


		// get vector of key indexes where vector index is the index of the activeAnimation and value is the keyIndex
		std::vector<TRS> activeAnimationsTRS;
		for (auto& aa : this->activeAnimations)
		{
			//std::cout << aa.animation->name << "\n";
			//std::cout << bone.name << "\n";
			auto channel = &aa.animation->channels[bone.name];
			auto it = std::upper_bound(channel->positionKeyTimes.begin(), channel->positionKeyTimes.end(), aa.animationKeyTime);
			auto tmpKey = (size_t)(it - channel->positionKeyTimes.begin());
			size_t currentKeyIndex = !(tmpKey == channel->positionKeyTimes.size()) ? (tmpKey - 1) : (tmpKey - 2);

			TRS trs;
			trs.translation = this->calcTranslation(aa.animationKeyTime, currentKeyIndex, channel);
			trs.rotation = this->calcRotation(aa.animationKeyTime, currentKeyIndex, channel);
			trs.scale = this->calcScale(aa.animationKeyTime, currentKeyIndex, channel);

			activeAnimationsTRS.push_back(trs);
		}

		// if activeAnimations has a size greater than 1, then do interpolation
		if (activeAnimationsTRS.size() > 1)
		{
			TRS lerpTRS;
			for (size_t i = 0; i < activeAnimationsTRS.size() - 1; i++)
			{
				// no need to lerp last element
				if (i + 1 < activeAnimationsTRS.size())
				{
					// if this is the first element, prime lerpTRS using by lerping with first element
					if (i == 0)
					{
						lerpTRS.translation = glm::lerp(activeAnimationsTRS[i].translation, activeAnimationsTRS[i + 1].translation, this->activeAnimations[i + 1].blendPercentage);
						lerpTRS.rotation = glm::slerp(activeAnimationsTRS[i].rotation, activeAnimationsTRS[i + 1].rotation, this->activeAnimations[i + 1].blendPercentage);
						lerpTRS.scale = glm::lerp(activeAnimationsTRS[i].scale, activeAnimationsTRS[i + 1].scale, this->activeAnimations[i + 1].blendPercentage);
					}
					// otherwise lerp using lerpTRS
					else
					{
						lerpTRS.translation = glm::lerp(lerpTRS.translation, activeAnimationsTRS[i + 1].translation, this->activeAnimations[i + 1].blendPercentage);
						lerpTRS.rotation = glm::slerp(lerpTRS.rotation, activeAnimationsTRS[i + 1].rotation, this->activeAnimations[i + 1].blendPercentage);
						lerpTRS.scale = glm::lerp(lerpTRS.scale, activeAnimationsTRS[i + 1].scale, this->activeAnimations[i + 1].blendPercentage);
					}
				}
			}

			bone.matrix = glm::mat4(1.0f);
			bone.matrix = glm::translate(bone.matrix, lerpTRS.translation);
			bone.matrix = bone.matrix * glm::toMat4(lerpTRS.rotation);
			bone.matrix = glm::scale(bone.matrix, lerpTRS.scale);

		}
		// otherwise, generate the bone matrix using the single animation
		else
		{
			bone.matrix = glm::mat4(1.0f);
			bone.matrix = glm::translate(bone.matrix, activeAnimationsTRS[0].translation);
			bone.matrix = bone.matrix * glm::toMat4(activeAnimationsTRS[0].rotation);
			bone.matrix = glm::scale(bone.matrix, activeAnimationsTRS[0].scale);
		}

		bone.matrix = parentMatrix * bone.matrix;
		glm::decompose(bone.matrix, bone.scale, bone.rotation, bone.translation, bone.skew, bone.perspective);
		bone.rotation = glm::conjugate(bone.rotation);
	}

	void Armature::updateAnimation(double runTime, std::optional<glm::mat4> parentMatrix)
	{
		this->previousRunTime = this->runTime;
		this->runTime = runTime;
		auto stepTime = this->runTime - this->previousRunTime;

		//if (this->activeAnimations.size() == 0)
		//	return;

		// get most recent active animation
		auto& activeAnimation = this->activeAnimations.back();

		// current active animation is either set to repeat, or not repeat but has not completed it's first cycle
		if (activeAnimation.repeat || (!activeAnimation.repeat && activeAnimation.currentAnimationCycle == 0))
		{

			if (activeAnimation.blendTime > 0.0)
				activeAnimation.blendPercentage = (float)(activeAnimation.animationTime / (activeAnimation.blendTime / 1000.0));
			else
				activeAnimation.blendPercentage = 1.0f;

			// if blendPercentage is greater than or equal to 1.0f, then we have completed the blending phase and this animation
			// can continue to play without interpolating between previous animations, therefore we clear all previous animations
			// from this->activeAnimations
			if (activeAnimation.blendPercentage >= 1.0f)
			{
				for (size_t i = 0; i < this->activeAnimations.size() - 1; i++)
					this->activeAnimations.pop_front();

				activeAnimation = this->activeAnimations.back();
			}

			//std::cout << this->activeAnimations.size() << "\n";

			activeAnimation.lastAnimationKeyTime = activeAnimation.animationKeyTime;
			activeAnimation.animationKeyTime = (float)fmod(activeAnimation.animationTime * activeAnimation.animation->tps, activeAnimation.animation->duration);

			//std::cout << activeAnimation.animationKeyTime << "\n";

			if (activeAnimation.animationKeyTime < activeAnimation.lastAnimationKeyTime)
				activeAnimation.currentAnimationCycle++;


			if (activeAnimation.currentAnimationCycle == 1 && !activeAnimation.repeat)
			{
				activeAnimation.animationKeyTime = activeAnimation.lastAnimationKeyTime;

				for (size_t i = 0; i < this->bones.size(); i++)
				{
					auto& bone = this->bones[i];
					bone.previousTranslation = bone.translation;
					bone.previousRotation = bone.rotation;
					bone.previousScale = bone.scale;
				}
			}
			else
			{
				for (size_t i = 0; i < this->bones.size(); i++)
				{
					if (i == 0)
						this->updateBone(0, glm::mat4(1.0f));
					else
						this->updateBone(i, this->bones[this->bones[i].parent].matrix);
				}

				activeAnimation.animationTime += stepTime;
			}
		}
	}



	void Armature::playAnimation(std::string animationName, bool repeat, int blendTime)
	{
		ActiveAnimation a;
		a.animation = this->getAnimation(animationName);
		a.animationName = animationName;
		a.blendTime = (double)blendTime;
		a.animationTime = 0.0;
		a.animationKeyTime = 0.0f;
		a.lastAnimationKeyTime = 0.0f;
		a.currentAnimationCycle = 0;
		a.blendPercentage = 0.0f;
		a.repeat = repeat;

		this->activeAnimations.push_back(a);
	}

	float Armature::getCurrentAnimationKeyTime()
	{
		return this->activeAnimations.back().animationKeyTime;
	}

	unsigned int Armature::getCurrentAnimationCycle()
	{
		if (this->activeAnimations.size() > 0)
			return this->activeAnimations.back().currentAnimationCycle;
		else
			return 0;
	}

	std::string Armature::getCurrentAnimationName()
	{
		if (this->activeAnimations.size() > 0)
			return this->activeAnimations.back().animationName;
		else
			return "";
	}

	const std::vector<std::pair<std::string, Animation*>>& Armature::getAnimations() const
	{
		return this->animations;
	}

	const std::string& Armature::getName() const
	{
		return this->name;
	}

	void Armature::addAnimation(std::string name, Animation* anim)
	{
		this->animations.push_back(std::pair<std::string, Animation*>(name, anim));
	}

	void Armature::addBone(ArmatureBone b)
	{
		this->bones.push_back(b);
	}

	ArmatureBone& Armature::getRootBone()
	{
		return this->bones[0];
	}

	std::vector<ArmatureBone>& Armature::getBones()
	{
		return this->bones;
	}

	const std::vector<ArmatureBone>& Armature::getBones() const
	{
		return this->bones;
	}

	ArmatureBone* Armature::getBone(std::string boneName)
	{
		for (auto& b : this->bones)
			if (b.name == boneName)
				return &b;

		return nullptr;
	}

	ArmatureBone& Armature::getBone(size_t index)
	{
		return this->bones.at(index);
	}

	Animation* Armature::getAnimation(std::string animationName)
	{
		for (auto& p : this->animations)
			if (p.first == animationName)
				return p.second;
            
#ifdef DEBUG_LOG
    std::string msg = std::string("Armature::getAnimation(): Attempting to get animation pointer of non-existing animation name: ") + animationName;
    Log::crash(msg);
#endif
    
    }

	size_t Armature::getBoneIndex(std::string boneName)
	{
		for (size_t i = 0; i < this->bones.size(); i++)
			if (this->bones.at(i).name == boneName)
				return i;
        
#ifdef DEBUG_LOG
    std::string msg = std::string("Armature::getBoneIndex(): Attempting to get index of non-existing bone name: ") + boneName;
    Log::crash(msg);
#endif
        
	}

}