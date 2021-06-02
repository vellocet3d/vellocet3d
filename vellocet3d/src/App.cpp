#include <iostream>
#include <limits>
#include <thread> 
#include <chrono>


#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


#include "vel/App.h"

using namespace std::chrono_literals;

namespace vel
{
	App* App::instance = nullptr;

    void App::init(Config conf)
    {
		static App inst(conf);
		App::instance = &inst;
    }
    
    App& App::get()
    {
        return *App::instance;
    }

    App::App(Config conf) :
        config(conf),
        logger(this->config.LOG_ENABLED, this->config.LOG_PATH, this->config.LOG_USE_CONSOLE),
		window(std::make_unique<Window>(this->config)),
		gpu(std::make_unique<GPU>()),
		scene(nullptr),
        startTime(std::chrono::high_resolution_clock::now())
    {


    }

	void App::forceImguiRender()
	{
		this->window->renderGui();
	}

	void App::showMouseCursor()
	{
		this->window->showMouseCursor();
	}

	void App::hideMouseCursor()
	{
		this->window->hideMouseCursor();
	}

	ImFont* App::getImguiFont(std::string key)
	{
		return this->window->getImguiFont(key);
	}

	Scene* App::getScene()
	{
		return this->scene.get();
	}

    void App::setScene(Scene* scene)
    {
		if(this->window->getImguiFrameOpen())
			this->forceImguiRender();

        this->scene = std::move(std::move(std::unique_ptr<Scene>(scene)));
    }

    void App::close()
    {
        this->shouldClose = true;
        this->window->setToClose();        
    }

    const double App::time() const
    {
        std::chrono::duration<double> t = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - this->startTime);
        return t.count();
    }

    const glm::ivec2& App::getScreenSize() const
    {
        return this->window->getScreenSize();
    }

    const InputState& App::getInputState() const
    {
        return this->window->getInputState();
    }

    void App::clearScene()
    {
        this->scene.reset();
    }

	GPU* App::getGPU()
	{
		return this->gpu.get();
	}

	float App::getFrameTime()
	{
		return (float)this->frameTime;
	}

	float App::getLogicTime()
	{
		return (float)this->fixedLogicTime;
	}

	void App::calculateAverageFrameTime()
	{
		this->canDisplayAverageFrameTime = false;

		if (this->time() - this->lastFrameTimeCalculation >= 1.0)
		{
			this->lastFrameTimeCalculation = this->time();

			double average = 0.0;

			for (auto& v : this->averageFrameTimeArray)
				average += v;

			average = average / this->averageFrameTimeArray.size();

			this->averageFrameTime = average;
			this->averageFrameRate = 1000 / (average * 1000);

			this->averageFrameTimeArray.clear();

			this->canDisplayAverageFrameTime = true;
		}

		this->averageFrameTimeArray.push_back(this->frameTime);
	}

    void App::displayAverageFrameTime()
    {
        if (this->canDisplayAverageFrameTime)
        {    
			std::string message = "FrameTime: " + std::to_string(this->averageFrameRate) + " | FPS: " + std::to_string(this->averageFrameRate);

			this->window->setTitle(message);
        }
    }

	bool App::getPauseBufferClearAndSwap()
	{
		return this->pauseBufferClearAndSwap;
	}

	void App::setPauseBufferClearAndSwap(bool in)
	{
		this->pauseBufferClearAndSwap = in;
	}

    void App::execute()
    {
        this->fixedLogicTime = 1 / this->config.LOGIC_TICK;
        this->currentTime = this->time();

        while (true)
        {
            if (this->shouldClose || this->window->shouldClose()) 
                break;

            if (this->scene != nullptr && !this->scene->loaded)
            {
                this->scene->load();
                this->scene->loaded = true;
				this->setPauseBufferClearAndSwap(false);
            }

            this->newTime = this->time();
            this->frameTime = this->newTime - this->currentTime;


            if (this->frameTime >= (1 / this->config.MAX_RENDER_FPS)) // cap max fps
            {
				this->calculateAverageFrameTime();
				this->displayAverageFrameTime();

                this->currentTime = this->newTime;				

                // prevent spiral of death
                if (this->frameTime > 0.25)
                    this->frameTime = 0.25;

                this->accumulator += this->frameTime;


				// update window, which includes capturing input state
				this->window->update();
				

                // process update logic
                while (this->accumulator >= this->fixedLogicTime)
                {                    
                    if (this->scene != nullptr && this->scene->loaded) 
                    {
						//// update animations
						//this->scene->updateAnimations(this->fixedLogicTime);

						//// step physics simulation
						//this->scene->stepPhysics((float)this->fixedLogicTime);

						// applyTransforms() ? incorporating current savePreviousTransforms() logic
						this->scene->applyTransformations();

						// execute all contact triggers
						this->scene->processSensors();

						// execute inner loop (fixed rate) logic
						this->scene->innerLoop((float)this->fixedLogicTime);

						// update animations
						this->scene->updateAnimations(this->fixedLogicTime);

						// step physics simulation
						this->scene->stepPhysics((float)this->fixedLogicTime);

						// call postPhysics method to allow correction of any issues caused by collision solver
						this->scene->postPhysics((float)this->fixedLogicTime);
                    }
                    
                    // decrement accumulator
                    this->accumulator -= this->fixedLogicTime;
                }

                

				float renderLerpInterval = (float)(this->accumulator / this->fixedLogicTime);

				// execute outer loop (immediate) logic
				if (this->scene->swapping)
				{
					this->setPauseBufferClearAndSwap(true);
					std::this_thread::sleep_for(2000ms); // for testing, can be removed
					this->setScene(this->scene->sceneToSwap);
				}
				else
				{
					this->scene->outerLoop((float)this->frameTime, renderLerpInterval);
				}
				

				// perform draw (render) logic
                this->gpu->enableDepthTest();
                //this->gpu->drawLinesOnly();
				if(!this->pauseBufferClearAndSwap)
					this->gpu->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);

                if (this->scene != nullptr && this->scene->loaded)
                {
					//std::cout << "call draw:" << this->currentTime << "\n";
                    this->scene->draw(renderLerpInterval);

					this->window->renderGui();
                }
				if (!this->pauseBufferClearAndSwap)
					this->window->swapBuffers();
                
            }

        }
    }

}