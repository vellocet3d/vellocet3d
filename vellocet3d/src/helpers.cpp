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

	bool approximatelyEqual(float a, float b, float epsilon)
	{
		return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool essentiallyEqual(float a, float b, float epsilon)
	{
		return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool definitelyGreaterThan(float a, float b, float epsilon)
	{
		return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	bool definitelyLessThan(float a, float b, float epsilon)
	{
		return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
	}

	btVector3 glmToBulletVec3(glm::vec3 glmVec)
	{
		return btVector3(glmVec.x, glmVec.y, glmVec.z);
	}

}