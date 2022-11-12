#pragma once

#include <string>
#include <map>
#include <vector>


namespace vel
{
	struct ImGuiFont
	{
		std::string key;
		std::string path;
		float		pixels;
	};

    class Config
    {
    private:
        std::map<std::string, std::string>	loadFromFile(std::string path);
        std::map<std::string, std::string>	userConfigParams;        


    public:
											Config();
        // Application defined
		double								LOGIC_TICK = 60.0;
		bool								CURSOR_HIDDEN = true;
		bool								USE_IMGUI = true;
        const std::string					LOG_PATH = "data/log.txt";
        bool                                OPENGL_DEBUG_CONTEXT = true;
		std::string							APP_EXE_NAME = "MyApp.exe";
		std::string							APP_NAME = "MyApp";
		bool								HEADLESS = false;
		

		std::vector<ImGuiFont>				imguiFonts;
        

        // User defined via config.ini
        int									SCREEN_WIDTH;
        int									SCREEN_HEIGHT;
        bool								FULLSCREEN;
        double								MAX_RENDER_FPS;
		float								MOUSE_SENSITIVITY;
		bool								VSYNC;

		void								updateConfigFile();

    };
}