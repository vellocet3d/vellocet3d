#include <iostream>
#include <limits>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


#include "vel/App.h"


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
            auto w = std::make_unique<Window>(this->config.SCREEN_WIDTH, this->config.SCREEN_HEIGHT, 
				this->config.FULLSCREEN, this->config.CURSOR_HIDDEN, this->config.USE_IMGUI);
            this->window = std::move(w);

			// initialize GPU
			this->gpu = GPU(this->config.SHADER_FILE_PATH);

			// create default shaders
            this->gpu->loadShader("default", this->config.DEFAULT_VERTEX_SHADER, this->config.DEFAULT_FRAGMENT_SHADER);
			this->gpu->loadShader("default_skinned", this->config.DEFAULT_SKINNED_VERTEX_SHADER, this->config.DEFAULT_SKINNED_FRAGMENT_SHADER);
			this->gpu->loadShader("default_debug", this->config.DEFAULT_DEBUG_VERTEX_SHADER, this->config.DEFAULT_DEBUG_FRAGMENT_SHADER);

			// create default texture
			this->gpu->loadTexture("diffuse", ("data/models/default_texture.jpg"));
        }

    }

    void App::setScene(Scene* scene)
    {
        this->scene = std::move(std::move(std::unique_ptr<Scene>(scene)));
    }

    Scene* App::getScene()
    {
        return this->scene.value().get();
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

    std::optional<GPU>& App::getGPU()
    {
        return this->gpu;
    }

    void App::displayAverageFrameTime()
    {
        if (this->time() - this->lastDisplayTime >= 1.0) 
        {    
            this->lastDisplayTime = this->time();

            double average = 0.0;
            for (auto& v : this->averageFrameTimeArray)
            {
                average += v;
            }

            average = average / this->averageFrameTimeArray.size();
            
			std::string message = "FrameTime: " + std::to_string(average) + " | FPS: " + std::to_string(1000 / (average * 1000));

			if (!this->config.HEADLESS)
			{
				this->window.value()->setTitle(message);
			}
			else
			{
				std::cout << message << "\n";
			}

			this->averageFrameTimeArray.clear();
        }

        this->averageFrameTimeArray.push_back(this->frameTime);
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

            if (this->scene && !this->scene.value()->loaded)
            {
                this->scene.value()->load();
                this->scene.value()->loaded = true;
            }

            this->newTime = this->time();
            this->frameTime = this->newTime - this->currentTime;


            if (this->frameTime >= (1 / this->config.MAX_RENDER_FPS)) // cap max fps
            {
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
						// save previous transforms for interpolation
						//this->scene.value()->savePreviousTransforms();

						// update animations
						this->scene.value()->updateAnimations(this->fixedLogicTime);

						// step physics simulation
						this->scene.value()->stepPhysics((float)this->fixedLogicTime);

						// applyTransforms() ? incorporating current savePreviousTransforms() logic
						this->scene.value()->applyTransformations();

						// execute all contact triggers
						this->scene.value()->processSensors();

						// execute inner loop (fixed rate) logic
						this->scene.value()->innerLoop((float)this->fixedLogicTime);
                    }
                    
                    // decrement accumulator
                    this->accumulator -= this->fixedLogicTime;
                }

                
                if (!this->config.HEADLESS)
                {
					float renderLerpInterval = (float)(this->accumulator / this->fixedLogicTime);

					// execute outer loop controllers
					//this->scene.value()->executeOuterLoopControllers((float)this->frameTime, renderLerpInterval);

					// execute outer loop (immediate) logic
					this->scene.value()->outerLoop((float)this->frameTime, renderLerpInterval);

					// perform draw (render) logic
                    this->gpu->enableDepthTest();
                    //this->gpu->drawLinesOnly();
                    this->gpu->clearBuffers(0.2f, 0.3f, 0.3f, 1.0f);

                    if (this->scene && this->scene.value()->loaded)
                    {
                        this->scene.value()->draw(renderLerpInterval);
                    }

					this->window.value()->renderGui();
                    this->window.value()->swapBuffers();
                }

            }

        }
    }

}