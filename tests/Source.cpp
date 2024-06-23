#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Token.h"

std::string ReadSourceCode(const char* sourceFilePath)
{
    // Open file containing source code
    std::ifstream sourceFile(sourceFilePath);
    if (!sourceFile.is_open())
    {
        std::cerr << "Failed to open file";
        return std::string();
    }

    // Return string containing the whole file.
    std::stringstream ss;
    ss << sourceFile.rdbuf();
    return ss.str();
}
int main()
{
    // Read Source Code from file.
    std::string sourceCodeText = ReadSourceCode("../tests/Src.txt");
    std::string::iterator sourceIterator = sourceCodeText.begin();
    std::string::iterator sourceEnd = sourceCodeText.end();

    // Convert Source Code into an array of tokens.
    std::vector<Token> tokens;

    Token newToken;
    do
    {
        std::string::iterator copy = sourceIterator;
        newToken = Token(&sourceIterator, sourceEnd);
        tokens.push_back(newToken);

        std::cout << newToken << " ";
        for (; copy != sourceIterator; copy++)
            if (*copy == '\n')
                std::cout << '\n';
    }
    while (newToken.type != RuleType::None);
}