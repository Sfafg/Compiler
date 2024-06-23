#pragma once
#include "Rules.h"
#include <iostream>
#include <span>

class Token
{
public:
    RuleType type;
    int value;
    std::string_view str;

    Token() : type(RuleType::None), value(-1) {}

    Token(std::string::iterator* sourceIt, std::string::iterator sourceEnd) : type(RuleType::None), value(-1)
    {
        // Find the longest match amongst productions rules with only terminal characters.
        std::string_view longestMatch;
        RuleType longestMatchType = RuleType::None;
        for (int i = 0; i < terminalRuleProductions.size(); i++)
        {
            const std::regex& production = terminalRuleProductions[i];
            std::match_results<std::string::iterator> match;
            if (std::regex_search(*sourceIt, sourceEnd, match, production, std::regex_constants::match_continuous) && match.length() > longestMatch.size())
            {
                longestMatch = std::string_view(*sourceIt, *sourceIt + match.length());
                longestMatchType = (RuleType) i;
            }
        }

        // If found nothing then return.
        if (longestMatchType == RuleType::None)
            return;

        // Apply found match.
        type = longestMatchType;
        str = longestMatch;

        // If matched string is not the first instance then find it and get it's id
        auto& stringMap = terminalRuleStringMap[(int) type];
        for (const auto& pair : stringMap)
        {
            if (pair.second == longestMatch)
            {
                value = pair.first;
                break;
            }
        }

        // Otherwise create a new entry.
        if (value == -1)
        {
            value = stringMap.size();
            stringMap[value] = longestMatch;
        }

        // Advance iterator by match length and skip any non usable characters.
        (*sourceIt) += longestMatch.size();
        while (std::string("\n \t").contains(**sourceIt)) ++(*sourceIt);
    }

};
std::ostream& operator<<(std::ostream& os, const Token& rhs)
{
    if (rhs.type == RuleType::None)
        return os << "<None>()";

    int id = (int) rhs.type;
    return os << '<' << ruleTypeNames[id] << ">(" << terminalRuleStringMap[id].at(rhs.value) << ")";
}