#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "plf_colony/plf_colony.h"
#include "vel/sac.h"
#include "vel/Actor.h"
#include "vel/Camera.h"
#include "vel/Renderable.h"
#include "vel/CollisionWorld.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/Sensor.h"
#include "vel/GPU.h"
#include "vel/HDR.h"


namespace vel
{
	class Scene;

	class Stage
	{
	private:

		bool											visible;
		std::optional<Camera>							camera;
		sac<Actor>										actors;
		sac<Armature>									armatures;
		sac<Renderable>									renderables;
		CollisionWorld*									collisionWorld;
		bool											clearDepthBuffer;
		std::string										name;
        HDR*                                            activeHdr;
		bool											drawHdr;

		// Need this to apply correct IBL to first person stage that
		// logically does not have a view matrix (fixed to screen). With this member
		// we can obtain what the correct IBL would be if the objects in this stage where
		// using the camera pointed to by IBLCamera.
		Camera*											IBLCamera;
		
		void											_removeActor(Actor* a);


	public:
														Stage(std::string name);
														~Stage();
		void											updateActorAnimations(double runTime);
		Actor*											addActor(Actor a);
		void											removeActor(std::string name);
		void											removeActor(Actor* a);
		Actor*											getActor(std::string name);
		std::vector<Actor*>&							getActors();
		std::vector<Renderable*>& 						getRenderables();
		std::optional<Camera>&							getCamera();
		void											show();
		void											hide();
		void											addPerspectiveCamera(float nearPlane, float farPlane, float fov);
		void											addOrthographicCamera(float nearPlane, float farPlane, float scale);
		const bool										isVisible();
		void											setClearDepthBuffer(bool b);
		bool											getClearDepthBuffer();
		void											applyTransformations();
		void											setCollisionWorld(float gravity = -10.0f);
		CollisionWorld*									getCollisionWorld();
		void											stepPhysics(float delta);
		Armature*										addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actors);	
		const std::string&								getName() const;
		
		void 											setActiveHdr(HDR* h);
		HDR* 											getActiveHdr();
		void											setDrawHdr(bool b);
		bool											getDrawHdr();

		void											setIBLCamera(Camera* c);
		Camera*											getIBLCamera();
		
		Armature*										getArmature(std::string armatureName);

		

	};
}