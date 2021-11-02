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
		window(std::make_unique<Window>(this->config)),
		gpu(std::make_unique<GPU>(this->window.get())),
		assetManager(AssetManager(this->gpu.get())),
		activeScene(nullptr),
        startTime(std::chrono::high_resolution_clock::now())
    {        
        // load default shaders
        this->assetManager.loadShader("defaultEquirectangularToCubemap", 
        "data/shaders/cubemap.vert", "data/shaders/equirectangular_to_cubemap.frag");
        
        this->assetManager.loadShader("defaultIrradianceConvolution", 
        "data/shaders/cubemap.vert", "data/shaders/irradiance_convolution.frag");
        
        this->assetManager.loadShader("defaultPrefilter", 
        "data/shaders/cubemap.vert", "data/shaders/prefilter.frag");
        
        this->assetManager.loadShader("defaultBrdf", 
        "data/shaders/brdf.vert", "data/shaders/brdf.frag");
        
        this->assetManager.loadShader("defaultBackground", 
        "data/shaders/background.vert", "data/shaders/background.frag");
        
        
        
        this->assetManager.loadShader("defaultDebug", 
        "data/shaders/default_debug.vert", "data/shaders/default_debug.frag");
        
        this->assetManager.loadShader("defaultRenderable", 
        "data/shaders/default_renderable.vert", "data/shaders/default_renderable.frag");
        
        this->assetManager.loadShader("defaultSkinnedRenderable", 
        "data/shaders/default_skinned_renderable.vert", "data/shaders/default_renderable.frag");
        
        
        
        // send all these shaders to gpu
        this->assetManager.sendAllToGpu();
        
        
        // pass shaders gpu needs for generating hdr assets
        this->gpu->initPbrShaders(
            this->assetManager.getShader("defaultEquirectangularToCubemap"),
            this->assetManager.getShader("defaultIrradianceConvolution"),
            this->assetManager.getShader("defaultPrefilter"),
            this->assetManager.getShader("defaultBrdf"),
            this->assetManager.getShader("defaultBackground")
        );
        
        
        // load default hdr image
        this->assetManager.loadHdr("defaultHdr", "data/default_textures/default.hdr");
        this->assetManager.sendAllToGpu();
        
        
        // load default textures
		this->assetManager.loadTexture("defaultAlbedo", "albedo", "data/default_textures/albedo.jpg");
		this->assetManager.loadTexture("defaultAO", "ao", "data/default_textures/ao.jpg");
		this->assetManager.loadTexture("defaultMetallic", "metallic", "data/default_textures/metallic.jpg");
		this->assetManager.loadTexture("defaultRoughness", "roughness", "data/default_textures/roughness.jpg");
		this->assetManager.loadTexture("defaultNormal", "normal", "data/default_textures/normal.jpg");
		this->assetManager.sendAllToGpu();


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
		//if(this->window->getImguiFrameOpen())
		//	this->forceImguiRender();
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

			//        if (this->scene != nullptr && !this->scene->loaded)
			//        {
			//            this->scene->load();
			//            this->scene->loaded = true;
						//this->setPauseBufferClearAndSwap(false);
			//        }


            this->newTime = this->time();
            this->frameTime = this->newTime - this->currentTime;
			
			this->window->updateInputState();

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




				// update window, which includes capturing input state
				//this->window->updateInputState();
				this->window->update();
				
				
                // process update logic
                while (this->accumulator >= this->fixedLogicTime)
                {
					//// update animations
					//this->scene->updateAnimations(this->fixedLogicTime);

					//// step physics simulation
					//this->scene->stepPhysics((float)this->fixedLogicTime);

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
				if(!this->pauseBufferClearAndSwap)
					this->gpu->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);


                this->activeScene->draw(renderLerpInterval);


				this->window->renderGui();


				if (!this->pauseBufferClearAndSwap)
					this->window->swapBuffers();
                

            }

        }
    }

}