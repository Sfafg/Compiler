#pragma once
#include <string_view>
#include <map>
#include "Parser.h"

enum class IDType
{
    None = 0, Rule, TokenType
};

class SymbolTable
{
public:
    SymbolTable(const Node& node)
    {
        Populate(node);
    }

public:
    auto operator [](std::string_view key)
    {
        if (map.contains(key))
            return map[key];

        return IDType::None;
    }
private:
    void Populate(const Node& node)
    {
        if (node.IsLeaf())
        {
            if (node.type == Type::TT_Identifire)
            {
                if (node.parent->type == Type::NT_Rule) map[node.GetData()] = IDType::Rule;
                else
                {
                    for (Type i = Type::TT_First; i <= Type::TT_Last; i++)
                    {
                        if (node.GetData() == i)
                        {
                            map[node.GetData()] = IDType::TokenType;
                            break;
                        }
                    }
                }
            }
            return;
        }
        for (auto&& child : node.GetChildren())
        {
            Populate(*child);
        }
    }
    std::map<std::string_view, IDType> map;
};