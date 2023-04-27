#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/sac.h"
#include "vel/ptrsac.h"
#include "vel/Actor.h"
#include "vel/Camera.h"
#include "vel/Renderable.h"
#include "vel/GPU.h"
#include "vel/TextActor.h"

namespace vel
{
	class Scene; // ?
	
	class Stage
	{
	private:
		bool											visible;
		std::vector<Camera*>							cameras;
		sac<Actor>										actors;
		sac<Armature>									armatures;
		sac<Renderable>									renderables;
		sac<TextActor>									textActors;
		bool											clearDepthBuffer;
		std::string										name;	


		void											_removeActor(Actor* a);
		void											addActorToRenderable(Actor* actor);


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

		TextActor*										addTextActor(TextActor ta);
		TextActor*										getTextActor(std::string name);
		std::vector<TextActor*>&						getTextActors();
		void											removeTextActor(TextActor*);
		void											removeTextActor(std::string name);
		

		void											addCamera(Camera* c);
		Camera*											getCamera(std::string name);
		std::vector<Camera*>&							getCameras();

		void											show();
		void											hide();
		const bool										isVisible();
		void											setClearDepthBuffer(bool b);
		bool											getClearDepthBuffer();
		void											updatePreviousTransforms();

		Armature*										addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actors);	
		const std::string&								getName() const;
		
		Armature*										getArmature(std::string armatureName);	

		ptrsac<Material*>								animatedMaterials;

		/*
			After refactoring xd327 so that we can load either a headless or headful scene, it was discovered that
			since we do not add renderables to actors before adding them to their stage, that renderable functionality
			would be broken because renderables added to actors after the actor has been added to the stage would not be
			processed, therefore we need a way to "refresh" those lists. Since this is something that should only ever need
			to be done upon initialization of a scene, it shouldn't make much of a difference, although you would not want to
			have to do this everytime you added a renderable...BUT...again...once the scene is loaded any actors that are created
			during runtime should just be clones of existing preloaded renderables/actors, negating any reason for this to need to
			be run multiple times after initialization
		*/
		void											refreshRenderables();

	};
}