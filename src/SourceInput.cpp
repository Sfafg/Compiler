#include "SourceInput.h"
#include <sstream>
#include <iostream>
#include <fstream>

SourceInput::SourceInput(const char* path)
{
    std::stringstream ss;
    ss << std::ifstream(path).rdbuf();
    source = ss.str();

    current = source.begin();
}
SourceInput::SourceInput(const std::string& source) : source(source), current(this->source.begin()) {}
int SourceInput::Match(const std::string_view& pattern)
{
    if (current >= source.end() && pattern.size() == 0) return 1;
    for (int i = 0; i < pattern.size(); i++)
    {
        if (current + i >= source.end() || pattern[i] != *(current + i)) return -1;
    }
    return pattern.size();
}

int SourceInput::MatchRegex(const std::string_view& pattern)
{
    std::match_results<std::string_view::iterator> match;
    std::string_view sv(current, source.end());

    if (std::regex_search(sv.begin(), sv.end(), match, std::regex(pattern.begin(), pattern.end()), std::regex_constants::match_continuous))
        return match.length();

    return -1;
}
