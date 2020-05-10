#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <chrono>

#include "vel/Config.h"
#include "vel/Logger.h"
#include "vel/Window.h"
#include "vel/scene/Scene.h"
#include "vel/GPU.h"


namespace vel 
{
    // https://stackoverflow.com/a/1008289/1609485
    class App
    {
    public:
        const Config									config;
        Logger											logger;


    private:
														App();
        std::optional<std::unique_ptr<Window>>			window;
        std::optional<std::unique_ptr<Scene>>			scene;
        std::optional<GPU>								gpu;

        std::chrono::high_resolution_clock::time_point	startTime;
        bool											shouldClose = false;
        double											deltaTime = 0.0;
        double											currentTime = 0.0;
        double											newTime = 0.0;
        double											frameTime = 0.0;
        double											accumulator = 0.0;        
        std::vector<double>								averageFrameTimeArray;
        double											lastDisplayTime = 0.0;
        void											displayAverageFrameTime();
    

    public:
        static App&										get();
        static void										init();
														App(App const&) = delete;
        void											operator=(App const&) = delete;
        void											setScene(Scene* scene);
        Scene*											getScene();
        std::optional<GPU>&								getGPU();
        void											clearScene();
        const double									time() const;
        const InputState&								getInputState() const;
        const glm::ivec2&								getScreenSize() const;
        void											execute();
        void											close();

    };
};
