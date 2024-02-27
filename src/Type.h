#pragma once
#include <string_view>
#include <ostream>

#define EXTRA(ENUM_NAME)
#define ENTRY(ENUM_NAME, DISPLAY_NAME, PATTERN) 
#define PATTE(ENUM_NAME, DISPLAY_NAME, PATTERN)
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME) Node* FUNC_NAME(Tokenizer&);
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) Node* FUNC_NAME(Tokenizer&);
#define ENUM \
ENTRY(None               , "None"             ,                                                                                     )\
PATTE(KW_Rule            , "Rule"             , "rule"                                                                              )\
PATTE(SC_Lambda          , "Lambda"           , "λ"                                                                                 )\
PATTE(SC_Semicolon       , "Semicolon"        , ";"                                                                                 )\
PATTE(SC_LBracket        , "LBracket"         , "["                                                                                 )\
PATTE(SC_RBracket        , "RBracket"         , "]"                                                                                 )\
PATTE(SC_LParenthesis    , "LParenthesis"     , "("                                                                                 )\
PATTE(SC_RParenthesis    , "RParenthesis"     , ")"                                                                                 )\
PATTE(SC_LCurly          , "LCurly"           , "{"                                                                                 )\
PATTE(SC_RCurly          , "RCurly"           , "}"                                                                                 )\
PATTE(SC_BackSlash       , "BackSlash"        , "\\"                                                                                )\
PATTE(SC_Eof             , "Eof"              , "\0"                                                                                  )\
PATTE(OP_Assignment      , "Assignment"       , "="                                                                                 )\
PATTE(OP_Or              , "Or"               , "|"                                                                                 )\
FUPAT(TT_Identifire      , "Identifire"       , "[a-zA-Z_][a-zA-Z0-9_]*"    , TryParseTT_Identifire     , GenerateTT_Identifire     )\
FUPAT(TT_StringLiteral   , "StringLiteral"    , R"('(?:[^'\\]|\\[\s\S])*')" , TryParseTT_StringLiteral  , GenerateTT_StringLiteral  )\
FUPAT(TT_NumericLiteral  , "NumericLiteral"   , R"([0-9]+|[0-9]+\.[0-9]+)"  , TryParseTT_NumericLiteral , GenerateTT_NumericLiteral )\
FUNCT(TT_Keyword         , "Keyword"          , TryParseTT_Keyword          , GenerateTT_Keyword                                    )\
FUNCT(TT_SpecialCharacter, "SpecialCharacter" , TryParseTT_SpecialCharacter , GenerateTT_SpecialCharacter                           )\
FUNCT(TT_Operator        , "Operator"         , TryParseTT_Operator         , GenerateTT_Operator                                   )\
FUNCT(NT_Root            , "Root"             , TryParseNT_Root             , GenerateNT_Root                                       )\
FUNCT(NT_Rule            , "Rule"             , TryParseNT_Rule             , GenerateNT_Rule                                       )\
FUNCT(NT_Options         , "Options"          , TryParseNT_Options          , GenerateNT_Options                                    )\
FUNCT(NT_Sequence        , "Sequence"         , TryParseNT_Sequence         , GenerateNT_Sequence                                   )\
FUNCT(NT_GenerateCode, "GenerateCode", TryParseNT_GenerateCode, GenerateNT_GenerateCode)\
ENTRY(TokenType, "TokenType", )\
ENTRY(NodeType, "NodeType", )\
EXTRA(KW_First = KW_Rule)\
EXTRA(KW_Last = KW_Rule)\
EXTRA(SC_First = SC_Lambda)\
EXTRA(SC_Last = SC_Eof)\
EXTRA(OP_First = OP_Assignment)\
EXTRA(OP_Last = OP_Or)\
EXTRA(TT_First = TT_Identifire)\
EXTRA(TT_Last = TT_Operator)\
EXTRA(NT_First = NT_Root)\
EXTRA(NT_Last = NT_GenerateCode)\
EXTRA(FirstTerminal = KW_First)\
EXTRA(LastTerminal = TT_Last)\
EXTRA(FirstWithPattern = KW_First)\
EXTRA(LasttWithPattern = TT_NumericLiteral)\
EXTRA(FirstWithFunPtr = TT_Identifire)\
EXTRA(LastWithFunPtr = NT_GenerateCode)\

class Tokenizer;
class Node;

ENUM;

#undef FUNCT
#undef FUPAT
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME) std::string GENERATION_NAME( Node&);
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) std::string GENERATION_NAME( Node&);

ENUM;


#undef EXTRA
#undef ENTRY
#undef PATTE
#undef FUNCT
#undef FUPAT
#define EXTRA(ENUM_NAME) ENUM_NAME,
#define ENTRY(ENUM_NAME, DISPLAY_NAME, PATTERN) ENUM_NAME,
#define PATTE(ENUM_NAME, DISPLAY_NAME, PATTERN) ENUM_NAME,
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME) ENUM_NAME,
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) ENUM_NAME,

struct Type
{
    enum Value { ENUM };

    Type(int value = Type::None);

    Type T() const;
    bool IsTerminal() const;
    const char* Prefix() const;
    std::string_view Pattern() const;
    Node* (*ParsingFunc() const)(Tokenizer&);
    std::string(*GenerationFunc() const)(Node&);

    operator int& ();
    operator int() const;
    explicit operator const char* () const;
    operator std::string_view() const;

private:

#undef EXTRA
#undef ENTRY
#undef PATTE
#undef FUNCT
#undef FUPAT
#define EXTRA(ENUM_NAME)
#define ENTRY(ENUM_NAME, DISPLAY_NAME, PATTERN) 
#define PATTE(ENUM_NAME, DISPLAY_NAME, PATTERN)
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME)  FUNC_NAME,
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) FUNC_NAME,

    Value value;
    static const char* typeNames[];
    static std::string_view typePatterns[];
    inline static Node* (*parsingFunctions[])(Tokenizer&) = { ENUM };
#undef FUNCT
#undef FUPAT    
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME)  GENERATION_NAME,
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) GENERATION_NAME,
    inline static std::string(*generationFunctions[])(Node&) = { ENUM };
};
std::ostream& operator<<(std::ostream&, const Type&);

#undef EXTRA
#undef ENTRY
#undef PATTE
#undef FUNCT
#undef FUPAT
#define EXTRA(ENUM_NAME)
#define ENTRY(ENUM_NAME, DISPLAY_NAME, PATTERN) DISPLAY_NAME,
#define PATTE(ENUM_NAME, DISPLAY_NAME, PATTERN) DISPLAY_NAME,
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME) DISPLAY_NAME,
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) DISPLAY_NAME,

inline const char* Type::typeNames[]{ ENUM };

#undef EXTRA
#undef ENTRY
#undef PATTE
#undef FUNCT
#undef FUPAT
#define EXTRA(ENUM_NAME)
#define ENTRY(ENUM_NAME, DISPLAY_NAME, PATTERN) 
#define PATTE(ENUM_NAME, DISPLAY_NAME, PATTERN) PATTERN,
#define FUNCT(ENUM_NAME, DISPLAY_NAME, FUNC_NAME, GENERATION_NAME)
#define FUPAT(ENUM_NAME, DISPLAY_NAME, PATTERN, FUNC_NAME, GENERATION_NAME) PATTERN,

inline std::string_view Type::typePatterns[]{ ENUM };

#undef EXTRA
#undef ENTRY
#undef PATTE
#undef FUNCT
#undef ENUM