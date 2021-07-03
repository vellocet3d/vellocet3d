#include <fstream>
#include <iostream>

#include "vel/Logger.h"

namespace vel
{
    Logger::Logger(bool enabled, std::string path, bool useConsole) :
        enabled(enabled),
        path(path),
		useConsole(useConsole),
		shouldDie(false)
	{};

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
		this->shouldDie = true;
		exit(EXIT_FAILURE);
	}

	bool Logger::getShouldDie() // useful for checking if logger exited on a separate thread
	{
		return this->shouldDie;
	}

}