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
        bool								LOG_ENABLED = true;
		double								LOGIC_TICK = 60.0;
		bool								CURSOR_HIDDEN = true;
		bool								USE_IMGUI = true;
		bool								AUTO_GENERATE_MIPMAPS = true;
        const std::string					LOG_PATH = "data/log.txt";
        const std::string					SHADER_FILE_PATH = "data/shaders";
        const std::string					DEFAULT_VERTEX_SHADER = "default.vert";
        const std::string					DEFAULT_FRAGMENT_SHADER = "default.frag";
		const std::string					DEFAULT_SKINNED_VERTEX_SHADER = "default_skinned.vert";
		const std::string					DEFAULT_SKINNED_FRAGMENT_SHADER = "default_skinned.frag";
		const std::string					DEFAULT_DEBUG_VERTEX_SHADER = "default_debug.vert";
		const std::string					DEFAULT_DEBUG_FRAGMENT_SHADER = "default_debug.frag";

		std::vector<ImGuiFont>				imguiFonts;
        

        // User defined via config.ini
        int									SCREEN_WIDTH;
        int									SCREEN_HEIGHT;
        bool								FULLSCREEN;
        double								MAX_RENDER_FPS;
		float								MOUSE_SENSITIVITY;

		void								updateConfigFile();

    };
}