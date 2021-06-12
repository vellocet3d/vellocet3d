
#include "glm/gtx/compatibility.hpp"


#include "vel/scene/stage/Transform.h"
#include "vel/assets/armature/ArmatureBone.h"

namespace vel
{
	glm::mat4 ArmatureBone::getRenderMatrix(float alpha)
	{
		Transform t;
		t.setTranslation(glm::lerp(this->previousTranslation, this->translation, alpha));
		t.setRotation(glm::slerp(this->previousRotation, this->rotation, alpha));
		t.setScale(glm::lerp(this->previousScale, this->scale, alpha));
		return t.getMatrix();
	}
}