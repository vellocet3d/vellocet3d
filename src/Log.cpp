#include <fstream>
#include <sstream>
#include <iostream>

#include "vel/Log.h"

namespace vel
{
    Log* Log::instance = nullptr;

    void Log::init(std::string logFilePath)
    {
        if (Log::instance == nullptr)
        {
            static Log l(logFilePath);
            Log::instance = &l;
        }
    }

    Log& Log::get()
    {
        return *Log::instance;
    }

    Log::Log(std::string logFilePath) :
        filePath(logFilePath){}

    // ***NOTE*** 
    // Will not be called if program terminates before main returns 
    // (if you close cmd.exe by clicking close button for example)
    Log::~Log()
    {
        this->publishLog();
    }

    void Log::publishLog()
    {
        std::ofstream outStream(this->filePath, std::ofstream::trunc);

        for (auto& l : this->fileBuffer)
            outStream << l << std::endl;

        outStream.close();
    }

    void Log::toFile(std::string msg)
    {
        Log::get().fileBuffer.push_back(msg);
    }

    void Log::toCli(std::string msg)
    {
        std::cout << msg << std::endl;
    }

	void Log::toCliAndFile(std::string msg)
	{
		Log::toCli(msg);
		Log::toFile(msg);
	}

    void Log::crash(std::string msg)
    {
        Log::toFile(msg);
        exit(EXIT_FAILURE);
    }
}