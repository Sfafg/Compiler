#pragma once

namespace test
{
    template <class Type>
    struct Node : public Type
    {
        Type type;
        int childrenCount;
        Node** children;
        Node* parent = nullptr;
    };

    Node<ID> id;
}