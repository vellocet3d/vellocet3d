#include <iostream>
#include <limits>
#include <thread> 
#include <chrono>


#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


#include "vel/App.h"
#include "vel/Log.h"

using namespace std::chrono_literals;

namespace vel
{
	App* App::instance = nullptr;

    void App::init(Config conf)
    {
		// initialze Log
		Log::init(conf.LOG_PATH);

		static App inst(conf);
		App::instance = &inst;
    }
    
    App& App::get()
    {
        return *App::instance;
    }

    App::App(Config conf) :
        config(conf),
		window(conf.HEADLESS ? nullptr : std::make_unique<Window>(this->config)),
		gpu(conf.HEADLESS ? nullptr : std::make_unique<GPU>()),
		assetManager(AssetManager(conf.HEADLESS ? nullptr : this->gpu.get())),
		activeScene(nullptr),
        startTime(std::chrono::high_resolution_clock::now())
    {

        /* load default shaders and textures if not in headless mode
		***************************************************************/
		if (!conf.HEADLESS)
		{
			this->assetManager.loadShader("defaultDebug",
				"data/shaders/defaults/debug.vert", "data/shaders/defaults/debug.frag");

			this->assetManager.loadShader("default",
				"data/shaders/defaults/default.vert", "data/shaders/defaults/default.frag");

			this->assetManager.loadShader("defaultInvertUV",
				"data/shaders/defaults/default.vert", "data/shaders/defaults/defaultInvertUV.frag");

			this->assetManager.loadShader("defaultSkinned",
				"data/shaders/defaults/default_skinned.vert", "data/shaders/defaults/default.frag");

			// used for rendering texture to screen buffer
			this->assetManager.loadShader("defaultScreen",
				"data/shaders/defaults/screen.vert", "data/shaders/defaults/screen.frag");

			// send all these shaders and textures to gpu
			this->assetManager.sendAllToGpu();

			this->gpu->setDefaultShader(this->assetManager.getShader("defaultScreen"));
		}
        

		//create separate thread that will poll scenes and load them into system memory asynchronously
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

	Scene* App::getActiveScene()
	{
		return this->activeScene;
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
#ifdef DEBUG_LOG
	Log::toCliAndFile("Removing Scene: " + name);
#endif
		size_t i = 0;
		for (auto& s : this->scenes)
		{
			if (s->getName() == name)
				break;
			
			i++;
		}

		this->scenes.erase(this->scenes.begin() + i);
	}
	
	bool App::sceneExists(std::string name)
	{
		for (auto& s : this->scenes)
			if (s->getName() == name)
				return true;

		return false;
	}

	void App::swapScene(std::string name)
	{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Swapping to Scene: " + name);
#endif
		for (auto& s : this->scenes)
			if (s->getName() == name)
				this->activeScene = s.get();
	}

    void App::addScene(Scene* scene, bool swapWhenLoaded)
    {
		// TODO: this was required before to get imgui to work correctly, but that was when we could only ever load
		// one scene at a time. Need to figure out how to integrate imgui into the new api
		if(this->window != nullptr && this->window->getImguiFrameOpen())
			this->forceImguiRender();
        //this->scene = std::move(std::move(std::unique_ptr<Scene>(scene)));

		std::string className = typeid(*scene).name();// name is "class Test" when we need just "Test", so trim off "class "
		className.erase(0, 6);
		scene->setName(className);
		scene->swapWhenLoaded = swapWhenLoaded;
		
#ifdef DEBUG_LOG
	Log::toCliAndFile("Adding Scene: " + className);
#endif

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
        //if (this->canDisplayAverageFrameTime)
        //{
			std::string message = "CurrentTime: " + std::to_string(this->currentTime) + " | FPS: " + std::to_string(this->averageFrameRate);

			this->window->setTitle(message);
        //}
    }

	bool App::getPauseBufferClearAndSwap()
	{
		return this->pauseBufferClearAndSwap;
	}

	void App::setPauseBufferClearAndSwap(bool in)
	{
		this->pauseBufferClearAndSwap = in;
	}

	double App::getCurrentTime()
	{
		return this->currentTime;
	}

	void App::stepSimulation(float dt)
	{
		if (this->activeScene == nullptr)
			return;

		// check if there is a scene loading on the loading thread, and if so and it is complete,
		// and it should be swapped to when loaded, then swap the active scene with this one and
		// clear it from the loading queue
		if (this->sceneLoadingQueue.size() > 0)
		{
			if (this->sceneLoadingQueue.at(0)->mainMemoryloaded)
			{
				if (this->sceneLoadingQueue.at(0)->swapWhenLoaded)
				{
					this->activeScene = this->sceneLoadingQueue.at(0).get();
				}

				this->scenes.push_back(std::move(this->sceneLoadingQueue.at(0)));

				this->sceneLoadingQueue.pop_front();
			}
		}

		//// step physics simulation
		this->activeScene->stepPhysics(dt);

		// TODO: at some point we will probably want to break dynamic actors out into
		// their own container so we're not looping over and checking static actors,
		// which there could be many of and would never need to have their transforms
		// updated
		this->activeScene->applyTransformations();

		// execute all contact triggers
		this->activeScene->processSensors();

		// execute inner loop (fixed rate) logic
		this->activeScene->innerLoop(dt);

		// update animations
		this->activeScene->updateFixedAnimations(dt);

		// call postPhysics method to allow correction of any issues caused by collision solver
		this->activeScene->postPhysics(dt);
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
			this->assetManager.sendNextToGpu();


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


            this->newTime = this->time();
            this->frameTime = this->newTime - this->currentTime;
			
			//this->window->updateInputState(); // not sure there's a reason for doing this here as opposed to in capped fps loop since it would never be sampled until then anyway
			//this->window->update();

            if (this->frameTime >= (1 / this->config.MAX_RENDER_FPS)) // cap max fps
            {
				this->calculateAverageFrameTime();
				this->displayAverageFrameTime();
				

                this->currentTime = this->newTime;				

				
				//std::cout << "---------------------------------------\n" << this->currentTime << ": " << this->frameTime << "----------------------------" << std::endl;

                // prevent spiral of death
                if (this->frameTime > 0.25)
                    this->frameTime = 0.25;

                this->accumulator += this->frameTime;




				// update window
				this->window->updateInputState();
				this->window->update();
				
				
                // process update logic
                while (this->accumulator >= this->fixedLogicTime)
                {
					//// update animations
					//this->scene->updateAnimations(this->fixedLogicTime);

					//// step physics simulation
					this->activeScene->stepPhysics((float)this->fixedLogicTime);

					// TODO: at some point we will probably want to break dynamic actors out into
					// their own container so we're not looping over and checking static actors,
					// which there could be many of and would never need to have their transforms
					// updated
					this->activeScene->applyTransformations();

					// execute all contact triggers
					this->activeScene->processSensors();

					// execute inner loop (fixed rate) logic
					this->activeScene->innerLoop((float)this->fixedLogicTime);

					// update animations
					this->activeScene->updateFixedAnimations(this->fixedLogicTime);

					// step physics simulation
					//this->activeScene->stepPhysics((float)this->fixedLogicTime);

					// call postPhysics method to allow correction of any issues caused by collision solver
					this->activeScene->postPhysics((float)this->fixedLogicTime);
                    
                    
                    // decrement accumulator
                    this->accumulator -= this->fixedLogicTime;
                }
				
				this->activeScene->updateMaterialAnimations(this->frameTime);

				//this->activeScene->updateAnimations(this->frameTime); // not sure why this was commented out, but probably shouldn't be
																		// i believe this is where we would update a skeletal animation which
																		// does not have a flag of shouldInterpolate...meaning it would be a realtime
																		// animation, not required to be updated at a fixed rate for consistent logic
																		// so yeah, this doesn't need to be commented out, but leaving for now until
																		// we're done with what we're doing


				float renderLerpInterval = (float)(this->accumulator / this->fixedLogicTime);


				// execute outer loop (immediate) logic
				this->activeScene->outerLoop((float)this->frameTime, renderLerpInterval);
				

				// perform draw (render) logic

				//if (!this->pauseBufferClearAndSwap)
				//{
				//this->gpu->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);
				//	//this->gpu->clearBuffers(0.0f, 0.0f, 0.0f, 1.0f);
				//}

				// clear all previous render target buffers, this is done here as doing it right before or right after
				// we draw, wouldn't work as far as I can tell at the moment as many stages can have many cameras and
				// many cameras can have many stages, meaning that if we clear render buffers after drawing to camera's render target
				// in one stage, if it's used in another stage we would get not good results
				this->activeScene->clearAllRenderTargetBuffers();
				
					

                this->activeScene->draw(renderLerpInterval);


				//this->window->renderGui();



				
				this->window->swapBuffers();
            }

        }
    }

}