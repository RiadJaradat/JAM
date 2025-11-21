/*
 * a better version of main.c for CJAM written in C++
 * date: 2025-11-19
 * this is a full Compiler and runner that take a source code and turns into .exe file
 */

#include <cstdio>
#include <exception>
#include <iostream>
#include "properties.h"
#include <stdexcept> 

using namespace std;

string out;
string s_data;
string s_text;

bool boilerplate = false;

void init()
{
    s_data.append(".section data\n");

    s_text.append(".section text\n");
    s_text.append("global start\n");
    s_text.append("start:\n\t");
    s_text.append("hlt\n\t");

}

void done() {
    s_text.append("xor rdi, rdi\n\t");
    s_text.append("mov rax, 60\n\t");
    s_text.append("syscall\n\t");

}


void code_loop(const char* path) {
    FILE *f;   
    try{
   	 FILE *f = fopen(path, "r");
    } catch (exception e) {
	    cerr << "FileNotFoundErr: " << Err::FileNotFoundErr << endl;
    }
    Token t;
    Token next;
    while ((t = getNextToken(f)).type != TOKEN_EOF) {
        cout << t.type << endl;
        if (t.type == TOKEN_MAIN) {
            if (!boilerplate) {
                init();
                boilerplate = true;
            } else {
                cerr << "boilerErr: " << Err::boilerErr << endl;
            }

        } else if (t.type == TOKEN_RETURN) {
            done();
        }
    } if (!boilerplate) {
        cerr << "noStartPoint" << Err::noStartPoint << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "noSourceErr: " << Err::noSourceErr << endl;
        return 1;
    }

    char* fileName = argv[1];

    // start point

    code_loop(fileName);

    return 0;
}
