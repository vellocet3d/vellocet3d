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
											Config(std::string userConfigPath);
        // Application defined
        const bool							HEADLESS = false;
        const bool							LOG_ENABLED = true;
        const std::string					LOG_PATH = "data/log.txt";
        const std::string					SHADER_FILE_PATH = "data/shaders";
        const std::string					DEFAULT_VERTEX_SHADER = "default.vert";
        const std::string					DEFAULT_FRAGMENT_SHADER = "default.frag";
		const std::string					DEFAULT_SKINNED_VERTEX_SHADER = "default_skinned.vert";
		const std::string					DEFAULT_SKINNED_FRAGMENT_SHADER = "default_skinned.frag";
        const double						LOGIC_TICK = 30.0;

        // User defined via config.ini
        const int							SCREEN_WIDTH;
        const int							SCREEN_HEIGHT;
        const bool							FULLSCREEN;
        const double						MAX_RENDER_FPS;

    };
}