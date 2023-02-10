#include <iostream>

#include "glm/gtx/compatibility.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include "vel/Transform.h"



namespace vel
{

    Transform::Transform() :
        translation(glm::vec3(0.0f, 0.0f, 0.0f)),
        scale(glm::vec3(1.0f, 1.0f, 1.0f)),
		rotation(glm::quat())
    {}

    Transform::Transform(glm::vec3 t, glm::quat r, glm::vec3 s) :
        translation(t), 
        rotation(r), 
        scale(s){}   

    void Transform::print()
    {
        std::cout << "T:(" << this->translation.x << "," << this->translation.y << "," << this->translation.z << ")\n";
        std::cout << "R:(" << this->rotation.x << "," << this->rotation.y << "," << this->rotation.z << "," << this->rotation.w << ")\n";
        std::cout << "S:(" << this->scale.x << "," << this->scale.y << "," << this->scale.z << ")\n";
        std::cout << "---------------------------------\n";
    }

	glm::vec3 Transform::getRotationEulers()
	{
		auto tmp = glm::eulerAngles(this->rotation);
		return glm::vec3((tmp.x * 180.0f / 3.141592653589793f), (tmp.y * 180.0f / 3.141592653589793f), (tmp.z * 180.0f / 3.141592653589793f));
	}

    void Transform::setTranslation(glm::vec3 translation) 
    {
        this->translation = translation;
    }

	void Transform::setRotation(glm::quat rotation)
	{
		this->rotation = rotation;
	}

    void Transform::setRotation(float angle, glm::vec3 axis)
    {
		this->rotation = glm::angleAxis(glm::radians(angle), axis);
    }

	void Transform::appendRotation(float angle, glm::vec3 axis)
	{
		this->rotation *= glm::angleAxis(glm::radians(angle), axis);
	}

    void Transform::setScale(glm::vec3 scale)
    {
        this->scale = scale;
    }

    const glm::vec3& Transform::getTranslation() const
    {
        return this->translation;
    }

	const glm::quat& Transform::getRotation() const
    {
        return this->rotation;
    }

	const glm::vec3& Transform::getScale() const
    {
        return this->scale;
    }

    const glm::mat4 Transform::getMatrix() const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, this->translation);
		m = m * glm::toMat4(this->rotation);
        m = glm::scale(m, this->scale);
        return m;
    }

	glm::vec3 Transform::interpolateTranslations(const Transform& previousTransform, const Transform& currentTransform, float alpha)
	{
		return glm::lerp(previousTransform.getTranslation(), currentTransform.getTranslation(), alpha);
	}

	glm::quat Transform::interpolateRotations(const Transform& previousTransform, const Transform& currentTransform, float alpha)
	{
		return glm::slerp(previousTransform.getRotation(), currentTransform.getRotation(), alpha);
	}

	glm::vec3 Transform::interpolateScales(const Transform& previousTransform, const Transform& currentTransform, float alpha)
	{
		return glm::lerp(previousTransform.getScale(), currentTransform.getScale(), alpha);
	}

	glm::mat4 Transform::interpolateTransforms(const Transform& previousTransform, const Transform& currentTransform, float alpha)
	{
		Transform t;
		t.setTranslation(glm::lerp(previousTransform.getTranslation(), currentTransform.getTranslation(), alpha));
		t.setRotation(glm::slerp(previousTransform.getRotation(), currentTransform.getRotation(), alpha));
		t.setScale(glm::lerp(previousTransform.getScale(), currentTransform.getScale(), alpha));
		return t.getMatrix();
	}


}