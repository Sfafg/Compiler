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

struct Node
{
    RuleType type = RuleType::None;
    Node* parent = nullptr;
    Node** children = nullptr;
    int childrenCount = 0;
    Token* token = nullptr;
};
std::string Shorten(const Node& ptr);
std::ostream& operator<<(std::ostream& os, const Node& node)
{
    os << "\"" << Shorten(node) << "\" [label=\"";
    if (node.token != nullptr)
    {
        if (node.type == RuleType::StringLiteral)
        {
            std::string_view data = node.token->str;
            for (int i = 0; i < data.size(); i++)
            {
                if (data[i] == '\\' || data[i] == '\"') os << '\\';
                os << data[i];
                if (i == 15 - 1 && data.size() > 5 + 15 * 2)
                {
                    os << "\\n...\\n";
                    i = data.size() - 15;
                }
            }
            return os << "\"]\n";
        }
        else if (node.token->str == "\"" || node.token->str == "\\")return os << "\\n\\" << node.token->str << "\"]\n";
        else return os << node.token->str << "\"]\n";
    }
    else
    {
        if (node.type != RuleType::None)
            os << ruleTypeNames[(int) node.type] << "\"]\n";
        else
            os << "None\"]\n";
        for (int i = 0; i < node.childrenCount; i++)
        {
            auto& child = node.children[i];
            os << "\"" << Shorten(node) << "\" -> \"" << Shorten(*child) << "\"\n";
        }
        if (node.childrenCount != 0)os << '\n';
        for (int i = 0; i < node.childrenCount; i++)
        {
            auto& child = node.children[i];
            os << *child;
        }
        return os;
    }
}
std::string Shorten(const Node& ptr)
{
    unsigned long long num = (unsigned long long) & ptr;
    static const char* digits = "1234567890-=!@#$%^&*()_+qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGHJKL:|ZXCVBNM<>?~`";
    std::stringstream ss;
    while (num != 0)
    {
        int digit = num % 91;
        num /= 91;
        ss << digits[digit];
    }

    return ss.str();
}
extern std::array<std::function<Node* (std::vector<Token>::iterator&)>, 6> nonTerminalRuleProductions;
Node* Parse(std::vector<Token>::iterator& iterator, RuleType type)
{
    int id = (int) type - (int) RuleType::FirstNonTerminalRule;
    if (id < 0)
        return nullptr;

    return nonTerminalRuleProductions[id](iterator);
}
std::array<std::function<Node* (std::vector<Token>::iterator&)>, 6> nonTerminalRuleProductions{
    // TerminalRule
    [](std::vector<Token>::iterator& iterator) {
        if (iterator->type == RuleType::StringLiteral)
        {
            Node** children = new Node * [1] {new Node{ iterator->type,nullptr,nullptr,0,&*iterator }};
            iterator++;
            return new Node{RuleType::TerminalRule, nullptr,children,1 };
        }
        return (Node*)nullptr;
    },
    // NonTerminalRule
    [](std::vector<Token>::iterator& iterator) {
        if (iterator->type == RuleType::SpacialCharacter && iterator->str != ";")
        {
            Node** children = new Node * [1] {new Node{ iterator->type,nullptr,nullptr,0,&*iterator }};
            iterator++;
            return new Node{ RuleType::NonTerminalRule, nullptr,children,1 };
        }
        if (iterator->type == RuleType::Identifire || iterator->type == RuleType::Keyword || iterator->type == RuleType::Operator)
        {
            Node** children = new Node * [1] {new Node{ iterator->type,nullptr,nullptr,0,&*iterator }};
            iterator++;
            return new Node{ RuleType::NonTerminalRule, nullptr,children,1 };
        }
        return (Node*)nullptr;
    },
    // RuleSequence 
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::TerminalRule);
        if (child == nullptr)
            child = Parse(iterator, RuleType::NonTerminalRule);
        if (child == nullptr)
            return (Node*)nullptr;
        Node* child1 = Parse(iterator, RuleType::RuleSequence);

        if (child && child1)
        {
            Node* parent = new Node{ RuleType::RuleSequence, nullptr, new Node * [2] {child,child1},2, nullptr };
            child->parent = parent;
            child1->parent = parent;
            return parent;
        }
        else
        {
            Node* parent = new Node{ RuleType::RuleSequence, nullptr, new Node * [1] {child},1, nullptr };
            child->parent = parent;
            return parent;
        }

    },
    // RuleProduction
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::RuleSequence);
        if (child == nullptr)
            return (Node*)nullptr;

        Node* child1 = nullptr;
        if (iterator->type == RuleType::Operator && iterator->str == "|")
        {
            auto start = iterator;
            iterator++;
            child1 = Parse(iterator, RuleType::RuleProduction);
            if (child1 == nullptr)
            {
                std::cerr << "Wrong";
                iterator = start;
                return (Node*)nullptr;
            }
        }

        if (child && child1)
        {
            Node* parent = new Node{ RuleType::RuleProduction, nullptr, new Node * [2] {child,child1},2, nullptr };
            child->parent = parent;
            child1->parent = parent;
            return parent;
        }
        else
        {
            Node* parent = new Node{ RuleType::RuleProduction, nullptr, new Node * [1] {child},1, nullptr };
            child->parent = parent;
            return parent;
        }

        return (Node*)nullptr;
    },
    // Rule
    [](std::vector<Token>::iterator& iterator) {
        auto startIt = iterator;
        Node* child = nullptr;
        if (iterator->type == RuleType::Keyword && iterator->str == "rule")
        {
            iterator++;
            if (iterator->type == RuleType::Identifire)
            {
                child = new Node{ RuleType::Identifire,nullptr,nullptr,0,&*iterator };
                iterator++;
            }
            else
            {
                std::cerr << "Wrong";
                return (Node*)nullptr;
            }
        }
        else
            return (Node*)nullptr;

        if (iterator->type == RuleType::Operator && iterator->str == "=")
        {
            iterator++;
        }
        else
        {
            iterator = startIt;
            return (Node*)nullptr;
        }

        Node* child1 = Parse(iterator, RuleType::RuleProduction);
        if (child1 == nullptr)
        {
            iterator = startIt;
            return (Node*)nullptr;
        }

        Node* parent = new Node{ RuleType::Rule, nullptr, new Node * [2] {child,child1},2, nullptr };
        child->parent = parent;
        child1->parent = parent;
        return parent;
    },
    // Start
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::Rule);
        if (child == nullptr)
            return (Node*)nullptr;

        Node* child1 = nullptr;
        if (iterator->type == RuleType::SpacialCharacter &&
            iterator->str == ";")
        {
            iterator++;
            child1 = Parse(iterator, RuleType::Start);
        }

        if (child && child1)
        {
            Node* parent = new Node{ RuleType::Start, nullptr, new Node * [2] {child,child1},2, nullptr };
            child->parent = parent;
            child1->parent = parent;
            return parent;
        }
        else
        {
            Node* parent = new Node{ RuleType::Start, nullptr, new Node * [1] {child},1, nullptr };
            child->parent = parent;
            return parent;
        }
    },
};