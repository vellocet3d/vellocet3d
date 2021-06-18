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
		assetManager(AssetManager()),
		activeScene(nullptr),
        startTime(std::chrono::high_resolution_clock::now())
    {

		// create separate thread that will poll scenes and load them into system memory asynchronously
		std::thread t([this] {

			while (true)
			{
				std::this_thread::sleep_for(250ms);

				Scene* nextScene = this->getNextSceneToLoad();

				if (nextScene != nullptr)
				{
					nextScene->load();
					nextScene->mainMemoryloaded = true;
				}
			}

		});
		t.detach();

    }

	Scene* App::getNextSceneToLoad()
	{
		if (this->sceneLoadingQueue.size() == 0)
			return nullptr;

		return this->sceneLoadingQueue.at(0).get();
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

	ImFont* App::getImguiFont(std::string key) const
	{
		return this->window->getImguiFont(key);
	}

	void App::removeScene(std::string name)
	{
		size_t i = 0;
		for (auto& s : this->scenes)
		{
			if (s->getName() == name)
			{
				s->freeAssets();
				break;
			}
			i++;
		}

		this->scenes.erase(this->scenes.begin() + i);
	}

    void App::addScene(Scene* scene, bool swapWhenLoaded)
    {
		//if(this->window->getImguiFrameOpen())
		//	this->forceImguiRender();

        //this->scene = std::move(std::move(std::unique_ptr<Scene>(scene)));

		this->sceneLoadingQueue.push_back(std::move(std::unique_ptr<Scene>(scene)));
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

	GPU* App::getGPU()
	{
		return this->gpu.get();
	}

	AssetManager& App::getAssetManager()
	{
		return this->assetManager;
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

			

			// load a single gpu asset for this loop cycle if needed
			this->assetManager.sendToGpu();
			

			if (this->sceneLoadingQueue.size() > 0)
			{
				// if there is a scene in the loading queue, then check whether all of it's assets have
				// been loaded into both main memory and gpu memory and if so and it has swapWhenLoaded set
				// then set activeScene to this scene after moving it into this->scenes, then pop it off sceneLoadingQueue

				if (this->sceneLoadingQueue.at(0)->isFullyLoaded())
				{
					if (this->sceneLoadingQueue.at(0)->swapWhenLoaded)
						this->activeScene = this->sceneLoadingQueue.at(0).get();

					this->scenes.push_back(std::move(this->sceneLoadingQueue.at(0)));

					// TODO: I believe this is necessary, but not 100%, so this COULD be a nasty runtime bug
					this->sceneLoadingQueue.pop_front();
				}

			}

			if (this->activeScene == nullptr)
				continue;

    //        if (this->scene != nullptr && !this->scene->loaded)
    //        {
    //            this->scene->load();
    //            this->scene->loaded = true;
				//this->setPauseBufferClearAndSwap(false);
    //        }

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

					//// update animations
					//this->scene->updateAnimations(this->fixedLogicTime);

					//// step physics simulation
					//this->scene->stepPhysics((float)this->fixedLogicTime);

					// applyTransforms() ? incorporating current savePreviousTransforms() logic
					this->activeScene->applyTransformations();

					// execute all contact triggers
					this->activeScene->processSensors();

					// execute inner loop (fixed rate) logic
					this->activeScene->innerLoop((float)this->fixedLogicTime);

					// update animations
					this->activeScene->updateAnimations(this->fixedLogicTime);

					// step physics simulation
					this->activeScene->stepPhysics((float)this->fixedLogicTime);

					// call postPhysics method to allow correction of any issues caused by collision solver
					this->activeScene->postPhysics((float)this->fixedLogicTime);
                    
                    
                    // decrement accumulator
                    this->accumulator -= this->fixedLogicTime;
                }

                

				float renderLerpInterval = (float)(this->accumulator / this->fixedLogicTime);

				// execute outer loop (immediate) logic
				this->activeScene->outerLoop((float)this->frameTime, renderLerpInterval);
				

				// perform draw (render) logic
                this->gpu->enableDepthTest();
                //this->gpu->drawLinesOnly();
				if(!this->pauseBufferClearAndSwap)
					this->gpu->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);

				//std::cout << "call draw:" << this->currentTime << "\n";
                this->activeScene->draw(renderLerpInterval);

				this->window->renderGui();
                
				if (!this->pauseBufferClearAndSwap)
					this->window->swapBuffers();
                
            }

        }
    }

}