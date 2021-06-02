#include <fstream>
#include <iostream>

#include "vel/Logger.h"

namespace vel
{
    Logger::Logger(bool enabled, std::string path, bool useConsole) :
        enabled(enabled),
        path(path),
		useConsole(useConsole){};

    void Logger::log(std::string message)
    {
        if (this->enabled)
        {
            std::ofstream fs;
            fs.open(this->path, std::ios_base::app);
            fs << message << "\n";
            fs.close();

			if (this->useConsole)
			{
				std::cout << message << std::endl;
				std::cin.get();
			}
        }
    }

	void Logger::die(std::string message)
	{
		this->log(message);
		exit(EXIT_FAILURE);
	}

}