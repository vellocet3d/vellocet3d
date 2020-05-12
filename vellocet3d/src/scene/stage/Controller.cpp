


#include "vel/App.h"
#include "vel/scene/stage/Controller.h"



namespace vel::scene::stage
{

	Controller::Controller() :
		input(App::get().getInputState())
	{}

	void Controller::setDeltaTime(float delta)
	{
		this->deltaTime = delta;
	}

	void Controller::setAlphaTime(float alpha)
	{
		this->alphaTime = alpha;
	}



}