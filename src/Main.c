#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *filePath = ".\\Main.j";
int start_found = 0;
int boilerplate = 0;
int count_print = 0;
int exit_found = 0;
void init();
void write();
void print(const char *msg);
void done(const char *exit_code);
char file[4096] = "";
char data[2000] = "";
char text[2000] = "";
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

#include <ctype.h> // for isalpha, isspace

Token getNextToken(FILE *f)
{
    Token token;
    token.type = TOKEN_UNKNOWN;
    token.value[0] = '\0';

    int c = fgetc(f);
    while (isspace(c))
        c = fgetc(f); // skip spaces and newlines

    if (c == EOF)
    {
        token.type = TOKEN_EOF;
        return token;
    }

    // Handle parentheses or braces
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

    // Handle strings
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

    // Handle identifiers and keywords
    if (isalpha(c))
    {
        int i = 0;
        token.value[i++] = (char)c;
        while (isalnum(c = fgetc(f)))
        {
            token.value[i++] = (char)c;
        }
        token.value[i] = '\0';
        ungetc(c, f); // put back the last read character

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

    return token; // Unknown character fallback
}

int main()
{
    FILE *f = fopen("Main.j", "r");
    Token t;
    Token next;
    while ((t = getNextToken(f)).type != TOKEN_EOF)
    {
        // printf("Token: type=%d, value='%s'\n", t.type, t.value);
        if (t.type == TOKEN_MAIN)
        {
            init();
            start_found = 1;
        }
        else if (t.type == TOKEN_PRINT)
        {
            next = getNextToken(f);
            if (next.type == TOKEN_LPAREN)
            {
                next = getNextToken(f);
                if (next.type == TOKEN_STRING)
                {
                    print(next.value);
                }
                getNextToken(f);
            }
        }
        else if (t.type == TOKEN_RETURN)
        {
            next = getNextToken(f);

            if (strlen(next.value) > 0 && next.value[0] != ' ')
            {
                done(next.value);
                exit_found = 1;
            }

            getNextToken(f);
        }
    }
    fclose(f);
    write();
    return 0;
}

void init()
{
    strcat(file, "; Compiled in CJam, Made possible with Riad! **:\n");
    strcat(file, "default rel\n");
    strcat(file, "bits 64\n");
    strcat(file, "global main\n");
    strcat(text, "section .text\n");
    strcat(text, "main:\n");
    strcat(data, "section .data\n");
    boilerplate = 1;
}
void print(const char *msg)
{
    if (boilerplate == 1)
    {

        strcat(file, "extern GetStdHandle\n");
        strcat(file, "extern WriteConsoleA\n");

        char str[100];
        sprintf(str, "%d", count_print);

        strcat(text, "\t; print func\n");

        strcat(text, "\tsub rsp, 40\n");
        strcat(text, "\tmov ecx, -11\n");
        strcat(text, "\tcall GetStdHandle\n");
        strcat(text, "\tmov r8, rax\n");

        strcat(text, "\tlea rcx, [r8]\n");
        strcat(text, "\tlea rdx, [printmsg");
        strcat(text, str);
        strcat(text, "]\n");
        strcat(text, "\tmov r8, printlen");
        strcat(text, str);
        strcat(text, "\n");
        strcat(text, "\tmov r9, 0\n");
        strcat(text, "\tcall WriteConsoleA\n");

        strcat(data, "printmsg");
        strcat(data, str);
        strcat(data, " db '");
        strcat(data, msg);
        strcat(data, "', 0x0A, 0\n");
        strcat(data, "printlen");
        strcat(data, str);
        strcat(data, " equ $ - printmsg");
        strcat(data, str);
        strcat(data, "\n");

        count_print += 1;
    }
}

void done(const char *exit_code)
{
    strcat(file, "extern ExitProcess\n");
    strcat(text, "\n");
    strcat(text, "\tmov ecx, ");
    strcat(text, exit_code);
    strcat(text, "\n");
    strcat(text, "\tcall ExitProcess\n");
}
void write()
{
    if (start_found == 0)
    {
        fprintf(stderr, "init Error: No 'main' start point found!");
        exit(1);
    }
    else if (exit_found == 0)
    {
        fprintf(stderr, "Exit Error: No 'return' found in 'main'!");
        exit(1);
    }
    else
    {
        FILE *out = fopen("Main.asm", "w");
        if (!out)
        {
            fprintf(stderr, "Failed to open output file: %s\n", "Main.asm");
            return;
        }
        strcat(file, data);
        strcat(file, text);
        fprintf(out, "%s", file);
        fclose(out);
    }
}