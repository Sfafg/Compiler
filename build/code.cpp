Node* TryParseRoot(Tokenizer& tk);
Node* TryParseProduction(Tokenizer& tk);
Node* TryParseProductions(Tokenizer& tk);
Node* TryParseRule1(Tokenizer& tk);
Node* TryParseRoot(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0;

    a0 = TryParseRule(tk);
    if (a0 != nullptr)
    {
        return new Node("Root", { a0 });
    }
    tk.current = c;

    a0 = TryParseRule1(tk);
    if (a0 != nullptr)
    {
        return new Node("Root", { a0 });
    }
    tk.current = c;

    a0 = (tk.Peek().type == Token::Type::Identifire) ? new Node("ID", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        return new Node("Root", { a0 });
    }
    tk.current = c;

    return nullptr;
}
Node* TryParseProduction(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0; Node* a1;

    a0 = (tk.Peek().type == Token::Type::Identifire) ? new Node("Symbol", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        a1 = TryParseProductions(tk);
        if (a1 != nullptr)
        {
            return new Node("Production", { a0, a1 });
        }
    }
    tk.current = c;

    a0 = (tk.Peek().type == Token::Type::Identifire) ? new Node("Symbol", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        return new Node("Production", { a0 });
    }
    tk.current = c;

    a0 = (tk.Peek().type == Token::Type::Keyword && tk.Peek().key == "Lambda") ? new Node("Keyword", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        return new Node("Production", { a0 });
    }
    tk.current = c;

    return nullptr;
}
Node* TryParseProductions(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0; Node* a1; Node* a2;

    a0 = TryParseProduction(tk);
    if (a0 != nullptr)
    {
        a1 = (tk.Peek().type == Token::Type::Keyword && tk.Peek().key == "OrWord") ? new Node("Keyword", tk.GetToken()) : nullptr;
        if (a1 != nullptr)
        {
            a2 = TryParseProductions(tk);
            if (a2 != nullptr)
            {
                return new Node("Productions", { a0, a1, a2 });
            }
        }
    }
    tk.current = c;

    a0 = (tk.Peek().type == Token::Type::SpecialCharacter && tk.Peek().key == "LBracket") ? new Node("SpecialCharacter", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        a1 = TryParseProductions(tk);
        if (a1 != nullptr)
        {
            a2 = (tk.Peek().type == Token::Type::SpecialCharacter && tk.Peek().key == "RBracket") ? new Node("SpecialCharacter", tk.GetToken()) : nullptr;
            if (a2 != nullptr)
            {
                return new Node("Productions", { a1, });
            }
        }
    }
    tk.current = c;

    a0 = TryParseProduction(tk);
    if (a0 != nullptr)
    {
        return new Node("Productions", { a0 });
    }
    tk.current = c;

    return nullptr;
}
Node* TryParseRule1(Tokenizer& tk)
{
    int c = tk.current;
    Node* a0; Node* a1; Node* a2; Node* a3;

    a0 = (tk.Peek().type == Token::Type::Keyword && tk.Peek().key == "Rule1") ? new Node("Keyword", tk.GetToken()) : nullptr;
    if (a0 != nullptr)
    {
        a1 = (tk.Peek().type == Token::Type::Identifire) ? new Node("Name", tk.GetToken()) : nullptr;
        if (a1 != nullptr)
        {
            a2 = (tk.Peek().type == Token::Type::Operator && tk.Peek().key == "Assignment") ? new Node("Operator", tk.GetToken()) : nullptr;
            if (a2 != nullptr)
            {
                a3 = TryParseProductions(tk);
                if (a3 != nullptr)
                {
                    return new Node("Rule1", { a0, a1, a2, a3 });
                }
            }
        }
    }
    tk.current = c;

    return nullptr;
}
