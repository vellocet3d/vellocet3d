#pragma once

#include <string>
#include <map>


namespace vel
{
    class Config
    {
    private:
        std::map<std::string, std::string>	loadFromFile(std::string path);
        std::map<std::string, std::string>	userConfigParams;        


    public:
											Config();
        // Application defined
        bool								HEADLESS = false;
        bool								LOG_ENABLED = true;
		double								LOGIC_TICK = 30.0;
		bool								CURSOR_HIDDEN = true;
        const std::string					LOG_PATH = "data/log.txt";
        const std::string					SHADER_FILE_PATH = "data/shaders";
        const std::string					DEFAULT_VERTEX_SHADER = "default.vert";
        const std::string					DEFAULT_FRAGMENT_SHADER = "default.frag";
		const std::string					DEFAULT_SKINNED_VERTEX_SHADER = "default_skinned.vert";
		const std::string					DEFAULT_SKINNED_FRAGMENT_SHADER = "default_skinned.frag";
        

        // User defined via config.ini
        const int							SCREEN_WIDTH;
        const int							SCREEN_HEIGHT;
        const bool							FULLSCREEN;
        const double						MAX_RENDER_FPS;

    };
}