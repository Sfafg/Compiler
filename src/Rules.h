#pragma once
#include <string_view>
#include <map>
#include <array>
#include <regex>

enum class RuleType
{
    None = -1,
    Operator,
    SpacialCharacter,
    Keyword,
    StringLiteral,
    Identifire,
};
const std::array<const char*, 5> ruleTypeNames
{
    "Operator",
    "SpacialCharacter",
    "Keyword",
    "StringLiteral",
    "Identifire",
};

const std::array<std::regex, 5> terminalRuleProductions{
    std::regex("=|\\|"),
    std::regex(";|:|\\\\\\||\\\\\\;"),
    std::regex("rule"),
    std::regex("\".*\""),
    std::regex("[a-zA-Z_][a-zA-Z0-9_]*"),
};
std::array<std::map<int, std::string_view>, 5> terminalRuleStringMap{
};