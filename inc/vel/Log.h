#pragma once

#include <iostream>
#include <string>
#include <deque>
#include <thread>


namespace vel 
{
    class Log {
        
    private:

        std::deque<std::string>		fileBuffer;
        std::string					filePath;

                                    Log(std::string logFilePath);
        static Log*					instance;
        static Log&					get();
        void						publishLog();
        




    public:
                            ~Log();
                            Log(Log const&) = delete;
        void				operator=(Log const&) = delete;
        static void			init(std::string logFilePath);
        

        static void			toFile(std::string msg);
        static void			toCli(std::string msg);
        static void			crash(std::string msg);

    };
}