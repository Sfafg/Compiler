#pragma once
#include "Token.h"
#include <sstream>
struct Node
{
    RuleType type = RuleType::None;
    Node* parent = nullptr;
    Node** children = nullptr;
    int childrenCount = 0;
    Token* token = nullptr;

    Node(RuleType type, const std::vector<Node*>& children) : type(type), children(new Node* [children.size()]), childrenCount(children.size())
    {
        for (int i = 0; i < childrenCount; i++)
        {
            this->children[i] = children[i];
            this->children[i]->parent = this;
        }
    }
    Node(Token* token) :type(token->type), token(token) {}
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

extern std::array<std::function<Node* (std::vector<Token>::iterator&)>, 8> nonTerminalRuleProductions;
Node* Parse(std::vector<Token>::iterator& iterator, RuleType type)
{
    int id = (int) type - (int) RuleType::FirstNonTerminalRule;
    if (id < 0)
    {
        if (iterator->type == type)
        {
            Node* node = new Node(&*iterator);
            iterator++;
            return node;
        }
        return nullptr;
    }

    return nonTerminalRuleProductions[id](iterator);
}
Node* Parse(std::vector<Token>::iterator& iterator, RuleType type, const char* str)
{
    int id = (int) type - (int) RuleType::FirstNonTerminalRule;
    if (id < 0)
    {
        if (iterator->type == type && iterator->str == str)
        {
            Node* node = new Node(&*iterator);
            iterator++;
            return node;
        }
        return nullptr;
    }

    return nullptr;
}
std::array<std::function<Node* (std::vector<Token>::iterator&)>, 8> nonTerminalRuleProductions{
    // TerminalRule
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::StringLiteral);
        if (child == nullptr)
            return (Node*)nullptr;

        return new Node(RuleType::TerminalRule,{child});
    },
    // NonTerminalRule
    [](std::vector<Token>::iterator& iterator) {
        if (iterator->type == RuleType::SpacialCharacter && iterator->str != ";")
        {
            Node* child = new Node(&*iterator);
            iterator++;
            return new Node{ RuleType::NonTerminalRule,{child} };
        }
        if (iterator->type == RuleType::Identifire || iterator->type == RuleType::Keyword || iterator->type == RuleType::Operator)
        {
            Node* child = new Node(&*iterator);
            iterator++;
            return new Node{ RuleType::NonTerminalRule,{child} };
        }
        return (Node*)nullptr;
    },
    // LabeledSymbol
    [](std::vector<Token>::iterator& iterator) {

        Node* child = Parse(iterator, RuleType::Identifire);
        if (!child)
            return (Node*)nullptr;

        iterator++;

        if (iterator->type != RuleType::SpacialCharacter || iterator->str != ":")
        {
            iterator--;
            iterator--;
            return (Node*)nullptr;
        }
        iterator++;

        Node* child1 = Parse(iterator, RuleType::Identifire);
        if (!child1)
        {
            iterator -= 3;
            return (Node*)nullptr;
        }
        iterator++;

        return new Node{ RuleType::LabeledSymbol, {child,child1} };
    },
    // RuleToken
    [](std::vector<Token>::iterator& iterator) {

        Node* child = Parse(iterator, RuleType::LabeledSymbol);

        if (child == nullptr)
            child = Parse(iterator, RuleType::TerminalRule);

        if (child == nullptr)
            child = Parse(iterator, RuleType::NonTerminalRule);

        if (child == nullptr)
            return (Node*)nullptr;

        return new Node{ RuleType::RuleToken,{child} };
    },
    // RuleSequence 
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::RuleToken);

        if (child == nullptr)
            return (Node*)nullptr;

        Node* child1 = Parse(iterator, RuleType::RuleSequence);

        if (child && child1)
            return new Node(RuleType::RuleSequence, {child,child1});
        else
            return new Node(RuleType::RuleSequence,{child});
    },
    // RuleProduction
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::RuleSequence);
        if (child == nullptr)
            return (Node*)nullptr;

        Node* child1 = nullptr;
        auto start = iterator;
        if (Parse(iterator, RuleType::Operator, "|"))
        {
            child1 = Parse(iterator, RuleType::RuleProduction);
            if (child1 == nullptr)
            {
                std::cerr << "Wrong";
                iterator = start;
                return (Node*)nullptr;
            }
        }

        if (child && child1)
            return new Node(RuleType::RuleProduction, {child,child1});

        else
            return new Node(RuleType::RuleProduction, {child});
    },
    // Rule
    [](std::vector<Token>::iterator& iterator) {
        auto startIt = iterator;
        Node* child = nullptr;
        if (Parse(iterator,RuleType::Keyword,"rule"))
        {
            child = Parse(iterator, RuleType::Identifire);
            if (!child)
            {
                std::cerr << "Wrong";
                return (Node*)nullptr;
            }
        }
        else
            return (Node*)nullptr;

        if (!Parse(iterator,RuleType::Operator, "="))
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

        return new Node(RuleType::Rule, {child,child1});
    },
    // Start
    [](std::vector<Token>::iterator& iterator) {
        Node* child = Parse(iterator,RuleType::Rule);
        if (child == nullptr)
            return (Node*)nullptr;

        Node* child1 = nullptr;
        if (Parse(iterator, RuleType::SpacialCharacter, ";"))
            child1 = Parse(iterator, RuleType::Start);

        if (child && child1)
            return new Node(RuleType::Start, {child,child1});

        else
            return new Node(RuleType::Start, {child});
    },
};