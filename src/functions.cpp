#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

#include "glm/gtc/type_ptr.hpp"

#include "vel/functions.h"

namespace vel
{

	bool isPowerOfTwo(int n)
	{
		if (n == 0)
			return false;

		return (ceil(log2(n)) == floor(log2(n)));
	}

	std::string str_replace(const std::string& from, const std::string& to, std::string str)
	{
		if (from.empty())
			return str;

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}

		return str;
	}

	btQuaternion glmToBulletQuat(glm::quat glmQuat)
	{
		return btQuaternion(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w);
	}

	glm::quat bulletToGlmQuat(btQuaternion btQuat)
	{
		return glm::quat(btQuat.getW(),
			btQuat.getX(),
			btQuat.getY(),
			btQuat.getZ()
		);
	}

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

	bool sortOfEquals(const float a, const float b, const float tolerance)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	btVector3 glmToBulletVec3(glm::vec3 glmVec)
	{
		return btVector3(glmVec.x, glmVec.y, glmVec.z);
	}

	glm::vec3 bulletToGlmVec3(btVector3 btVec)
	{
		return glm::vec3(btVec.getX(), btVec.getY(), btVec.getZ());
	}

	glm::vec2 invertVec2(glm::vec2 in)
	{
		return glm::vec2(-in.x, -in.y);
	}

	glm::mat4 bulletTransformToGlmMat4(btTransform t)
	{
		glm::mat4 out;
		t.getOpenGLMatrix(glm::value_ptr(out));
		return out;
	}

	btMatrix3x3 glmMat3ToBulletMat3(const glm::mat3& m) 
	{ 
		return btMatrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]); 
	}


	btTransform glmMat4ToBulletTransform(const glm::mat4& m)
	{
		glm::mat3 m3(m);
		return btTransform(glmMat3ToBulletMat3(m3), glmToBulletVec3(glm::vec3(m[3][0], m[3][1], m[3][2])));
	}

	//https://stackoverflow.com/questions/4353525/floating-point-linear-interpolation
	float lerpf(float a, float b, float f)
	{
		return (a * (1.0 - f)) + (b * f);
	}

	bool randomFiftyFifty()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, 1);

		return distrib(gen) == 1;
	}

}