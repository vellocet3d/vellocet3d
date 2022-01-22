
#include "glm/gtx/compatibility.hpp"


#include "vel/Transform.h"
#include "vel/ArmatureBone.h"

namespace vel
{
	glm::mat4 ArmatureBone::getRenderMatrixInterpolated(float alpha)
	{
		Transform t;
		t.setTranslation(glm::lerp(this->previousTranslation, this->translation, alpha));
		t.setRotation(glm::slerp(this->previousRotation, this->rotation, alpha));
		t.setScale(glm::lerp(this->previousScale, this->scale, alpha));
		return t.getMatrix();
	}

	glm::mat4 ArmatureBone::getRenderMatrix()
	{
		Transform t;
		t.setTranslation(this->translation);
		t.setRotation(this->rotation);
		t.setScale(this->scale);
		return t.getMatrix();
	}

}