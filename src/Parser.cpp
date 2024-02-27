#include <sstream>
#include "Parser.h"
#include "iostream"
#include "assert.h"
#include "SymbolTable.h"
#define ASSERT_LEAF(type) assert(type.IsTerminal())
#define ASSERT_NON_LEAF(type) assert(!type.IsTerminal())

Node::Node() :type(), data(nullptr), dataSize(0) {}

Node::Node(Type type) : type(type), data(nullptr), dataSize(0)
{
    if (!type.IsTerminal())children = new std::vector<Node*>();
}

Node::Node(Token token) : type(token.type), data(((std::string_view) token).data()), dataSize(token.size)
{
    ASSERT_LEAF(type);
}

Node::Node(Type type, std::vector<Node*>& children) : type(type), children(nullptr), dataSize(0)
{
    ASSERT_NON_LEAF(type);
    this->children = new std::vector<Node*>();
    AddChildren(children);
}
Node::Node(Type type, std::initializer_list<Node*> children) : type(type), children(nullptr), dataSize(0)
{
    ASSERT_NON_LEAF(type);
    if (!type.IsTerminal())this->children = new std::vector<Node*>();
    std::vector<Node*> c = children;
    AddChildren(c);
}

bool Node::IsLeaf() const
{
    return Type::FirstTerminal <= type && type <= Type::LastTerminal;
}

const std::vector<Node*>& Node::GetChildren() const
{
    ASSERT_NON_LEAF(type);
    return *children;
}
std::vector<Node*>& Node::GetChildren()
{
    ASSERT_NON_LEAF(type);
    return *children;
}

std::string_view Node::GetData() const
{
    ASSERT_LEAF(type);
    return std::string_view(data, dataSize);
}

Type Node::GetType() const
{
    ASSERT_LEAF(type);
    return type.T();
}

void Node::AddChildren(std::vector<Node*> nodes)
{
    ASSERT_NON_LEAF(type);

    for (int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->type != type)
        {
            nodes[i]->parent = this;
            children->push_back(nodes[i]);
        }
        else
        {
            for (auto&& child : nodes[i]->GetChildren())
            {
                child->parent = this;
                children->push_back(child);
            }
        }
    }
    dataSize = children->size();
}
Node* Node::AddChild(Node* node)
{
    if (node)
    {
        AddChildren({ node });
        return GetChildren()[GetChildren().size() - 1];
    }
    return nullptr;
}

void Node::RemoveChildren(int start, int count)
{
    ASSERT_NON_LEAF(type);
    assert(start + count <= dataSize);
    for (int i = start; i < start + count; i++) (*children)[i]->parent = nullptr;
    children->erase(children->begin() + start, children->begin() + start + count);
    dataSize = children->size();
}

Node* Node::RemoveChild(int index)
{
    ASSERT_NON_LEAF(type);
    Node* n = GetChildren()[index];
    RemoveChildren(index, 1);

    return n;
}

Node* Node::RemoveChild(Node* node)
{
    ASSERT_NON_LEAF(type);
    int i = Find(node);
    if (i != -1)
    {
        RemoveChildren(i, 1);
        return node;
    }

    return nullptr;
}

void Node::InsertChildren(int destination, std::vector<Node*> nodes)
{
    ASSERT_NON_LEAF(type);

    for (int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->type != type)
        {
            nodes[i]->parent = this;
        }
        else
        {
            Node* node = nodes[i];
            nodes.erase(nodes.begin() + i);
            for (auto&& child : node->GetChildren())
            {
                child->parent = this;
                nodes.push_back(child);
            }
        }
    }

    children->insert(children->begin() + destination, nodes.begin(), nodes.end());
    dataSize = children->size();
}
void Node::InsertChild(int destination, Node* node)
{
    ASSERT_NON_LEAF(type);
    InsertChildren(destination, { node });
}

void Node::Clear()
{
    ASSERT_NON_LEAF(type);
    children->clear();
    dataSize = 0;
}


int Node::Find(Node* node) const
{
    ASSERT_NON_LEAF(type);
    int i = 0;
    for (; i < GetChildren().size(); i++)
    {
        if (GetChildren()[i] == node)
        {
            return i;
        }
    }
    return -1;
}


Node* Node::Copy() const
{
    Node* node = new Node(*this);
    node->parent = nullptr;
    if (!IsLeaf())
    {
        node->data = nullptr;
        node->dataSize = 0;
        std::vector<Node*> childrenCopy;
        childrenCopy.reserve(this->dataSize);
        for (auto&& child : GetChildren())
            childrenCopy.push_back(child->Copy());
        node->AddChildren(childrenCopy);
    }
    return node;
}

Node::operator bool() const { return type != Type::None; }
bool Node::operator ==(const Type& rhs) const { return type == rhs; }

std::string Shorten(const Node& ptr);
std::ostream& operator<<(std::ostream& os, const Node& node)
{
    os << "\"" << Shorten(node) << "\" [label=\"";
    if (node.IsLeaf())
    {
        if (node.type == Type::TT_StringLiteral)
        {
            std::string_view data = node.GetData();
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
        else if (node.GetData() == "\"" || node.GetData() == "\\")return os << "\\n\\" << node.GetData() << "\"]\n";
        else return os << node.GetData() << "\"]\n";
    }
    else
    {
        os << node.type << "\"]\n";
        for (const auto& child : node.GetChildren()) os << "\"" << Shorten(node) << "\" -> \"" << Shorten(*child) << "\"\n";
        if (node.GetChildren().size() != 0)os << '\n';
        for (const auto& child : node.GetChildren())os << *child;
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
#pragma region // Parsing functions.
Node* TryParseTT_StringLiteral(Tokenizer& tk)
{
    if (tk[0].type == Type::TT_StringLiteral)
        return new Node(tk.Get());
    return nullptr;
}
Node* TryParseTT_NumericLiteral(Tokenizer& tk)
{
    if (tk[0].type == Type::TT_NumericLiteral)
        return new Node(tk.Get());
    return nullptr;
}
Node* TryParseTT_SpecialCharacter(Tokenizer& tk)
{
    if (tk[0].type.T() == Type::TT_SpecialCharacter)
        return new Node(tk.Get());
    return nullptr;
}
Node* TryParseTT_Identifire(Tokenizer& tk)
{
    if (tk[0].type == Type::TT_Identifire)
        return new Node(tk.Get());
    return nullptr;
}
Node* TryParseTT_Operator(Tokenizer& tk)
{
    if (tk[0].type.T() == Type::TT_Operator)
        return new Node(tk.Get());
    return nullptr;
}
Node* TryParseTT_Keyword(Tokenizer& tk)
{
    if (tk[0].type.T() == Type::TT_Keyword)
        return new Node(tk.Get());
    return nullptr;
}


Node* TryParseNT_Root(Tokenizer& tk)
{
    Node* n = TryParseNT_Rule(tk);
    if (n)return n;
    n = TryParseNT_GenerateCode(tk);
    if (n)return n;
    return n;
}

Node* TryParseNT_Rule(Tokenizer& tk)
{
    int c = tk.current;

    if (tk.Get() == Type::KW_Rule &&
        tk[0] == Type::TT_Identifire)
    {
        Node* a1 = new Node(tk.Get());
        if (tk.Get() == Type::OP_Assignment)
        {
            Node* a3 = TryParseNT_Options(tk);
            if (a3)
            {
                if (tk.Get() == Type::SC_Semicolon)
                    return new Node(Type::NT_Rule, { a1, a3 });
            }
        }
    }
    tk.current = c;

    return nullptr;
}

Node* TryParseNT_Options(Tokenizer& tk)
{
    Node* a0; Node* a1;

    int c = tk.current;
    if (a0 = TryParseNT_Sequence(tk))
    {
        int c1 = tk.current;
        if (tk.Get() == Type::OP_Or)
            if (a1 = TryParseNT_Options(tk))
                return new Node(Type::NT_Options, { a0, a1 });
        tk.current = c1;

        return new Node(Type::NT_Options, { a0 });
    }
    tk.current = c;

    return nullptr;
}

Node* TryParseNonOrOperator(Tokenizer& tk)
{
    int c = tk.current;
    Node* a;
    if ((a = TryParseTT_Operator(tk)) && a->type != Type::OP_Or)
        return a;
    tk.current = c;

    return nullptr;
}

Node* TryParseEscapedSpecialCharacterOrCharacter(Tokenizer& tk)
{
    int c = tk.current;
    Node* a;
    if (a = TryParseTT_SpecialCharacter(tk))
    {
        if (a->type == Type::SC_BackSlash)
        {
            Node* b;
            if ((b = TryParseTT_SpecialCharacter(tk)) || (b = TryParseTT_Operator(tk)))
                return b;
        }

        if (a->type != Type::SC_Semicolon && a->type != Type::SC_LBracket && a->type != Type::SC_RBracket)
            return a;
    }
    tk.current = c;

    return nullptr;
}

Node* TryParseNT_Sequence(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0, * a1, * a2;

    if ((a0 = TryParseTT_Keyword(tk)) ||
        (a0 = TryParseTT_Identifire(tk)) ||
        (a0 = TryParseTT_StringLiteral(tk)) ||
        (a0 = TryParseTT_NumericLiteral(tk)) ||
        (a0 = TryParseNonOrOperator(tk)) ||
        (a0 = TryParseEscapedSpecialCharacterOrCharacter(tk)))
    {
        int c1 = tk.current;
        if (tk.Get() == Type::SC_LBracket)
            if (a1 = TryParseNT_Options(tk))
                if (tk.Get() == Type::SC_RBracket)
                {
                    if (a2 = TryParseNT_Sequence(tk))
                        return new Node(Type::NT_Sequence, { a0, a1,a2 });

                    return new Node(Type::NT_Sequence, { a0, a1 });
                }
        tk.current = c1;

        if (a1 = TryParseNT_Sequence(tk))
            return new Node(Type::NT_Sequence, { a0, a1 });
        tk.current = c1;

        return new Node(Type::NT_Sequence, { a0 });
    }

    tk.current = c;
    if (tk.Get() == Type::SC_LBracket)
        if (a0 = TryParseNT_Options(tk))
            if (tk.Get() == Type::SC_RBracket)
            {
                if (a1 = TryParseNT_Sequence(tk))
                    return new Node(Type::NT_Sequence, { a0,a1 });

                return new Node(Type::NT_Sequence, { a0 });
            }
    tk.current = c;

    return nullptr;
}
Node* TryParseNT_GenerateCode(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0, * a1, * a2;
    if ((a0 = Node::TryParse(tk, Type::TT_Identifire)) &&
        tk.Get() == Type::SC_LParenthesis &&
        (a1 = Node::TryParse(tk, Type::TT_Identifire)) &&
        tk.Get() == Type::SC_RParenthesis &&
        tk.Get() == Type::SC_LCurly &&
        (a2 = Node::TryParse(tk, Type::TT_StringLiteral)) &&
        tk.Get() == Type::SC_RCurly)
    {
        return new Node(Type::NT_GenerateCode, { a0,a1,a2 });
    }
    tk.current = c;

    return nullptr;
}

#pragma endregion

#pragma region // Generating functions.
std::string GenerateTT_Keyword(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateTT_Identifire(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateTT_SpecialCharacter(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateTT_Operator(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateTT_StringLiteral(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateTT_NumericLiteral(Node& node)
{
    std::string code;
    code = node.GetData();
    return code;
}

std::string GenerateNT_Root(Node& node)
{
    std::string code;
    code = "#include \"../src/Parser.h\"\n#include \"../src/Tokenizer.h\"\n\n";
    for (auto&& child : node.GetChildren())
        code += child->GenerateCode();

    return code;
}

std::string GenerateNT_Rule(Node& node)
{
    std::string code;
    code = "Node* TryParseNT_" + node.GetChildren()[0]->GenerateCode() + "(Tokenizer& tk)\n{\nNode*ret;\n";
    code += node.GetChildren()[1]->GenerateCode();
    code += "\nreturn nullptr;\n}\n";
    return code;
}

std::string GenerateNT_Options(Node& node)
{
    std::string code;
    for (int i = 0; i < node.GetChildren().size(); i++)
    {
        code += "\nif(" + node.GetChildren()[i]->GenerateCode() + ")\nreturn ret;\n\nret->Clear();\n";
    }
    return code;
}

std::string Symbol(Node& node, int index)
{
    std::string code;

    if (node.type.T() == Type::NodeType)
    {
        code = node.GenerateCode();
    }
    else if (node.type.T() == Type::TokenType)
    {
        if (node.type == Type::TT_SpecialCharacter || node.type == Type::TT_Keyword)
        {
            code += "TryParseTT_";
            code += node.type;
            code += "(tk)";
        }
        else if (node.type == Type::TT_Identifire && (*node.symbolTable)[node.GetData()] == IDType::Rule)
        {
            code += "ret->AddChild( TryParseNT_";
            code += node.GetData();
            code += "(tk))";
        }
        else if (node.type == Type::TT_Identifire && (*node.symbolTable)[node.GetData()] == IDType::TokenType)
        {
            code += "ret->AddChild( TryParseTT_";
            code += node.GetData();
            code += "(tk))";
        }
        else
        {
            code += "ret->AddChild( TryParseTT_";
            code += node.type;
            code += "(tk))";
        }
    }
    else
    {
        code += "(";
        code += "TryParseTT_";
        code += node.type.T();
        code += "(tk)->type == Type::";
        code += node.type.Prefix();
        code += "_";
        code += node.type;
        code += ")";
    }

    return code;
}

std::string GenerateNT_Sequence(Node& node)
{
    std::string code;

    for (int i = 0; i < node.GetChildren().size() - 1; i++)
    {
        code += Symbol(*node.GetChildren()[i], i) + " &&\n";
    }
    code += Symbol(*node.GetChildren()[node.GetChildren().size() - 1], node.GetChildren().size() - 1);

    return code;
}

std::string GenerateNT_GenerateCode(Node& node)
{
    std::string code;
    return code;
}

std::string GenerateNT_Symbol(Node& node) { return ""; }
std::string GenerateNT_EscapedOperator(Node& node) { return ""; }
std::string GenerateNT_EscapedCharacter(Node& node) { return ""; }

#pragma endregion

Node* Node::TryParse(Tokenizer& tk, Type type)
{
    Type t = type;
    if (t < Type::TT_First) t = t.T();
    return t.ParsingFunc()(tk);
}

std::string Node::GenerateCode()
{
    Type t = type;
    if (t < Type::TT_First) t = t.T();
    return t.GenerationFunc()(*this);
}