#include <iostream>
#include <fstream>
#include <sstream>

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
    std::cout << sourceCodeText;
}
