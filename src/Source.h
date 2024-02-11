#pragma once
#include <string>
#include <regex>
#include <iostream>
#include <fstream>

struct Source
{
    std::string source;
    std::string::iterator current;

    Source(const char* path)
    {
        std::stringstream ss;
        ss << std::ifstream(path).rdbuf();
        source = ss.str();

        current = source.begin();
    }
    Source(const std::string& source) : source(source), current(this->source.begin()) {}

    std::string::iterator Match(std::string_view word)
    {
        if (word == std::string("") && *current == 0) return current + 1;
        for (int i = 0; i < word.size(); i++)
        {
            if (current + i > source.end() || current[i] != word[i])
            {
                if (word[i] == '\0')
                    return current + word.size();
                return current;
            }
        }
        return current + word.size();
    }
    std::string::iterator Match(const std::regex& pattern)
    {
        std::match_results<std::string_view::iterator> match;
        std::string_view sv(current, source.end());

        if (std::regex_search(sv.begin(), sv.end(), match, pattern, std::regex_constants::match_continuous))
            return current + match.length();

        return current;
    }
};
