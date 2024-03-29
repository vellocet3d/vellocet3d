#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/sac.h"
#include "vel/Actor.h"
#include "vel/Camera.h"
#include "vel/Renderable.h"
#include "vel/Sensor.h"
#include "vel/GPU.h"
#include "vel/RenderMode.h"
#include "vel/Cubemap.h"


namespace vel
{
	class Scene;
	


	class Stage
	{
	private:
		bool											visible;
		Camera*											camera;
		sac<Actor>										actors;
		sac<Armature>									armatures;
		sac<Renderable>									renderables;
		bool											clearDepthBuffer;
		std::string										name;
		RenderMode										renderMode;
		Cubemap*										activeInfiniteCubemap;
		bool											useSceneCameraPositionForLighting;

		


		void											_removeActor(Actor* a);


	public:
														Stage(std::string name);
														~Stage();
		void											updateFixedArmatureAnimations(double runTime);
		void											updateArmatureAnimations(double runTime);
		Actor*											addActor(Actor a);
		void											removeActor(std::string name);
		void											removeActor(Actor* a);
		Actor*											getActor(std::string name);
		std::vector<Actor*>&							getActors();
		std::vector<Renderable*>& 						getRenderables();
		void											setCamera(Camera* c);
		Camera*											getCamera();
		void											show();
		void											hide();
		const bool										isVisible();
		void											setClearDepthBuffer(bool b);
		bool											getClearDepthBuffer();
		void											applyTransformations();

		Armature*										addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actors);	
		const std::string&								getName() const;
		
		Armature*										getArmature(std::string armatureName);

		RenderMode										getRenderMode();
		void											setRenderMode(RenderMode rm);

		void 											setActiveInfiniteCubemap(Cubemap* c);
		Cubemap* 										getActiveInfiniteCubemap();

		void											setUseSceneCameraPositionForLighting(bool b);
		bool											getUseSceneCameraPositionForLighting();
		

	};
}