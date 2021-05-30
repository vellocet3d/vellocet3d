#pragma once

#include <string>
#include <vector>
#include <deque>
#include <optional>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "vel/scene/animation/Animation.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/armature/ArmatureBone.h"
#include "vel/scene/stage/Transform.h"
#include "vel/scene/armature/ActiveAnimation.h"




namespace vel
{
	class Scene;

	class Armature
	{
	private:
		std::string											name;
		Scene*												parentScene;
		std::vector<ArmatureBone>							bones;
		std::vector<std::pair<std::string, size_t>>			animations;
		std::deque<ActiveAnimation>							activeAnimations;
		double												runTime;
		double												previousRunTime;		
		void												updateBone(size_t index, glm::mat4 parentMatrix);
		glm::vec3											calcTranslation(const float& time, size_t currentKeyIndex, Channel* channel);
		glm::quat											calcRotation(const float& time, size_t currentKeyIndex, Channel* channel);
		glm::vec3											calcScale(const float& time, size_t currentKeyIndex, Channel* channel);


	public:
															Armature(std::string name, Scene* parentScene);
		void												addBone(ArmatureBone b);
		void												addAnimation(std::string name, size_t index);
		ArmatureBone&										getRootBone();
		std::vector<ArmatureBone>&							getBones();
		const std::vector<ArmatureBone>&					getBones() const;
		ArmatureBone&										getBone(size_t index);
		ArmatureBone*										getBone(std::string boneName);
		const std::string&									getName() const;
		const std::vector<std::pair<std::string, size_t>>&	getAnimations() const;
		size_t												getBoneIndex(std::string boneName);
		void												updateAnimation(double runTime, std::optional<glm::mat4> parentMatrix);
		size_t												getAnimationIndex(std::string animationName);

		std::string											getCurrentAnimationName();
		unsigned int										getCurrentAnimationCycle();
		float												getCurrentAnimationKeyTime();

		void												playAnimation(std::string animationName, bool repeat = true, int blendTime = 0);


	};
}