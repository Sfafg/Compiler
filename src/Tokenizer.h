#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Source.h"

struct Token
{
    enum class Type
    {
        None = 0, Keyword, Identifire, SpecialCharacter, Operator, Literal
    };

    Type type;
    std::string_view key;
    std::string_view value;
};
std::ostream& operator<<(std::ostream& os, const Token::Type& type)
{
    switch (type)
    {
    case Token::Type::Keyword:return os << "Keyword";
    case Token::Type::Identifire:return os << "Identifire";
    case Token::Type::SpecialCharacter:return os << "SpecialCharacter";
    case Token::Type::Operator:return os << "Operator";
    case Token::Type::Literal:return os << "Literal";
    default: return os;
    }
}

class Tokenizer
{
    Source* source;
    std::vector<Token> tokenBuffer;
    std::ofstream tokenizerOutputFile;
public:
    std::vector<Token> matchInfos;
    int current;
    bool eof;

    Tokenizer(Source* source, const std::initializer_list<Token>& matchInfos, const char* tokenizerOutputPath)
        :source(source), matchInfos(matchInfos), current(0), tokenizerOutputFile(tokenizerOutputPath), eof(false)
    {
        std::sort(this->matchInfos.begin(), this->matchInfos.end(), [](auto& a, auto& b) {return a.value.size() > b.value.size(); });
    }

    std::vector<Token>& GetBuffer()
    {
        return tokenBuffer;
    }

    void Revert()
    {
        current = 0;
    }

    Token Consume(int count = 1)
    {
        while (tokenBuffer.size() < count) GetToken();
        Token token = tokenBuffer[count - 1];
        tokenBuffer.erase(tokenBuffer.begin(), tokenBuffer.begin() + count);
        current -= count;
        if (current < 0)current = 0;

        return token;
    }
    Token ConsumeLast(int count = 1)
    {
        while (tokenBuffer.size() < count) GetToken();
        Token token = tokenBuffer[tokenBuffer.size() - count];
        tokenBuffer.erase(tokenBuffer.end() - count, tokenBuffer.end());
        if (current <= tokenBuffer.size() && current != 0)current = tokenBuffer.size() - 1;

        return token;
    }

    Token Prev()
    {
        current--;
        if (current < 0) current = 0;
        return tokenBuffer[current];
    }
    Token GetToken()
    {
        if (current < tokenBuffer.size())
        {
            Token token = tokenBuffer[current];
            current++;
            return token;
        }
        Token matchInfo(Token::Type::None, "", "");
        for (int i = 0; i < matchInfos.size(); i++)
        {
            if (source->Match(matchInfos[i].value) != source->current)
            {
                matchInfo = matchInfos[i];
                break;
            }
        }

        auto end = source->Match(std::regex("[a-zA-Z_][a-zA-Z0-9_]*"));
        if ((end - source->current) > matchInfo.value.size())
        {
            matchInfo.key = std::string_view(source->current, end);
            matchInfo.value = matchInfo.key;
            matchInfo.type = Token::Type::Identifire;
        }

        end = source->Match(std::regex(R"("(?:[^"\\]|\\[\s\S])*"|'(?:[^'\\]|\\[\s\S])*'|[0-9]+(?:\.[0-9]+|))"));
        if ((end - source->current) > matchInfo.value.size())
        {
            matchInfo.key = std::string_view(source->current, end);
            matchInfo.value = matchInfo.key;
            matchInfo.type = Token::Type::Literal;
        }

        if (matchInfo.type == Token::Type::None)
        {
            if (*source->current == '\n')tokenizerOutputFile << '\n';
            source->current++;
            return GetToken();
        }
        matchInfo.value = std::string_view(source->current, source->current + matchInfo.value.size());
        source->current += matchInfo.value.size();
        tokenBuffer.push_back(matchInfo);
        current++;

        if (matchInfo.key == "EOF") eof = true;
        tokenizerOutputFile << "<" << matchInfo.type << ": " << matchInfo.key << ">";
        return matchInfo;
    }

    Token Peek()
    {
        Token token = GetToken();
        current--;
        return token;
    }
};