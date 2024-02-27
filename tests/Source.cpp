#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "SourceInput.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "SymbolTable.h"

void ProcessOptions(Node& node)
{
    if (node.type == Type::NT_Options && node.parent && node.parent->type == Type::NT_Sequence)
    {
        Node* parent = node.parent;
        int index = parent->Find(&node);
        node.parent->RemoveChild(index);

        for (int i = 1; i < node.GetChildren().size(); i++)
        {
            Node* cpy = parent->Copy();
            cpy->InsertChild(index, node.GetChildren()[i]);
            parent->parent->AddChild(cpy);
        }
        parent->InsertChild(index, node.GetChildren()[0]);

        return;
    }

    if (!node.IsLeaf())
        for (int i = 0; i < node.GetChildren().size(); i++)
        {
            ProcessOptions(*node.GetChildren()[i]);
        }
}

void SortSequences(Node& node)
{
    if (node.type == Type::NT_Options)
    {
        for (int i = 0; i < node.GetChildren().size(); i++)
        {
            int bestIndex = i;
            for (int j = i + 1; j < node.GetChildren().size(); j++)
            {
                if (node.GetChildren()[bestIndex]->GetChildren().size() < node.GetChildren()[j]->GetChildren().size())
                {
                    bestIndex = j;
                }
            }

            node.InsertChild(i, node.RemoveChild(bestIndex));
        }
        return;
    }

    if (!node.IsLeaf())
        for (int i = 0; i < node.GetChildren().size(); i++)
        {
            SortSequences(*node.GetChildren()[i]);
        }
}

int main()
{
    // Preprocessing
    SourceInput sourceInput("C:/Projekty/C++/Parser/tests/Src.txt");
    for (int i = 0; i < sourceInput.source.size(); i++)
        if (sourceInput.source[i] == '/' && sourceInput.source[i + 1] == '/')
        {
            int end = i;
            while (sourceInput.source[end] != '\n' && sourceInput.source[end] != 0) end++;
            sourceInput.source.erase(sourceInput.source.begin() + i, sourceInput.source.begin() + end);
        }
    std::ofstream("PreprocessorOutput.txt") << sourceInput.source;

    // Token analisys and parsing
    Tokenizer tk(&sourceInput, "tokens.txt");

    Node root(Type::NT_Root);
    while (!tk.eof)
    {
        Node* node = Node::TryParse(tk, Type::NT_Root);
        if (node)
        {
            root.AddChild(node);
            tk.Consume(tk.current);
        }
        else tk.Consume();
    }

    // Semantic analisys and tree preprocessing
    ProcessOptions(root);
    SortSequences(root);
    SymbolTable symbolTable(root);

    // Draw the tree.
    std::ofstream graphFile("syntax_tree.dot", std::ios_base::trunc);
    graphFile << "digraph SyntaxTree{\n\tgraph [bgcolor=black]\n\tnode[color = gray, fontcolor = gray, fontsize = \"60\", fontweight=\"bold\", penwidth = 5]\n\tedge[color = gray, penwidth = 5]\n" << root << "}";
    graphFile.close();

    // Code generation
    root.symbolTable = &symbolTable;
    std::ofstream("code.cpp") << root.GenerateCode();
}