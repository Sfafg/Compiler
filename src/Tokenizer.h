#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include "Type.h"

struct Token
{
    Token(Type type, const char* pattern);

    Token& operator =(const std::string_view& rhs);
    operator std::string_view() const;
    bool operator ==(const Type& rhs) const;

    Type type;
    int size;

private:
    const char* begin;
};

class SourceInput;
class Tokenizer
{
    SourceInput* source;
    std::vector<Type> sortedPatterns;
    std::vector<Token> tokenBuffer;
    std::ofstream debugOutputFile;

public:
    int current;
    bool eof;

    Tokenizer(SourceInput* source, const char* tokenizerOutputPath);

    Token Consume(int count = 1);
    Token& Get();
    Token& operator [](int index);
};