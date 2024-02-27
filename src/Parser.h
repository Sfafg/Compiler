#pragma once
#include <initializer_list>
#include <string_view>
#include <vector>
#include "Type.h"
#include "Tokenizer.h"

class SymbolTable;
class Node
{
public:
    Node();
    Node(Type type);
    Node(Token data);
    Node(Type type, std::vector<Node*>& children);
    Node(Type type, std::initializer_list<Node*> children);

    bool IsLeaf() const;
    const std::vector<Node*>& GetChildren() const;
    std::vector<Node*>& GetChildren();
    std::string_view GetData() const;
    Type GetType() const;

    void AddChildren(std::vector<Node*> nodes);
    Node* AddChild(Node* node);
    void RemoveChildren(int start, int count);
    Node* RemoveChild(int index);
    Node* RemoveChild(Node* node);
    void InsertChildren(int destination, std::vector<Node*> nodes);
    void InsertChild(int destination, Node* node);
    void  Clear();
    int Find(Node* node) const;
    Node* Copy() const;

    static Node* TryParse(Tokenizer& tk, Type type);
    std::string GenerateCode();

public:
    operator bool() const;
    bool operator ==(const Type& rhs) const;

public:
    static SymbolTable* symbolTable;
    Type type;

private:
    int dataSize;
    union
    {
        std::vector<Node*>* children;
        const char* data;
    };
public:
    Node* parent = nullptr;
};
inline SymbolTable* Node::symbolTable = nullptr;
std::ostream& operator<<(std::ostream& os, const Node& node);