/*
 * a better version of main.c for CJAM written in C++
 * date: 2025-11-19
 * this is a full Compiler and runner that take a source code and turns into .exe file
 */

#include <iostream>
#include <cstdio>
#include <string.h>

using namespace std;

string out;
string s_data;
string s_text;

void init()
{
    s_data.append(".section data\n");

    s_text.append(".section text\n");
    s_text.append("global start\n");
    s_text.append("start:\n\t");
    s_text.append("hlt\n\t");
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

void code_loop(const char* path) {
    FILE *f = fopen(path, "r");
    Token t;
    Token next;
    while ((t = getNextToken(f)).type != TOKEN_EOF)
    {
        cout << t.type << endl;
        if (t.type == TOKEN_MAIN) {
            init();
        }
    }
}

int main(int argc, char* argv[]) 
{
    if (argc < 2)
    {
        cout << "no source code found" << endl;
        return 1;
    }

    char* fileName = argv[1];

    // start point

    code_loop(fileName);

    return 0;
}