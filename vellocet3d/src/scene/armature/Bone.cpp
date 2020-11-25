
#include "glm/gtx/compatibility.hpp"

//#include "vel/App.h"
#include "vel/scene/stage/Transform.h"
#include "vel/scene/armature/Bone.h"

namespace vel::scene::armature
{
	//glm::mat4 Bone::getRenderMatrix(float alpha)
	//{
	//	vel::scene::stage::Transform t;
	//	if (App::get().getFrameTime() >= App::get().getLogicTime())
	//	{
	//		t.setTranslation(this->translation);
	//		t.setRotation(this->rotation);
	//		t.setScale(this->scale);
	//	}
	//	else
	//	{
	//		t.setTranslation(glm::lerp(this->previousTranslation, this->translation, alpha));
	//		t.setRotation(glm::slerp(this->previousRotation, this->rotation, alpha));
	//		t.setScale(glm::lerp(this->previousScale, this->scale, alpha));
	//	}
	//	
	//	return t.getMatrix();
	//}

	glm::mat4 Bone::getRenderMatrix(float alpha)
	{
		vel::scene::stage::Transform t;
		t.setTranslation(glm::lerp(this->previousTranslation, this->translation, alpha));
		t.setRotation(glm::slerp(this->previousRotation, this->rotation, alpha));
		t.setScale(glm::lerp(this->previousScale, this->scale, alpha));
		return t.getMatrix();
	}
}