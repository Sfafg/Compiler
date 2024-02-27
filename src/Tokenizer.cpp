#include "SourceInput.h"
#include "Tokenizer.h"

Token::Token(Type type, const char* string) :type(type), size(strlen(string)), begin((char*) string) {}

Token& Token::operator =(const std::string_view& rhs)
{
    begin = rhs.data();
    size = rhs.size();
    return *this;
}
Token::operator std::string_view() const
{
    return std::string_view(begin, size);
}

bool Token::operator ==(const Type& rhs) const { return type == rhs; }



Tokenizer::Tokenizer(SourceInput* source, const char* tokenizerOutputPath) :
    source(source),
    current(0),
    debugOutputFile(tokenizerOutputPath),
    eof(false)
{
    for (Type i = Type::FirstTerminal; i <= Type::LasttWithPattern && i.T() != Type::TokenType; i++) sortedPatterns.push_back(i);
    std::sort(this->sortedPatterns.begin(), this->sortedPatterns.end(), [](auto& a, auto& b) {return a.Pattern().size() > b.Pattern().size(); });
}

Token Tokenizer::Consume(int count)
{
    while (tokenBuffer.size() < count) Get();
    Token token = tokenBuffer[count - 1];
    tokenBuffer.erase(tokenBuffer.begin(), tokenBuffer.begin() + count);
    current -= count;
    if (current < 0)current = 0;

    return token;
}

Token& Tokenizer::Get()
{
    current++;
    if (current - 1 < tokenBuffer.size()) return tokenBuffer[current - 1];

    Token matchInfo(Type::None, "");
    while (true)
    {
        for (const Type& pattern : sortedPatterns)
        {
            int matchSize = source->Match(pattern.Pattern());
            if (matchSize > matchInfo.size)
            {
                matchInfo.type = pattern;
                if (source->current == source->source.end())  matchInfo = std::string_view(source->current, source->current);
                else matchInfo = std::string_view(source->current, source->current + matchSize);
                break;
            }
        }

        auto f = [&](Type t) {
            int matchSize = source->MatchRegex(t.Pattern());
            if (matchSize > matchInfo.size)
            {
                matchInfo.type = t;
                matchInfo = std::string_view(source->current, source->current + matchSize);
            }
            };
        f(Type::TT_Identifire);
        f(Type::TT_StringLiteral);
        f(Type::TT_NumericLiteral);

        if (matchInfo.type == Type::None)
        {
            if (*source->current == '\n')debugOutputFile << '\n';
            source->current++;
        }
        else break;
    }

    source->current += matchInfo.size;
    tokenBuffer.push_back(matchInfo);

    if (matchInfo.type == Type::SC_Eof) eof = true;
    debugOutputFile << "<" << matchInfo.type.T() << ": " << ((matchInfo.type.T() == Type::TokenType) ? (std::string_view) matchInfo : (std::string_view) matchInfo.type) << ">";
    return tokenBuffer.begin()[tokenBuffer.size() - 1];
}

Token& Tokenizer::operator[](int index)
{
    while (index + current >= tokenBuffer.size())
    {
        Token& token = Get();
        current--;
        if (token == Type::SC_Eof)
            return token;
    }

    return tokenBuffer[current + index];
}