#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Tokenizer.h"

// Zasady tworzenia drzewa z zasady:
// 1. Żadne znaki specjalne nie idą na drzewo.
// 2. Słowa kluczowe ida na drzewo tylko jeżeli są różne ich opcje.
// 3. Identyfikatory i literały zawsze są dodawane na drzewo.
// 4. Zasady zawsze są dodawane do drzewa.

std::string ToBase62(unsigned long long int num)
{
    static const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghiklmnopqrstuvxyz_";
    std::stringstream ss;
    while (num != 0)
    {
        int digit = num % 61;
        num /= 61;
        ss << digits[digit];
    }

    return ss.str();
}

class Node
{
public:
    Node(std::string name, Node* parent) :name(name), parent(parent), children({}), token(Token(Token::Type::None, "", ""))
    {
        for (const auto& child : children) child->parent = this;
    }
    Node(std::string name, std::initializer_list<Node*> children = {}, Node* parent = nullptr) :name(name), parent(parent), children(children), token(Token(Token::Type::None, "", ""))
    {
        for (const auto& child : children) child->parent = this;
    }
    Node(std::string name, Token token, std::initializer_list<Node*> children = {}, Node* parent = nullptr) : name(name), parent(parent), children(children), token(token)
    {
        for (const auto& child : children) child->parent = this;
    }

    void AddChild(Node* node)
    {
        node->parent = this;
        children.push_back(node);
    }
    void RemoveChild(Node* node)
    {
        node->parent = nullptr;
        int i = 0;
        for (; i < GetChildren().size(); i++) if (children[i] == node) break;
        children.erase(children.begin() + i);
    }
    const std::vector<Node*>& GetChildren() const
    {
        return children;
    }
    std::vector<Node*>& GetChildren()
    {
        return children;
    }

    void GenerateCode(std::ostream& os) const
    {
        if (name == "Root")
        {
            for (const auto& child : children)
                if (child->name == "Rule") os << "Node* TryParse" << child->token.value << "(Tokenizer& tk);\n";
            for (const auto& child : children)
                child->GenerateCode(os);
        }
        else if (name == "Rule")
        {
            os << "Node* TryParse" << token.value << "(Tokenizer& tk){\n int c = tk.current;\n";
            for (int i = 0; i < children[0]->GetChildren().size(); i++)
            {
                os << "Node* a" << i << ";";
            }
            os << "\n\n";

            for (const auto& child : children)
            {
                child->GenerateCode(os);
            }
            os << "return nullptr;\n}\n";
        }
        else if (name == "Production")
        {
            for (int i = 0; i < children.size(); i++)
            {
                std::stringstream ss;
                children[i]->GenerateCode(ss);
                os << "a" << i << " = " << ss.rdbuf() << ";\nif(a" << i << "!= nullptr){";
            }
            os << "return new Node(\"" << parent->token.value << "\",{";

            bool wasLastPrinted = children[0]->token.type != Token::Type::SpecialCharacter && children[0]->token.type != Token::Type::SpecialCharacter;
            if (wasLastPrinted) os << "a" << 0;
            for (int i = 1; i < children.size(); i++)
            {
                if (wasLastPrinted) os << ", ";
                wasLastPrinted = children[i]->token.type != Token::Type::SpecialCharacter && children[i]->token.type != Token::Type::SpecialCharacter;
                if (wasLastPrinted) os << "a" << i;
            }
            os << "});\n";
            for (int i = 0; i < children.size(); i++) os << "}\n";
            os << "tk.current = c;\n\n";
        }
        else if (name == "Symbol")
        {
            bool isRule = false;
            Node* root = parent;
            switch (token.type)
            {
            case Token::Type::Keyword:
            case Token::Type::Operator:
            case Token::Type::SpecialCharacter:
                os << "(tk.Peek().type == Token::Type::" << token.type << " && tk.Peek().key == \"" << token.key << "\") ? new Node(\"" << token.type << "\", tk.GetToken()) : nullptr";
                break;
            case Token::Type::Identifire:
                while (root->parent != nullptr) root = root->parent;
                for (auto&& child : root->GetChildren())
                {
                    if (child->name == "Rule" && child->token.value == token.value)
                    {
                        isRule = true;
                        break;
                    }
                }
                if (isRule || token.value == "Rule") os << "TryParse" << token.value << "(tk)";
                else os << "(tk.Peek().type == Token::Type::Identifire) ? new Node(\"" << token.key << "\", tk.GetToken()) : nullptr";
                break;
            case Token::Type::Literal:
                os << "(tk.Peek().type == Token::Type::Literal) ? new Node(\"" << (token.key[0] == '\"' ? token.key.substr(1, token.key.size() - 2) : token.key) << "\", tk.GetToken()) : nullptr";
                break;
            default: break;
            }
        }
    }

    std::string name;
    Node* parent;
    Token token;

private:
    std::vector<Node*> children;
};
std::ostream& operator<<(std::ostream& os, const Node& node)
{
    os << "\t\"" << ToBase62((unsigned long long) & node) << "\" [label=\"";
    if (node.name != "Symbol")
    {
        os << node.name;
        if (node.token.type != Token::Type::None) os << "\\n";
    }
    if (node.token.type != Token::Type::None)
    {
        os << node.token.value;
    }
    os << "\"]\n";

    for (const auto& child : node.GetChildren())
        os << "\t\"" << ToBase62((unsigned long long) & node) << "\" -> \"" << ToBase62((unsigned long long) child) << "\"\n";
    if (node.GetChildren().size() != 0)os << '\n';
    for (const auto& child : node.GetChildren())
        os << *child;

    return os;
}

Node* ParseProduction(Tokenizer& tk)
{
    Node* production = new Node("Production");
    while (tk.Peek().key != "Or" && tk.Peek().key != "Semicolon" && tk.Peek().key != "EOF")
    {
        production->AddChild(new Node("Symbol", tk.GetToken()));
    }
    if (production->GetChildren().size() == 0)
    {
        delete production;
        return nullptr;
    }
    return production;
}
Node* TryParseRule(Tokenizer& tk)
{
    if (tk.GetToken().key == "Rule")
    {
        Node* ruleNode = new Node("Rule");

        if (tk.Peek().type != Token::Type::Identifire)
        {
            std::cerr << "ERROR: Expected Identifire at " << tk.GetToken().value << "";
            return nullptr;
        }
        ruleNode->token = tk.GetToken();

        if (tk.Peek().key != "Assignment")
        {
            std::cerr << "ERROR: Expected Assignment Operator at " << tk.GetToken().value << "";
            return nullptr;
        }
        tk.GetToken();

        Node* production = ParseProduction(tk);
        if (production == nullptr)
        {
            std::cerr << "ERROR: Expected at least one production at " << tk.GetToken().value;
            return nullptr;
        }
        ruleNode->AddChild(production);
        while (tk.GetToken().key != "Semicolon")
        {
            production = ParseProduction(tk);
            ruleNode->AddChild(production);
        }

        return ruleNode;
    }
    tk.Revert();
    return nullptr;
}
#define EXP
#ifdef EXP
#include "C:\Projekty\C++\Parser\build\code.cpp"
#endif
Node* Parse(Tokenizer& tk)
{
#ifdef EXP
    Node* node = TryParseRoot(tk);
#else
    Node* node = TryParseRule(tk);
#endif
    tk.Consume(tk.current);
    return node;
}

int main()
{
    Source source("C:/Projekty/C++/Parser/tests/Src.txt");
    Tokenizer tk(
        &source,
        {
            {Token::Type::Keyword, "Int", "int"},
            {Token::Type::Keyword, "Float", "float"},
            {Token::Type::Keyword, "Char", "char"},
            {Token::Type::Keyword, "String", "string"},
            {Token::Type::Keyword, "Rule", "rule"},
            {Token::Type::Keyword, "Rule1", "rule1"},
            {Token::Type::Keyword, "OrWord", "or"},
            {Token::Type::Keyword, "Lambda", "λ"},
            {Token::Type::SpecialCharacter, "Semicolon", ";"},
            {Token::Type::SpecialCharacter, "LBracket", "["},
            {Token::Type::SpecialCharacter, "RBracket", "]"},
            {Token::Type::SpecialCharacter, "LCurly", "{"},
            {Token::Type::SpecialCharacter, "RCurly", "}"},
            {Token::Type::SpecialCharacter, "LParenthesis", "("},
            {Token::Type::SpecialCharacter, "RParenthesis", ")"},
            {Token::Type::SpecialCharacter, "Comma", ","},
            {Token::Type::SpecialCharacter, "EOF", ""},
            {Token::Type::Operator, "Assignment", "="},
            {Token::Type::Operator, "Add", "+"},
            {Token::Type::Operator, "Subtract", "-"},
            {Token::Type::Operator, "Multiply", "*"},
            {Token::Type::Operator, "Divide", "/"},
            {Token::Type::Operator, "Or", "|"},
            {Token::Type::Operator, "And", "&"},
        },
        "tokens.txt"
        );

    Node root("Root");
    while (!tk.eof)
    {
        Node* node = Parse(tk);
        if (node != nullptr)
        {
            if (node->name == "Root")
                for (auto&& child : node->GetChildren())
                    root.AddChild(child);
            else root.AddChild(node);
        }
        else tk.Consume();
    }

    std::ofstream outputCode("code.cpp");
    root.GenerateCode(outputCode);

    std::ofstream graphFile("syntax_tree.dot", std::ios_base::trunc);
    graphFile << "digraph SyntaxTree{\n\tgraph [bgcolor=black]\n\tnode[color = gray, fontcolor = gray, fontsize = \"60\", fontweight=\"bold\", penwidth = 5]\n\tedge[color = gray, penwidth = 5]\n" << root << "}";
    graphFile.close();
}