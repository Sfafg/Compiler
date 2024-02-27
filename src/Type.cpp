#include "Type.h"

Type::Type(int value) :value((Value) value) {}

Type Type::T() const
{
    if (Type::KW_First <= value && value <= Type::KW_Last) return Type::TT_Keyword;
    if (Type::SC_First <= value && value <= Type::SC_Last) return Type::TT_SpecialCharacter;
    if (Type::OP_First <= value && value <= Type::OP_Last) return Type::TT_Operator;
    if (Type::TT_First <= value && value <= Type::TT_Last) return Type::TokenType;
    if (Type::NT_First <= value && value <= Type::NT_Last) return Type::NodeType;
    return Type::None;
}

bool Type::IsTerminal() const
{
    return Type::FirstTerminal <= value && value <= Type::LastTerminal;
}

const char* Type::Prefix() const
{
    switch (T())
    {
    case Type::TT_Keyword: return "KW";
    case Type::TT_SpecialCharacter: return "SC";
    case Type::TT_Operator: return "OP";
    case Type::TokenType: return "TT";
    case Type::NodeType: return "NT";
    default: return "";
    }
}
std::string_view Type::Pattern() const
{
    if (!(Type::FirstWithPattern <= value && value <= Type::LasttWithPattern)) return "";
    return Type::typePatterns[value - Type::FirstWithPattern];
}

Node* (*Type::ParsingFunc() const)(Tokenizer&)
{
    if (!(Type::FirstWithFunPtr <= value && value <= Type::LastWithFunPtr)) return nullptr;
    return Type::parsingFunctions[value - Type::FirstWithFunPtr];
}
std::string(*Type::GenerationFunc() const)(Node&)
{
    if (!(Type::FirstWithFunPtr <= value && value <= Type::LastWithFunPtr)) return nullptr;
    return Type::generationFunctions[value - Type::FirstWithFunPtr];
}

Type::operator int& () { return *(int*) &value; }
Type::operator int() const { return value; }
Type::operator const char* () const { return typeNames[value]; }
Type::operator std::string_view() const { return typeNames[value]; }

std::ostream& operator<<(std::ostream& os, const Type& t)
{
    return os << (const char*) t;
}