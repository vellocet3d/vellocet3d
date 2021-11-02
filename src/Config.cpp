#include <fstream>
#include <sstream>
#include <iostream>

#include "vel/Config.h"
#include "vel/functions.h"


namespace vel
{
    Config::Config() :
        userConfigParams(this->loadFromFile("data/config.ini")),
        SCREEN_WIDTH(userConfigParams.count("screenWidth") != 0 ? std::stoi(this->userConfigParams["screenWidth"]) : 1280),
        SCREEN_HEIGHT(userConfigParams.count("screenHeight") != 0 ? std::stoi(this->userConfigParams["screenHeight"]) : 720),
        FULLSCREEN(userConfigParams.count("fullScreen") != 0 ? (this->userConfigParams["fullScreen"] == "0" ? false : true) : false),
        MAX_RENDER_FPS(userConfigParams.count("maxFps") != 0 ? std::stod(this->userConfigParams["maxFps"]) : 240),
		MOUSE_SENSITIVITY(userConfigParams.count("mouseSensitivity") != 0 ? std::stof(this->userConfigParams["mouseSensitivity"]) : 0.08f),
		VSYNC(userConfigParams.count("vsync") != 0 ? (this->userConfigParams["vsync"] == "0" ? false : true) : false)
	{};

	void Config::updateConfigFile()
	{
		std::ofstream outStream("data/config.ini", std::ofstream::trunc);

		outStream << "screenWidth=" << this->SCREEN_WIDTH << std::endl;
		outStream << "screenHeight=" << this->SCREEN_HEIGHT << std::endl;
		outStream << "fullScreen=" << (this->FULLSCREEN ? "1" : "0") << std::endl;
		outStream << "maxFps=" << this->MAX_RENDER_FPS << std::endl;
		outStream << "mouseSensitivity=" << this->MOUSE_SENSITIVITY << std::endl;
		outStream << "vsync=" << this->VSYNC << std::endl;

		outStream.close();

	}

    std::map<std::string, std::string> Config::loadFromFile(std::string path)
    {
        std::ifstream conf;
        try
        {
            conf.open(path); // not throwing error when file path incorrect for some reason

            if (!conf.is_open()) 
            {
                std::cout << "Failed to load config.ini file at path: " << path << "\n";
                std::cin.get();
                exit(EXIT_FAILURE);
            }

            std::map<std::string, std::string> returnMap;

            std::string line;
            while (std::getline(conf, line))
            {
                std::istringstream iss(line);
                std::vector<std::string> e = explode_string(iss.str(), '=');
                returnMap[e[0]] = e[1];
            }

            return returnMap;

        }
        catch (std::ifstream::failure e)
        {
            std::cout << "Failed to load config.ini file at path: " << e.what() << "\n";
            std::cin.get();
            exit(EXIT_FAILURE);
        }
    }

}