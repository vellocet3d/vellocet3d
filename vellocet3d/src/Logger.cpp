#include <fstream>

#include "vel/Logger.h"

namespace vel
{
    Logger::Logger(bool enabled, std::string path) :
        enabled(enabled),
        path(path){};

    void Logger::log(std::string message)
    {
        if (this->enabled)
        {
            std::ofstream fs;
            fs.open(this->path, std::ios_base::app);
            fs << message << "\n";
            fs.close();
        }
    }

}