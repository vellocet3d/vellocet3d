#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"


namespace vel::scene::stage
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
		void				setScale(glm::vec3 scale);
		const glm::vec3&	getTranslation() const;
		const glm::quat&	getRotation() const;
		const glm::vec3&	getScale() const;
		const glm::mat4		getMatrix() const;
		void				print();

		static glm::mat4	interpolateTransforms(const Transform& previousTransform, const Transform& currentTransform, float alpha);

	};
	
    
}