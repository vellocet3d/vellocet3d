#pragma once

#include <string>


namespace vel
{

    class Logger
    {
    private:
        bool                enabled;
        std::string         path;
		bool				useConsole;

    public:
                            Logger(bool enabled, std::string path, bool useConsole);
        void                log(std::string entry);
    };

}