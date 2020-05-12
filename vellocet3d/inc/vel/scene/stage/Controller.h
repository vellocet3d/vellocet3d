#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"


#include "vel/InputState.h"


namespace vel::scene::stage
{
	class Controller
	{
	protected:
		const InputState&	input;
		float				deltaTime = 0.0f;
		float				alphaTime = 0.0f;

	public:
							Controller();
		void				setDeltaTime(float delta);
		void				setAlphaTime(float alpha);
		virtual void		logic() = 0;
	};
}