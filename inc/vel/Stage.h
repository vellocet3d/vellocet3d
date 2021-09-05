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


	public:
														Stage(std::string name);
														~Stage();
		void											updateActorAnimations(double runTime);
		Actor*											addActor(Actor a);
		void											removeActor(std::string name);
		Actor*											getActor(std::string name);
		plf::colony<Actor>&								getActors();
		void											printRenderables() const;
		plf::colony<Renderable>& 						getRenderables();
		std::optional<Camera>&							getCamera();
		void											show();
		void											hide();
		void											addPerspectiveCamera(float nearPlane, float farPlane, float fov);
		void											addOrthographicCamera(float nearPlane, float farPlane, float scale);
		const bool										isVisible();
		void											parentActorToActor(std::string childName, std::string parentName);
		void											parentActorToActorBone(std::string childName, std::string parentName, std::string parentBoneName);
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

		

	};
}