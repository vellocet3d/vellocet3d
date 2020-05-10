#pragma once

#include <vector>
#include <string>

namespace vel
{
    std::vector<std::string> explode_string(std::string const & s, char delim);
    std::string char_to_string(char* a);
	bool sin_vector(std::string needle, std::vector<std::string> haystack);
	bool string_contains(std::string needle, std::string haystack);
}