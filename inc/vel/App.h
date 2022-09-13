#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <optional>
#include <chrono>

#include "vel/Config.h"
#include "vel/Window.h"
#include "vel/GPU.h"
#include "vel/Scene.h"
#include "vel/AssetManager.h"


struct GLFWusercontext;

namespace vel 
{
    // https://stackoverflow.com/a/1008289/1609485
    class App
    {
    public:
        Config											config;


    private:
														App(Config conf);
		static App*										instance;
        std::unique_ptr<Window>							window;
		std::unique_ptr<GPU>							gpu;
		AssetManager									assetManager;
		std::deque<std::unique_ptr<Scene>>				sceneLoadingQueue;
        std::vector<std::unique_ptr<Scene>>				scenes;
		Scene*											activeScene;
		
        

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
		bool											pauseBufferClearAndSwap = false;
    

    public:
        static App&										get();
        static void										init(Config conf);
														App(App const&) = delete;
        void											operator=(App const&) = delete;
        void											addScene(Scene* scene, bool swapWhenLoaded = false);
        const double									time() const;
        const InputState&								getInputState() const;
        const glm::ivec2&								getScreenSize() const;
        void											execute();
        void											close();

		float											getFrameTime();
		float											getLogicTime();


		ImFont*											getImguiFont(std::string key) const;
		void											hideMouseCursor();
		void											showMouseCursor();

		void											forceImguiRender();

		GPU*											getGPU();
		bool											getPauseBufferClearAndSwap();
		void											setPauseBufferClearAndSwap(bool in);

		AssetManager&									getAssetManager();
		Scene*											getNextSceneToLoad();

		void											removeScene(std::string name);
		void											swapScene(std::string name);
		bool											sceneExists(std::string name);

		Scene*											getActiveScene();

    };
};
