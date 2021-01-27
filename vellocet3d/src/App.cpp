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
        logger(this->config.LOG_ENABLED, this->config.LOG_PATH),
        startTime(std::chrono::high_resolution_clock::now())
    {
        if (!this->config.HEADLESS)
        {
			// create window
            auto w = std::make_unique<Window>(this->config);
            this->window = std::move(w);
        }

    }

	void App::forceImguiRender()
	{
		this->window.value()->renderGui();
	}

	void App::showMouseCursor()
	{
		this->window.value()->showMouseCursor();
	}

	void App::hideMouseCursor()
	{
		this->window.value()->hideMouseCursor();
	}

	ImFont* App::getImguiFont(std::string key)
	{
		if (!this->config.HEADLESS)
			return this->window.value()->getImguiFont(key);
		else
			return nullptr;
	}

	GLFWusercontext* App::getNextFreeOpenGLContext()
	{
		return this->window.value()->getNextFreeOpenGLContext();
	}

	scene::Scene* App::getScene()
	{
		return this->scene.value().get();
	}

    void App::blockingSceneLoad(scene::Scene* scene)
    {
		if(this->window && this->window.value()->getImguiFrameOpen())
			this->forceImguiRender();

        this->scene = std::move(std::move(std::unique_ptr<scene::Scene>(scene)));
		this->scene.value()->getGPU().value().primeGPU();
    }

	void App::nonBlockingSceneLoad(scene::Scene* scene, double minTime)
	{
		this->nextScene = scene;

		std::thread t([this, minTime]{

			double startTime = this->time();

			this->getNextScene()->getGPU().value().primeGPU();
			this->getNextScene()->load();
			this->getNextScene()->getGPU().value().finish();

			while (this->time() - startTime < minTime)
				std::this_thread::sleep_for(250ms);

			this->getNextScene()->loaded = true;

		});

		t.detach();
	}

	scene::Scene* App::getNextScene()
	{
		return this->nextScene;
	}

    void App::close()
    {
        this->shouldClose = true;
        if (!this->config.HEADLESS) 
        {
            this->window.value()->setToClose();
        }
    }

    const double App::time() const
    {
        std::chrono::duration<double> t = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - this->startTime);
        return t.count();
    }

    const glm::ivec2& App::getScreenSize() const
    {
        return this->window.value()->getScreenSize();
    }

    const InputState& App::getInputState() const
    {
        return this->window.value()->getInputState();
    }

    void App::clearScene()
    {
        this->scene.reset();
    }

	void App::clearNextScene()
	{
		this->nextScene = nullptr;
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
			{
				average += v;
			}

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

			if (!this->config.HEADLESS)
			{
				this->window.value()->setTitle(message);
			}
			else
			{
				std::cout << message << "\n";
			}
        }
    }

    void App::execute()
    {
        this->fixedLogicTime = 1 / this->config.LOGIC_TICK;
        this->currentTime = this->time();

        while (true)
        {
            if (this->shouldClose || (!this->config.HEADLESS && this->window.value()->shouldClose())) 
            {
                break;
            }

			// Used for when we load our initial scene from Main. When switching from one scene to another
			// the new scene will be loaded asynchronously so that the current scene will continue to be
			// responsive to the user
            if (this->scene && !this->scene.value()->loaded)
            {
                this->scene.value()->load();
                this->scene.value()->loaded = true;

				this->window.value()->setOpenGLContext(this->scene.value()->getGPU().value().getOpenGLContext());
            }

			
			if (this->nextScene && this->nextScene->loaded)
			{
				this->blockingSceneLoad(this->getNextScene());
				this->clearNextScene();

				// swap contexts
				this->window.value()->setOpenGLContext(this->scene.value()->getGPU().value().getOpenGLContext());

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
                {
                    this->frameTime = 0.25;
                }

                this->accumulator += this->frameTime;

				if (!this->config.HEADLESS)
				{
					// update window, which includes capturing input state
					this->window.value()->update();
				}

                // process update logic
                while (this->accumulator >= this->fixedLogicTime)
                {                    
                    if (this->scene && this->scene.value()->loaded) 
                    {
						// update animations
						this->scene.value()->updateAnimations(this->fixedLogicTime);

						//// step physics simulation
						//this->scene.value()->stepPhysics((float)this->fixedLogicTime);

						// applyTransforms() ? incorporating current savePreviousTransforms() logic
						this->scene.value()->applyTransformations();

						// execute all contact triggers
						this->scene.value()->processSensors();

						// execute inner loop (fixed rate) logic
						this->scene.value()->innerLoop((float)this->fixedLogicTime);

						// step physics simulation
						this->scene.value()->stepPhysics((float)this->fixedLogicTime);

						// call postPhysics method to allow correction of any issues caused by collision solver
						this->scene.value()->postPhysics((float)this->fixedLogicTime);
                    }
                    
                    // decrement accumulator
                    this->accumulator -= this->fixedLogicTime;
                }

                
                if (!this->config.HEADLESS)
                {
					float renderLerpInterval = (float)(this->accumulator / this->fixedLogicTime);

					// execute outer loop (immediate) logic
					this->scene.value()->outerLoop((float)this->frameTime, renderLerpInterval);

					// perform draw (render) logic
                    this->scene.value()->getGPU()->enableDepthTest();
                    //this->gpu->drawLinesOnly();
                    this->scene.value()->getGPU()->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);

                    if (this->scene && this->scene.value()->loaded)
                    {
						//std::cout << "call draw:" << this->currentTime << "\n";
                        this->scene.value()->draw(renderLerpInterval);

						this->window.value()->renderGui();
                    }

					
                    this->window.value()->swapBuffers();
                }

            }

        }
    }

}