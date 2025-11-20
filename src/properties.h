#include <cstdio>
#include <string.h>
#include <iostream>

using namespace std;

namespace Err {
    const string boilerErr = "Function Main called more than once"; 
    const string retErr = "Function 'return' not called";
    const string noSourceErr = "No source code found";
    const string noStartPoint = "No Start point found";
}

typedef enum
{
    TOKEN_MAIN,
    TOKEN_PRINT,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }
    TOKEN_EOF,
    TOKEN_RETURN,
    TOKEN_UNKNOWN
} TokenType;

typedef struct
{
    TokenType type;
    char value[256];
} Token;

Token getNextToken(FILE *f)
{
    Token token;
    token.type = TOKEN_UNKNOWN;
    token.value[0] = '\0';

    int c = fgetc(f);
    while (isspace(c))
        c = fgetc(f);

    if (c == EOF)
    {
        token.type = TOKEN_EOF;
        return token;
    }

    if (c == '(')
    {
        token.type = TOKEN_LPAREN;
        return token;
    }
    if (c == ')')
    {
        token.type = TOKEN_RPAREN;
        return token;
    }
    if (c == '{')
    {
        token.type = TOKEN_LBRACE;
        return token;
    }
    if (c == '}')
    {
        token.type = TOKEN_RBRACE;
        return token;
    }

    if (c == '"')
    {
        int i = 0;
        while ((c = fgetc(f)) != '"' && c != EOF)
        {
            token.value[i++] = (char)c;
        }
        token.value[i] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    if (isalpha(c))
    {
        int i = 0;
        token.value[i++] = (char)c;
        while (isalnum(c = fgetc(f)))
        {
            token.value[i++] = (char)c;
        }
        token.value[i] = '\0';
        ungetc(c, f);

        if (strcmp(token.value, "main") == 0)
            token.type = TOKEN_MAIN;
        else if (strcmp(token.value, "print") == 0)
            token.type = TOKEN_PRINT;
        else if (strcmp(token.value, "return") == 0)
        {
            token.type = TOKEN_RETURN;
        }
        else
            token.type = TOKEN_IDENTIFIER;

        return token;
    }
    if (isdigit(c))
    {
        int i = 0;
        token.value[i++] = (char)c;
        while (isdigit(c = fgetc(f)))
        {
            token.value[i++] = (char)c;
        }
        token.value[i] = '\0';
        ungetc(c, f);
        token.type = TOKEN_IDENTIFIER;
        return token;
    }

    return token;
}