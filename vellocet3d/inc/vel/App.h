#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <chrono>

#include "vel/Config.h"
#include "vel/Logger.h"
#include "vel/Window.h"
#include "vel/scene/Scene.h"



struct GLFWusercontext;

namespace vel 
{
    // https://stackoverflow.com/a/1008289/1609485
    class App
    {
    public:
        const Config									config;
        Logger											logger;


    private:
														App(Config conf);
		static App*										instance;
        std::optional<std::unique_ptr<Window>>			window;
        std::optional<std::unique_ptr<scene::Scene>>	scene;
		scene::Scene*									nextScene = nullptr;
        

        std::chrono::high_resolution_clock::time_point	startTime;
        bool											shouldClose = false;
        double											fixedLogicTime = 0.0;
        double											currentTime = 0.0;
        double											newTime = 0.0;
        double											frameTime = 0.0;
        double											accumulator = 0.0;        
        std::vector<double>								averageFrameTimeArray;
        double											lastFrameTimeCalculation = 0.0;
        void											displayAverageFrameTime();
		void											calculateAverageFrameTime();

		double											averageFrameTime = 0.0;
		double											averageFrameRate = 0.0;
		bool											canDisplayAverageFrameTime = false;
    

    public:
        static App&										get();
        static void										init(Config conf);
														App(App const&) = delete;
        void											operator=(App const&) = delete;
        void											setScene(scene::Scene* scene);
		scene::Scene*									getScene();
		void											loadNextScene(scene::Scene* scene);
		scene::Scene*									getNextScene();
		void											clearNextScene();
        void											clearScene();
        const double									time() const;
        const InputState&								getInputState() const;
        const glm::ivec2&								getScreenSize() const;
        void											execute();
        void											close();

		float											getFrameTime();
		float											getLogicTime();

		GLFWusercontext*								getNextFreeOpenGLContext();

    };
};
