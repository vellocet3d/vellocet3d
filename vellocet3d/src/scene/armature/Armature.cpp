#include <iostream>
#include <algorithm>

#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/gtx/compatibility.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/string_cast.hpp"

#include "vel/App.h"
#include "vel/scene/armature/Armature.h"


namespace vel::scene::armature
{
	Armature::Armature(std::string name) :
		name(name),
		runTime(0.0),
		previousRunTime(0.0),
		stepTime(0.0)
	{}

	glm::vec3 Armature::calcTranslation(const float& time, size_t currentKeyIndex, vel::scene::animation::Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->positionKeyTimes[nextKeyIndex] - channel->positionKeyTimes[currentKeyIndex];
		float factor = ((time - channel->positionKeyTimes[currentKeyIndex]) / deltaTime);

		return channel->positionKeyValues[currentKeyIndex] + factor * (channel->positionKeyValues[nextKeyIndex] - channel->positionKeyValues[currentKeyIndex]);
	}

	glm::quat Armature::calcRotation(const float& time, size_t currentKeyIndex, vel::scene::animation::Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->rotationKeyTimes[nextKeyIndex] - channel->rotationKeyTimes[currentKeyIndex];
		float factor = ((time - channel->rotationKeyTimes[currentKeyIndex]) / deltaTime);

		return glm::normalize(glm::slerp(channel->rotationKeyValues[currentKeyIndex], channel->rotationKeyValues[nextKeyIndex], factor));
	}

	glm::vec3 Armature::calcScale(const float& time, size_t currentKeyIndex, vel::scene::animation::Channel* channel)
	{
		size_t nextKeyIndex = currentKeyIndex + 1;

		float deltaTime = channel->scalingKeyTimes[nextKeyIndex] - channel->scalingKeyTimes[currentKeyIndex];
		float factor = ((time - channel->scalingKeyTimes[currentKeyIndex]) / deltaTime);

		return channel->scalingKeyValues[currentKeyIndex] + factor * (channel->scalingKeyValues[nextKeyIndex] - channel->scalingKeyValues[currentKeyIndex]);
	}

	void Armature::updateBone(size_t index, glm::mat4 parentMatrix)
	{
		// we assume there are always at least 2 keys of animation data

		// get current bone and update it's previous SRT values with current
		auto& bone = this->bones[index];
		bone.previousTranslation = bone.translation;
		bone.previousRotation = bone.rotation;
		bone.previousScale = bone.scale;


		// get vector of key indexes where vector index is the index of the activeAnimation and value is the keyIndex
		std::vector<std::pair<size_t, vel::scene::animation::Channel*>> keyIndexAndChannels;
		for (auto& aa : this->activeAnimations)
		{
			auto channel = &aa.animation->channels[bone.name];
			auto it = std::upper_bound(channel->positionKeyTimes.begin(), channel->positionKeyTimes.end(), aa.animationKeyTime);
			auto tmpKey = (size_t)(it - channel->positionKeyTimes.begin());
			size_t currentKeyIndex = !(tmpKey == channel->positionKeyTimes.size()) ? (tmpKey - 1) : (tmpKey - 2);

			keyIndexAndChannels.push_back(std::pair<size_t, vel::scene::animation::Channel*>(currentKeyIndex, channel));
		}

		// if activeAnimations has a size of 1, then do no interpolation
		if (this->activeAnimations.size() > 1)
		{



		}
		else
		{
			bone.matrix = glm::mat4(1.0f);
			bone.matrix = glm::translate(bone.matrix, this->calcTranslation(this->activeAnimations[0].animationKeyTime, keyIndexAndChannels[0].first, keyIndexAndChannels[0].second));
			bone.matrix = bone.matrix * glm::toMat4(this->calcRotation(this->activeAnimations[0].animationKeyTime, keyIndexAndChannels[0].first, keyIndexAndChannels[0].second));
			bone.matrix = glm::scale(bone.matrix, this->calcScale(this->activeAnimations[0].animationKeyTime, keyIndexAndChannels[0].first, keyIndexAndChannels[0].second));
			bone.matrix = parentMatrix * bone.matrix;

			glm::decompose(bone.matrix, bone.scale, bone.rotation, bone.translation, bone.skew, bone.perspective);
			bone.rotation = glm::conjugate(bone.rotation);
		}

		





		/////////////////////////////////////////////////////////////////////


		// get key index for current animation
		auto& channel = this->currentAnimation->channels[bone.name];
		auto it = std::upper_bound(channel.positionKeyTimes.begin(), channel.positionKeyTimes.end(), this->currentAnimationTime);
		auto tmpKey = (size_t)(it - channel.positionKeyTimes.begin());
		size_t currentKeyIndex = !(tmpKey == channel.positionKeyTimes.size()) ? (tmpKey - 1) : (tmpKey - 2);

		if (this->transitionAnimation == nullptr)
		{
			
			// generate a new matrix and SRT values for this bone based solely on the current animation time (single animation no blending)
			bone.matrix = glm::mat4(1.0f);
			bone.matrix = glm::translate(bone.matrix, this->calcTranslation(this->currentAnimationTime, currentKeyIndex, channel));
			bone.matrix = bone.matrix * glm::toMat4(this->calcRotation(this->currentAnimationTime, currentKeyIndex, channel));
			bone.matrix = glm::scale(bone.matrix, this->calcScale(this->currentAnimationTime, currentKeyIndex, channel));
			bone.matrix = parentMatrix * bone.matrix;

			glm::decompose(bone.matrix, bone.scale, bone.rotation, bone.translation, bone.skew, bone.perspective);
			bone.rotation = glm::conjugate(bone.rotation);
		}
		else
		{
			
			// generate a new matrix and SRT values for this bone by lerping between the animation data of the last key of the currentAnimation 
			// (currentAnimation stops moving forward at time when transitionAnimation is added ("Frozen Transition")) and the current key of the
			// transitionAnimation, using this->blendPercentage as interpolation alpha
			auto& transitionChannel = this->transitionAnimation->channels[bone.name];
			auto it2 = std::upper_bound(transitionChannel.positionKeyTimes.begin(), transitionChannel.positionKeyTimes.end(), this->transitionAnimationTime);
			auto tmpKey2 = (size_t)(it2 - transitionChannel.positionKeyTimes.begin());
			size_t transitionKeyIndex = !(tmpKey2 == transitionChannel.positionKeyTimes.size()) ? (tmpKey2 - 1) : (tmpKey2 - 2);

			bone.matrix = glm::mat4(1.0f);
			bone.matrix = glm::translate(bone.matrix, glm::lerp(this->calcTranslation(this->currentAnimationTime, currentKeyIndex, channel), this->calcTranslation(this->transitionAnimationTime, transitionKeyIndex, transitionChannel), this->blendPercentage));
			bone.matrix = bone.matrix * glm::toMat4(glm::slerp(this->calcRotation(this->currentAnimationTime, currentKeyIndex, channel), this->calcRotation(this->transitionAnimationTime, transitionKeyIndex, transitionChannel), this->blendPercentage));
			bone.matrix = glm::scale(bone.matrix, glm::lerp(this->calcScale(this->currentAnimationTime, currentKeyIndex, channel), this->calcScale(this->transitionAnimationTime, transitionKeyIndex, transitionChannel), this->blendPercentage));
			bone.matrix = parentMatrix * bone.matrix;

			glm::decompose(bone.matrix, bone.scale, bone.rotation, bone.translation, bone.skew, bone.perspective);
			bone.rotation = glm::conjugate(bone.rotation);
		}
		

	}

	void Armature::updateAnimation(double runTime, std::optional<glm::mat4> parentMatrix)
	{
		this->previousRunTime = this->runTime;
		this->runTime = runTime;
		this->stepTime = this->runTime - this->previousRunTime;


		// get most most recent active animation
		auto activeAnimation = this->activeAnimations.back();

		if (activeAnimation.blendTime > 0.0)
		{
			activeAnimation.blendPercentage = (float)(activeAnimation.animationTime / (activeAnimation.blendTime / 1000.0));
		}
		else
		{
			activeAnimation.blendPercentage = 1.0f;
		}

		// if blendPercentage is greater than or equal to 1.0f, then we have completed the blending phase and this animation
		// can continue to play without interpolating between previous animations, therefore we clear all previous animations
		// from this->activeAnimations and reset activeAnimation value (since removal will shift memory)
		if (activeAnimation.blendPercentage >= 1.0f)
		{
			for (size_t i = 0; i < this->activeAnimations.size() - 1; i++)
			{
				this->activeAnimations.pop_front();
			}

			activeAnimation = this->activeAnimations.back();
		}

		activeAnimation.lastAnimationKeyTime = activeAnimation.animationKeyTime;
		activeAnimation.animationKeyTime = (float)fmod(activeAnimation.animationTime, activeAnimation.animation->duration);

		if (activeAnimation.animationKeyTime < activeAnimation.lastAnimationKeyTime)
		{
			activeAnimation.currentAnimationCycle++;
		}

		for (size_t i = 0; i < this->bones.size(); i++)
		{
			if (i == 0)
			{
				this->updateBone(0, glm::mat4(1.0f));
			}
			else
			{
				this->updateBone(i, this->bones[this->bones[i].parent].matrix);
			}
		}
		
		activeAnimation.animationTime += this->stepTime;
		/////////////////////////

		/*
		if ((!this->repeatCurrentAnimation && this->currentAnimationCycle == 0) || this->repeatCurrentAnimation)
		{
			if (this->transitionAnimation != nullptr)
			{
				this->rollingBlendTime += this->stepTime;
				this->blendPercentage = (float)(this->rollingBlendTime / (this->blendTime / 1000.0));

				double transitionAnimationTimeInTicks = this->runTime * this->transitionAnimation->tps;
				this->transitionAnimationTime = (float)fmod(transitionAnimationTimeInTicks, this->transitionAnimation->duration);

				if (this->blendPercentage >= 1.0f)
				{
					this->currentAnimation = this->transitionAnimation;
					this->currentAnimationName = this->transitionAnimationName;

					this->transitionAnimationName = "";
					this->transitionAnimation = nullptr;

					this->lastAnimationTime = -1.0;
					this->currentAnimationCycle = 0;
					this->blendPercentage = 0.0f;
					this->rollingBlendTime = 0.0;
				}
			}

			// set current animation time, unless we have a transitionAnimation, in which case currentAnimationTime needs
			// to stay what it was
			if (this->transitionAnimation == nullptr)
			{
				double currentAnimationTimeInTicks = this->runTime * this->currentAnimation->tps;
				this->currentAnimationTime = (float)fmod(currentAnimationTimeInTicks, this->currentAnimation->duration);

				if (this->lastAnimationTime == -1.0)
				{
					this->lastAnimationTime = this->currentAnimationTime;
				}

				// if animationTime is less than the last animation time, we know that a new cycle has begun
				if (this->currentAnimationTime < this->lastAnimationTime)
				{
					this->currentAnimationCycle++;
				}
			}

			if ((!this->repeatCurrentAnimation && this->currentAnimationCycle == 0) || this->repeatCurrentAnimation)
			{
				for (size_t i = 0; i < this->bones.size(); i++)
				{
					if (i == 0)
					{
						this->updateBone(0, glm::mat4(1.0f));
					}
					else
					{
						this->updateBone(i, this->bones[this->bones[i].parent].matrix);
					}
				}
			}

			// set last transform properties of each bone to current if this is a non-repeatable animation and we have completed the first cycle
			if (!this->repeatCurrentAnimation && this->currentAnimationCycle == 1)
			{
				for (size_t i = 0; i < this->bones.size(); i++)
				{
					auto& bone = this->bones[i];
					bone.previousTranslation = bone.translation;
					bone.previousRotation = bone.rotation;
					bone.previousScale = bone.scale;
				}
			}
		}
		*/
		
	}



	void Armature::playAnimation(std::string animationName, int blendTime)
	{
		ActiveAnimation a;
		a.animation = &App::get().getScene()->getAnimation(this->getAnimationIndex(animationName));
		a.animationName = animationName;
		a.blendTime = (double)blendTime;
		a.animationTime = 0.0;
		a.lastAnimationKeyTime = 0.0f;
		a.currentAnimationCycle = 0.0f;
		a.blendPercentage = 0.0f;

		this->activeAnimations.push_back(a);
	}

	const std::vector<std::pair<std::string, size_t>>& Armature::getAnimations() const
	{
		return this->animations;
	}

	const std::string& Armature::getName() const
	{
		return this->name;
	}

	void Armature::addAnimation(std::string name, size_t index)
	{
		this->animations.push_back(std::pair<std::string, size_t>(name, index));
	}

	void Armature::addBone(Bone b)
	{
		this->bones.push_back(b);
	}

	Bone& Armature::getRootBone()
	{
		return this->bones[0];
	}

	std::vector<Bone>& Armature::getBones()
	{
		return this->bones;
	}

	const std::vector<Bone>& Armature::getBones() const
	{
		return this->bones;
	}

	Bone* Armature::getBone(std::string boneName)
	{
		for (auto& b : this->bones)
		{
			if (b.name == boneName)
				return &b;
		}
		return nullptr;
	}

	Bone& Armature::getBone(size_t index)
	{
		return this->bones.at(index);
	}

	

	size_t Armature::getAnimationIndex(std::string animationName)
	{
		for (auto& p : this->animations)
		{
			if (p.first == animationName)
			{
				return p.second;
			}
		}
		std::cout << "Trying to get index of non-existing animationName:" << animationName << "\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	size_t Armature::getBoneIndex(std::string boneName)
	{
		for (size_t i = 0; i < this->bones.size(); i++)
		{
			if (this->bones.at(i).name == boneName)
			{
				return i;
			}
		}
		std::cout << "Trying to get index of non-existing boneName:" << boneName << "\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

}