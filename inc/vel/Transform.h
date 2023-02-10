#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"


namespace vel
{
	class Transform
	{
	private:
		glm::vec3			translation;
		glm::quat			rotation;
		glm::vec3			scale;

	public:
							Transform();
							Transform(glm::vec3 t, glm::quat r, glm::vec3 s);
		void				setTranslation(glm::vec3 translation);
		void				setRotation(float angle, glm::vec3 axis);
		void				setRotation(glm::quat rotation);
		void				appendRotation(float angle, glm::vec3 axis);
		void				setScale(glm::vec3 scale);
		const glm::vec3&	getTranslation() const;
		const glm::quat&	getRotation() const;
		const glm::vec3&	getScale() const;
		const glm::mat4		getMatrix() const;
		void				print();

		glm::vec3			getRotationEulers();

		static glm::vec3	interpolateTranslations(const Transform& previousTransform, const Transform& currentTransform, float alpha);
		static glm::quat	interpolateRotations(const Transform& previousTransform, const Transform& currentTransform, float alpha);
		static glm::vec3	interpolateScales(const Transform& previousTransform, const Transform& currentTransform, float alpha);
		static glm::mat4	interpolateTransforms(const Transform& previousTransform, const Transform& currentTransform, float alpha);

	};
	
    
}