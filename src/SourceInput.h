#pragma once
#include <string>
#include <regex>

struct SourceInput
{
    std::string source;
    std::string::iterator current;

    SourceInput(const char* path);
    SourceInput(const std::string& source);

    int MatchRegex(const std::string_view& pattern);
    int Match(const std::string_view& pattern);
};
