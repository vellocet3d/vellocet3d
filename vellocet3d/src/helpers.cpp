#include <fstream>
#include <sstream>
#include <algorithm>

#include "vel/helpers.h"

namespace vel
{
    std::vector<std::string> explode_string(std::string const & s, char delim)
    {
        std::vector<std::string> result;
        std::istringstream iss(s);

        for (std::string token; std::getline(iss, token, delim); )
        {
            result.push_back(std::move(token));
        }

        return result;
    }

    std::string char_to_string(char* a)
    {
        std::string s = "";
        for (int i = 0; i < (sizeof(a) / sizeof(char)); i++) {
            s = s + a[i];
        }
        return s;
    }

	bool sin_vector(std::string needle, std::vector<std::string> haystack)
	{
		if (std::find(haystack.begin(), haystack.end(), needle) != haystack.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool string_contains(std::string needle, std::string haystack)
	{
		if (haystack.find(needle) != std::string::npos)
		{
			return true;
		}
		return false;
	}

}