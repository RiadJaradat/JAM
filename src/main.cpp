    /*
a better version of Main.c
handle platforms like linux and windows
takes a file and produce an out file
*/

#include <iostream>
#include <cctype>
#include <utility>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include <chrono>
#include <ctime>



std::random_device rd;
std::mt19937 engine(rd());
std::uniform_int_distribution<long long int> distribution(0, 9223372036854775807);
const long long int ID = distribution(engine);
const std::string PROGRAM_ID = "ID_" + std::to_string(ID) + "&" + std::to_string(time(nullptr));

bool isSymbol(const char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' ||
           c == ';' || c == ',' || c == '+' || c == '-' ||
           c == '*' || c == '/';
}

bool isNumber(const std::string &s) {
    for (const char c : s) {
        if (!isdigit(c)) {
            return false;
        }
    } return true;
}

enum TokenType {
    MAIN,
    RETURN,
    UNKNOWN,
    PRINT,
    RBRACKET,
    LBRACKET,
    RCURLBRACE,
    LCURLBRACE,
    STRING,
    SMICOLON,
    NUMBER,
    VOID,
    INT,
    IS,
    STR,
    IF,
    CMP,
    NCMP,
    ADD,
    PRINTLN,
    FOR,
    COMMA,
    ELSE
};

class Token {
    public:
        std::string val;
        TokenType type;

        Token() :
            val(""),
            type(TokenType::UNKNOWN)
        {
        }

        void parse() {
            if (val == "main") {
                type = TokenType::MAIN;
            } else if (val == "print") {
                type = TokenType::PRINT;
            } else if (val == "(") {
                type = TokenType::LBRACKET;
            } else if (val == ")") {
                type = TokenType::RBRACKET;
            } else if (val == "{") {
                type = TokenType::LCURLBRACE;
            } else if (val == "}") {
                type = TokenType::RCURLBRACE;
            } else if (val == "return") {
                type = TokenType::RETURN;
            } else if (!val.empty() && val.front() == '"' && val.back() == '"') {
                type = TokenType::STRING;
            } else if (!val.empty() && val.front() == '\'' && val.back() == '\'') {
                type = TokenType::STRING;
            } else if (val == ";") {
                type = TokenType::SMICOLON;
            } else if (isNumber(val)) {
                type = TokenType::NUMBER;
            } else if (val == "void") {
                type = TokenType::VOID;
            } else if (val == "int") {
                type = TokenType::INT;
            } else if (val == "=") {
                type = TokenType::IS;
            } else if (val == "string") {
                type = TokenType::STR;
            } else if (val == "if") {
                type = TokenType::IF;
            } else if (val == "==") {
                type = TokenType::CMP;
            } else if (val == "!=") {
                type = TokenType::NCMP;
            } else if (val == "+") {
                type = TokenType::ADD;
            } else if (val == "println") {
                type = TokenType::PRINTLN;
            } else if (val == "for") {
                type = TokenType::FOR;
            } else if (val == ",") {
                type = TokenType::COMMA;
            } else if (val == "else") {
                type = TokenType::ELSE;
            } else {
                type = TokenType::UNKNOWN;
            }
        }

        Token(const std::string &v, TokenType t) : val(v), type(t) {parse();}
        Token(const std::string &v) : val(v), type(TokenType::UNKNOWN) {parse();}
        Token(const char* s) : val(s), type(TokenType::UNKNOWN) {parse();}
};

bool operator==(const Token& a, const Token& b) {
    return a.val == b.val && a.type == b.type;
}

struct TokenHash {
    std::size_t operator()(const Token& t) const noexcept {
        const std::size_t h1 = std::hash<std::string>{}(t.val);
        const std::size_t h2 = std::hash<int>{}(static_cast<int>(t.type));
        return h1 ^ (h2 << 1);
    }
};

namespace CompilerConst {
    static std::vector<Token> var;
    static std::unordered_map<Token, std::string, TokenHash> var_map;
    static std::vector<std::function<void()>> calls;
    static std::vector<std::function<void()>> extras;
    static std::string _start;
    static std::string _data;
    static std::string _bss;
    static  std::string _text;
    static bool start_found = false;
    static int printc = 0;
    static int mainc = 0;
    static int ifc = 0;
    static int forc = 0;
}

void insert(std::vector<Token>& vec, const Token &val) {
    for (const Token& t : vec) {
        if (t == val) {
            std::cerr << "variable: " << "\'" << val.val << "\'" << " already exists!" << std::endl;
            exit(1);
        }
    } vec.push_back(val);
}

void insert_map(std::unordered_map<Token, std::string, TokenHash>& map, const Token &var, const std::string &val) {
    for (const auto& [key, value] : map) {
        if (key == var) {
            std::cerr << "variable: " << '\'' << var.val << "\'" << " already exists!" << std::endl;
            exit(1);
        }
    } map[var] = val;
}

std::string getToken(FILE *f) {
    if (!f)
        return "";

    int c;

    while ((c = fgetc(f)) != EOF && isspace(c)) {}

    if (c == EOF)
        return "";

    std::string Token;

    while (c != EOF && !isspace(c)) {
        if (isSymbol(static_cast<char>(c))) {
            if (!Token.empty()) {
                ungetc(c, f);
                return Token;
            }
            Token += static_cast<char>(c);
            return Token;
        }

        else if (c == '"') {
            if (!Token.empty()) {
                ungetc(c, f);
                return Token;
            }
            Token += '"';
            while ((c = fgetc(f)) != EOF) {
                Token += static_cast<char>(c);
                if (static_cast<char>(c) == '"') {
                    return Token;
                }
            }
            std::cerr << "String was never closed \"";
            return "";
        } if (c == '\'') {
            if (!Token.empty()) {
                ungetc(c, f);
                return Token;
            }
            Token += '\'';
            while ((c = fgetc(f)) != EOF) {
                Token +=  static_cast<char>(c);
                if (static_cast<char>(c) == '\'') {
                    return Token;
                }
            }
            std::cerr << "String was never closed \'";
            return  "";
        }
        Token += static_cast<char>(c);
        c = fgetc(f);
    } return Token;
}

std::string getType(const int expected) {
    switch (expected)
    {
    case TokenType::MAIN:
        return "main";
    case TokenType::LBRACKET:
        return "(";
    case TokenType::RBRACKET:
        return ")";
    case TokenType::LCURLBRACE:
        return "{";
    case TokenType::RCURLBRACE:
        return "}";
    case TokenType::PRINT:
        return "print";
    case TokenType::RETURN:
        return "return";
    case TokenType::SMICOLON:
        return ";";
    case TokenType::STRING:
        return "''";
    case TokenType::NUMBER:
        return "int literal";
    case TokenType::VOID:
        return "void";
    case TokenType::INT:
        return "int";
    case TokenType::IS:
        return "=";
    case TokenType::STR:
        return "string";
    case TokenType::IF:
        return "if statement";
    case TokenType::CMP:
        return "==";
    case TokenType::NCMP:
        return "!=";
    case TokenType::ADD:
        return "+";
    case TokenType::PRINTLN:
        return "println";
    case TokenType::FOR:
        return "for loop";
    case TokenType::COMMA:
        return ",";
    case TokenType::ELSE:
        return "else block";
    default:
        return "Unknown";
    }
}

Token expect(FILE* f, const TokenType expected) {
    Token tok = getToken(f);

    if (tok.val.empty()) {
        std::cerr << "unexpected end of file" << std::endl;
        exit(1);
    }

    if (tok.type != expected) {
        std::cerr << "syntax error: expected token "
                  << getType(expected) << " but got '"
                  << tok.val << "'" << std::endl;
        exit(1);
    }

    return tok;
}

bool in(const Token &val, const std::vector<Token>& vec) {
    for (const Token& t : vec)
        if (t == val)
            return true;
    return false;
}

bool in(const Token &val, std::unordered_map<Token, std::string, TokenHash>& vec) {
    for (const auto& [key, value] : vec)
        if (key == val)
            return true;
    return false;
}

namespace gen_tool {
    void _print(const std::string &arg, std::string &code) {
        CompilerConst::printc++;

        CompilerConst::_data += "\tmsg" + std::to_string(CompilerConst::printc) + ": db " + arg + "\n\t";
        CompilerConst::_data += "len" + std::to_string(CompilerConst::printc) + ": equ $ - msg" + std::to_string(CompilerConst::printc) +"\n";
        code += "\t; call print\n\t";
        code += "mov rax, 1\n\t";
        code += "mov rdi, 1\n\t";
        code += "mov rsi, msg" + std::to_string(CompilerConst::printc) + "\n\t";
        code += "mov rdx, len" + std::to_string(CompilerConst::printc) + "\n\t";
        code += "syscall\n";
    }

    void _println(const std::string &arg, std::string &code) {
        CompilerConst::printc++;

        CompilerConst::_data += "\tmsg" + std::to_string(CompilerConst::printc) + ": db " + arg + ", 10\n\t";
        CompilerConst::_data += "len" + std::to_string(CompilerConst::printc) + ": equ $ - msg" + std::to_string(CompilerConst::printc) +"\n";
        code += "\t; call println\n\t";
        code += "mov rax, 1\n\t";
        code += "mov rdi, 1\n\t";
        code += "mov rsi, msg" + std::to_string(CompilerConst::printc) + "\n\t";
        code += "mov rdx, len" + std::to_string(CompilerConst::printc) + "\n\t";
        code += "syscall\n";
    }

    void _print(const Token &arg, std::string &code) {
        if (in(arg, CompilerConst::var_map)) {
            code += "\t; call print\n\t";
            code += "mov rax, 1\n\t";
            code += "mov rdi, 1\n\t";
            code += "mov rsi, " + arg.val + "\n\t";
            code += "mov rdx, len_" + arg.val + "\n\t";
            code += "syscall\n";
        } else {
            std::cerr << "undefined variable: " << arg.val << std::endl;
            exit(1);
        }
    }

    void _main() {
        CompilerConst::mainc++;
        CompilerConst::_start += "main:\n";
    }

    void _end(const int exit_code) {
        CompilerConst::_start += "\t; call return\n\t";
        CompilerConst::_start += "mov rax, 60\n\t";
        CompilerConst::_start += "mov rdi, ";
        CompilerConst::_start += std::to_string(exit_code);
        CompilerConst::_start += "\n\t";
        CompilerConst::_start += "syscall\n";
    }

    void init_int(const Token &int_identifier, const Token &int_val, std::string &code) {
        CompilerConst::_bss += '\t' + int_identifier.val;
        CompilerConst::_bss += " resd 1\n";

        code += "\tmov dword [" + int_identifier.val + "], " + int_val.val + '\n';
    }

    void reassign_int(const Token &int_val, const Token &int_identifier, std::string &code) {
        code += "\tmov dword [" + int_identifier.val + "], " + int_val.val + '\n';
    }

    void init_int(const Token &int_identifier) {
        CompilerConst::_bss += '\t' + int_identifier.val;
        CompilerConst::_bss += " resd 1\n";
    }

    void init_str(const Token &str_identifier, const Token &_str_val) {
        CompilerConst::_data += "\t" + str_identifier.val;
        CompilerConst::_data += ": db ";
        CompilerConst::_data += _str_val.val + ", 10" + "\n\t";
        CompilerConst::_data += "len_" + str_identifier.val + ": equ $ - " + str_identifier.val;
        CompilerConst::_data += "\n";
    }

    void init_void(const Token& void_identifier, std::string &code) {
        if (CompilerConst::mainc == 0) {
            code += "\tjmp main\n";
        }
        code += void_identifier.val + ":\n";
    }

    void _call(const std::string &function_name, std::string &code) {
        code += "\tcall " + function_name + "\n";
    }

    void gen_ret(const std::string &value, std::string &code) {
        code += "\tret " + value + "\n";
    }

    void _if(const Token &x, const Token &y, const Token &_operator, std::string &code, bool ELSE_BLOCK) {
        CompilerConst::ifc++;

        switch (_operator.type)
        {
        case TokenType::CMP:
            code += "\tmov eax, [" + x.val + "]\n\t";
            code += "mov ebx, [" + y.val + "]\n\t";
            code += "cmp eax, ebx\n\t";
            ELSE_BLOCK? code += "jne .else" + std::to_string(CompilerConst::ifc) + "\n":
                code += "jne .L" + std::to_string(CompilerConst::ifc) + "\n";
            break;
        case TokenType::NCMP:
            code += "\tmov eax, [" + x.val + "]\n\t";
            code += "mov ebx, [" + y.val + "]\n\t";
            code += "cmp eax, ebx\n\t";
            ELSE_BLOCK? code += "je .else" + std::to_string(CompilerConst::ifc) + "\n":
                code += "je .L" + std::to_string(CompilerConst::ifc) + "\n";
            break;
        default:
            std::cerr << "unsupported operator at: " << _operator.val << std::endl;
            exit(1);
        }

    }

    void _end_if(std::string &code, const bool ELSE_BLOCK, const int Iifc) {
        if (ELSE_BLOCK) {
            code += "jmp .L" + std::to_string(Iifc) + "\n";
            code += ".else" + std::to_string(Iifc) + ":\n";
        } else {
            code += ".L" + std::to_string(Iifc) + ":\n";
        }
    }

    void _end_else(std::string &code, const int Iifc) {
        code += "\tjmp .L" + std::to_string(Iifc) + "\n";
        code += ".L" + std::to_string(Iifc) + ":\n";
    }

    void _add(const Token &base, const Token &first, const Token &second, std::string &code) {
        if (first.type == TokenType::NUMBER && second.type == TokenType::NUMBER) {
            code += "\tmov eax, " + first.val + "\n";
            code += "\tadd eax, " + second.val + "\n";
            code += "\tmov dword [" + base.val + "], eax\n";
        } else if (first.type == TokenType::UNKNOWN && second.type == TokenType::UNKNOWN) {
            code += "\tmov eax, dword [" + first.val + "]\n";
            code += "\tadd eax, dword [" + second.val + "]\n";
            code += "\tmov dword [" + base.val + "], eax\n";
        } else if (first.type == TokenType::NUMBER && second.type == TokenType::UNKNOWN) {
            code += "\tmov eax, " + first.val + "\n";
            code += "\tadd eax, dword [" + second.val + "]\n";
            code += "\tmov dword [" + base.val + "], eax\n";
        } else if (first.type == TokenType::UNKNOWN && second.type == TokenType::NUMBER) {
            code += "\tmov eax, dword [" + first.val + "]\n";
            code += "\tadd eax, " + second.val + "\n";
            code += "\tmov dword [" + base.val + "], eax\n";
        } else {
            std::cerr << "unsupported type/s for add statement at (" + first.val + ", " + second.val + ")" << std::endl;
            exit(1);
        }

    }

    void _for(const Token &start, const Token &jump, const Token& end) {
        CompilerConst::forc++;
        CompilerConst::_start += "\tmov r12, " + start.val + "\n";
        CompilerConst::_start += "\tmov r13, " + jump.val + "\n";
        CompilerConst::_start += "\tmov r14, " + end.val + "\n";

        CompilerConst::_start += "\tcall for" + std::to_string(CompilerConst::forc) + "\n";

        CompilerConst::_text += "for" + std::to_string(CompilerConst::forc) + ":\n";
        CompilerConst::_text += "\tcmp r12, r14\n";
        CompilerConst::_text += "\tjne .F" + std::to_string(CompilerConst::forc) + "\n";
        CompilerConst::_text += "\tret\n";
        CompilerConst::_text += ".F" + std::to_string(CompilerConst::forc) + ":\n";
        CompilerConst::_text += "\tadd r12, r13\n";

    }

    void _for_end() {
        CompilerConst::_text += "\tjmp for" + std::to_string(CompilerConst::forc) + "\n";
    }

}
namespace helper_f {
    void __int(FILE *f, std::string &code);
    void __str(FILE *f);
    void __print(FILE *f, std::string &code);
    void __println(FILE *f, std::string &code);
    void __reassign(FILE *f, const Token &base, std::string &code);
    void __if(FILE *f, Token J, std::string &code);
    void __for(FILE *f, std::string &code);


    void __int(FILE *f, std::string &code) {
        Token int_identifier = expect(f, TokenType::UNKNOWN);
        insert(CompilerConst::var, int_identifier);

        Token J = getToken(f);
        if (J.type == TokenType::IS) {
            Token _int_val = expect(f, TokenType::NUMBER);
            insert_map(CompilerConst::var_map, int_identifier, _int_val.val);
            CompilerConst::calls.push_back([int_identifier, _int_val, &code]() {gen_tool::init_int(int_identifier, _int_val, code);});
            expect(f, TokenType::SMICOLON);
        } else if (J.type == TokenType::SMICOLON) {
            insert_map(CompilerConst::var_map, int_identifier, "");
            CompilerConst::calls.push_back([int_identifier]() {gen_tool::init_int(int_identifier);});
        } else {
            std::cerr << "unexpected Token: '" + J.val + "'" << std::endl;
            exit(1);
        }

    }
    void __str(FILE *f) {
        Token str_identifier = expect(f, TokenType::UNKNOWN);
        insert(CompilerConst::var, str_identifier);
        expect(f, TokenType::IS);
        Token _str_val = expect(f, TokenType::STRING);

        insert_map(CompilerConst::var_map, str_identifier, _str_val.val);

        CompilerConst::calls.push_back([str_identifier, _str_val]() {gen_tool::init_str(str_identifier, _str_val);});

        expect(f, TokenType::SMICOLON);
    }
    void __print(FILE *f, std::string &code) {
        expect(f, TokenType::LBRACKET);

        if (Token arg = getToken(f); arg.type != TokenType::STRING && arg.type != TokenType::UNKNOWN) {
            std::cerr << "print only accepts string" << std::endl;
            exit(1);
        } else if (arg.type == TokenType::UNKNOWN) {
            if (in(arg, CompilerConst::var_map)) {
                if (isNumber(CompilerConst::var_map[arg])) {
                    CompilerConst::calls.push_back([arg, &code]() {gen_tool::_print(arg.val, code);});
                } else
                    CompilerConst::calls.push_back([arg, &code]() {gen_tool::_print(arg, code);});
            } else {
                std::cerr << "undefined variable: " << arg.val << std::endl;
                exit(1);
            }
        } else
            CompilerConst::calls.push_back([arg, &code]() {gen_tool::_print(arg.val, code);});

        expect(f, TokenType::RBRACKET);
        expect(f, TokenType::SMICOLON);
    }
    void __println(FILE *f, std::string &code) {
        expect(f, TokenType::LBRACKET);

        Token arg = getToken(f);
        if (arg.type != TokenType::STRING && arg.type != TokenType::UNKNOWN) {
            std::cerr << "print only accepts string" << std::endl;
            exit(1);
        } if (arg.type == TokenType::UNKNOWN) {
            if (in(arg, CompilerConst::var_map)) {
                if (isNumber(CompilerConst::var_map[arg])) {
                    CompilerConst::calls.push_back([arg, &code]() {gen_tool::_println(arg.val, code);});
                } else
                    CompilerConst::calls.push_back([arg, &code]() {gen_tool::_print(arg, code);});
            } else {
                std::cerr << "undefined variable: " << arg.val << std::endl;
                exit(1);
            }
        } else
            CompilerConst::calls.push_back([arg, &code]() {gen_tool::_println(arg.val, code);});

        expect(f, TokenType::RBRACKET);
        expect(f, TokenType::SMICOLON);
    }

    void __if(FILE *f, Token J, std::string &code) {
        std::vector<Token> arg_if;

        expect(f, TokenType::LBRACKET);
        while ((J = getToken(f)).type != TokenType::RBRACKET) {
            if (J.val.empty()) {
                std::cerr << "'(' was never closed" << std::endl;
                exit(1);
            }

            if (J.type == TokenType::UNKNOWN) {
                if (in(J, CompilerConst::var_map)) {
                    arg_if.push_back(J);
                } else {
                    std::cerr << "undefined variable '"
                            << J.val << "'"
                            << std::endl;
                    exit(1);
                }
            } else if (J.type == TokenType::CMP) {
                arg_if.push_back(J);
            } else if (J.type == TokenType::NCMP) {
                arg_if.push_back(J);
            } else {
                std::cerr << "unsupported token "
                        << "'" + J.val + "'"
                        << " in if statement" << std::endl;
                exit(1);
            }
        }
        if (arg_if.size() == 3 &&
            arg_if.at(0).type == TokenType::UNKNOWN &&
            arg_if.at(1).type == TokenType::CMP ||
            arg_if.at(1).type == TokenType::NCMP &&
            arg_if.at(2).type == TokenType::UNKNOWN
        ) {} else {
            std::cerr << "incorrect syntax for if statement" << std::endl;
            exit(1);
        }

        bool isElseBlock = false;
        expect(f, TokenType::LCURLBRACE);
        const long pos = ftell(f);
        while ((J = getToken(f)).type != TokenType::RCURLBRACE){}
        J = getToken(f);

        if (J.type == TokenType::ELSE) {
            fseek(f, pos, SEEK_SET);
            isElseBlock = true;
        } else {
            fseek(f, pos, SEEK_SET);
            isElseBlock = false;
        }
        CompilerConst::calls.push_back(
            [arg_if, &code, isElseBlock]() {
                try {
                    gen_tool::_if(arg_if.at(0), arg_if.at(2), arg_if.at(1), code, isElseBlock);
                } catch ([[maybe_unused]] const std::out_of_range& e) {
                    std::cerr << "not a valid statement in if statement at:"
                            << std::endl << "( ";
                    for (const Token& G : arg_if)
                        std::cerr << G.val << " ";
                    std::cerr << ")" << std::endl;
                    exit(1);
                }
            });

            const int Iifc = CompilerConst::ifc;

        while ((J = getToken(f)).type != TokenType::RCURLBRACE) {
            if (J.type == TokenType::PRINT) {
                helper_f::__print(f, code);
            }  else if (J.type == TokenType::STR) {
                helper_f::__str(f);
            } else if (J.type == TokenType::INT) {
                helper_f::__int(f, code);
            } else if (J.type == TokenType::PRINTLN) {
                helper_f::__println(f, code);
            } else if (J.type == TokenType::FOR) {
                helper_f::__for(f, code);
            } else {
                if (in(J, CompilerConst::var_map) || in(J, CompilerConst::var)) {
                    if (CompilerConst::var_map[J] == "function") {
                        CompilerConst::calls.push_back([J, &code]() {gen_tool::_call(J.val, code);});
                        expect(f, TokenType::LBRACKET);
                        expect(f, TokenType::RBRACKET);
                        expect(f, TokenType::SMICOLON);
                    } else {
                        helper_f::__reassign(f, J, code);
                    }
                } else {
                    std::cerr << "invalid statement at: " << J.val << std::endl;
                    exit(1);
                }
            }
        }
        CompilerConst::calls.push_back([&code, isElseBlock, Iifc]() {gen_tool::_end_if(code, isElseBlock, Iifc);});

        if (isElseBlock) {
            expect(f, TokenType::ELSE);
            expect(f, TokenType::LCURLBRACE);
            while ((J = getToken(f)).type != TokenType::RCURLBRACE) {
                if (J.type == TokenType::PRINT) {
                    helper_f::__print(f, code);
                } else if (J.type == TokenType::STR) {
                    helper_f::__str(f);
                } else if (J.type == TokenType::INT) {
                    helper_f::__int(f, code);
                } else if (J.type == TokenType::PRINTLN) {
                    helper_f::__println(f, code);
                } else if (J.type == TokenType::FOR) {
                    helper_f::__for(f, code);
                } else if (J.type == TokenType::IF) {
                    helper_f::__if(f, J, code);
                }
            }
            CompilerConst::calls.push_back([&code, Iifc](){gen_tool::_end_else(code, Iifc);});
        }

    }

    void __reassign(FILE *f, const Token &base, std::string &code) {
        if (Token J = getToken(f); J.type == TokenType::IS) {
            Token first = getToken(f);
            Token second;
            J = getToken(f);
            switch (J.type)
            {
            case TokenType::ADD:
                second = getToken(f);
                CompilerConst::calls.push_back([base, first, second, &code] {
                    gen_tool::_add(base, first, second, code);
                });
                expect(f, TokenType::SMICOLON);
                break;
            case TokenType::SMICOLON:
                CompilerConst::calls.push_back([first, base, &code]() {
                    gen_tool::reassign_int(first, base, code);
                });
                break;
            default:
                std::cerr << "unexpected operator at Token: '" << J.val << "'" << std::endl;
                exit(1);
            }
        } else if (J.type == TokenType::SMICOLON) {

        } else {
            std::cerr << "unexpected Token '" << J.val << "'" << std::endl;
            exit(1);
        }
    }

    void __for(FILE *f, std::string &code) {
        expect(f, TokenType::LBRACKET);
        Token start = expect(f, TokenType::NUMBER);
        expect(f, TokenType::COMMA);
        Token end = expect(f, TokenType::NUMBER);
        expect(f, TokenType::COMMA);
        Token jump = expect(f, TokenType::NUMBER);
        expect(f, TokenType::RBRACKET);
        CompilerConst::calls.push_back([start, end, jump]() {
            gen_tool::_for(start, jump, end);
        });
        expect(f, TokenType::LCURLBRACE);

        Token J;

        while ((J = getToken(f)).type != TokenType::RCURLBRACE) {
            if (J.type == TokenType::PRINT) {
                helper_f::__print(f, code);
            }  else if (J.type == TokenType::STR) {
                helper_f::__str(f);
            } else if (J.type == TokenType::INT) {
                helper_f::__int(f, code);
            } else if (J.type == TokenType::PRINTLN) {
                helper_f::__println(f, code);
            } else if (J.type == TokenType::FOR) {
                helper_f::__for(f, code);
            } else {
                if (in(J, CompilerConst::var_map) || in(J, CompilerConst::var)) {
                    if (CompilerConst::var_map[J] == "function") {
                        CompilerConst::calls.push_back([J, &code]() {gen_tool::_call(J.val, code);});
                        expect(f, TokenType::LBRACKET);
                        expect(f, TokenType::RBRACKET);
                        expect(f, TokenType::SMICOLON);
                    } else {
                        helper_f::__reassign(f, J, code);
                    }
                } else {
                    std::cerr << "invalid statement at: " << J.val << std::endl;
                    exit(1);
                }
            }
        }

        CompilerConst::calls.push_back([]() {gen_tool::_for_end();});
    }

}

void loop(FILE *f) {
    if (!f) {
        std::cerr << "Failed to open file\n";
        return;
    }
    std::string T;
    while (!(T = getToken(f)).empty()) {
        Token token = T;
        switch (token.type)
        {
        case TokenType::MAIN: {
            CompilerConst::start_found = true;
            if (CompilerConst::mainc == 0)
                CompilerConst::calls.push_back(gen_tool::_main);
            else {
                std::cerr << "start point called twice" << std::endl;
                exit(1);
            }

            expect(f, TokenType::LBRACKET);
            expect(f, TokenType::RBRACKET);

            expect(f, TokenType::LCURLBRACE);

            int depth = 1;
            bool ret = false;

            while (depth > 0) {
                Token J = getToken(f);

                if (J.val.empty()) {
                    std::cerr << "char '{' was never closed" << std::endl;
                    exit(1);
                }

                if (J.type == TokenType::LCURLBRACE) {
                    depth++;
                }
                else if (J.type == TokenType::RCURLBRACE) {
                    depth--;
                    if (depth == 0) break;
                }
                else if (J.type == TokenType::PRINT) {
                    helper_f::__print(f, CompilerConst::_start);
                }
                else if (J.type == TokenType::PRINTLN) {
                    helper_f::__println(f, CompilerConst::_start);
                }
                else if (J.type == TokenType::INT) {
                    helper_f::__int(f, CompilerConst::_start);
                }
                else if (J.type == TokenType::STR) {
                    helper_f::__str(f);
                }
                else if (J.type == TokenType::FOR) {
                    helper_f::__for(f, CompilerConst::_text);
                }
                else if (J.type == TokenType::RETURN) {
                    Token value = getToken(f);
                    ret = true;
                    if (value.type != TokenType::NUMBER) {
                        std::cerr << "return expects number" << std::endl;
                        exit(1);
                    }

                    CompilerConst::calls.push_back([value]() {gen_tool::_end(stoi(value.val));});

                    expect(f, TokenType::SMICOLON);
                }
                else if (J.type == TokenType::IF) {
                    helper_f::__if(f, J, CompilerConst::_start);
                } else {
                   if (in(J, CompilerConst::var_map) || in(J, CompilerConst::var)) {
                        if (CompilerConst::var_map[J] == "function") {
                            CompilerConst::calls.push_back([J]() {gen_tool::_call(J.val, CompilerConst::_start);});
                            expect(f, TokenType::LBRACKET);
                            expect(f, TokenType::RBRACKET);
                            expect(f, TokenType::SMICOLON);
                        } else {
                            helper_f::__reassign(f, J, CompilerConst::_start);
                        }
                    } else {
                        std::cerr << "invalid statement at: " << J.val << std::endl;
                        exit(1);
                    }
                }
            }

            if (!ret) {
                std::cerr << "No return statement" << std::endl;
                exit(1);
            }

            break;
        }

        case TokenType::VOID: {
            int depth = 0;
            Token _func_identifier = expect(f, TokenType::UNKNOWN);
            insert(CompilerConst::var, _func_identifier);
            insert_map(CompilerConst::var_map, _func_identifier, "function");
            expect(f, TokenType::LBRACKET);
            expect(f, TokenType::RBRACKET);
            expect(f, TokenType::LCURLBRACE);
            depth++;

            CompilerConst::var.push_back(_func_identifier);
            CompilerConst::calls.push_back([_func_identifier]() {gen_tool::init_void(_func_identifier, CompilerConst::_start);});

            while (depth > 0) {
                Token J = getToken(f);

                if (J.type == TokenType::RCURLBRACE) {
                    depth--;
                    if (depth <= 0)
                        break;
                }
                if (J.val.empty()) {
                    std::cerr << "'{' was never closed" << std::endl;
                    exit(1);
                }
                if (J.type == TokenType::PRINT) {
                    helper_f::__print(f, CompilerConst::_start);
                } else if (J.type == TokenType::IF) {
                    helper_f::__if(f, J, CompilerConst::_start);
                } else if (J.type == TokenType::INT) {
                    helper_f::__int(f, CompilerConst::_start);
                } else if (J.type == TokenType::PRINTLN) {
                    helper_f::__println(f, CompilerConst::_start);
                } else if (J.type == TokenType::FOR) {
                    helper_f::__for(f, CompilerConst::_text);
                } else {
                    if (in(J, CompilerConst::var_map) || in(J, CompilerConst::var)) {
                        if (CompilerConst::var_map[J] == "function") {
                            CompilerConst::calls.push_back([J]() {gen_tool::_call(J.val, CompilerConst::_start);});
                            expect(f, TokenType::LBRACKET);
                            expect(f, TokenType::RBRACKET);
                            expect(f, TokenType::SMICOLON);
                        } else
                            helper_f::__reassign(f, J, CompilerConst::_start);
                    } else {
                        std::cerr << "invalid statement at: " << J.val << std::endl;
                        exit(1);
                    }
                }
            }

            std::string ret_val;

            CompilerConst::calls.push_back([ret_val]() {gen_tool::gen_ret(ret_val, CompilerConst::_start);});

            break;
        }

        case TokenType::INT: {
            helper_f::__int(f, CompilerConst::_start);
            break;
        }

        case TokenType::STR: {
            helper_f::__str(f);
            break;
        }

        default: {
            std::cerr << "unexpected token '" << token.val << "'" << std::endl;
            exit(1);
        }

        };
    }

    fclose(f);
}

void write(FILE *f) {
    if (!f) {
        perror("fopen");
        return;
    }

    const std::string s = CompilerConst::_data + CompilerConst::_bss + CompilerConst::_text + CompilerConst::_start;

    fputs(s.c_str(), f);
    fclose(f);
}

std::string FILE_NAME;
bool gen_asm = false;

int main(const int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "No input File" << std::endl;
        return 1;
    }

    for (int i = 0; i < argc; i++) {
        if (std::string arg = argv[i]; arg == "-o") {
            // Check if there is actually a name after -o
            if (i + 1 < argc) {
                FILE_NAME = argv[i + 1];
            } else {
                std::cerr << "Error: -o requires a filename!" << std::endl;
                return 1;
            }
        } else if (arg == "-s") {
            gen_asm = true;
        }
    }

    CompilerConst::_data += "; code generated by 'CJAM'\n";
    CompilerConst::_data += "section .data\n";
    CompilerConst::_text += "section .text\n";
    CompilerConst::_start += "global _start\n";
    CompilerConst::_start += "_start:\n";
    CompilerConst::_bss += "section .bss\n";

    FILE *f = fopen(argv[1], "r");
    loop(f);
    if (!CompilerConst::start_found) {
        std::cerr << "start point not found!" << std::endl;
        exit(1);
    }
    for (auto& c : CompilerConst::extras)
        CompilerConst::calls.push_back(c);
    for (auto& c : CompilerConst::calls)
        c();

    const std::string ASM_FILENAME = PROGRAM_ID + ".s";

    FILE *out_f = fopen(ASM_FILENAME.c_str(), "w");
    write(out_f);

    std::string command = "nasm -f elf64 \"" + ASM_FILENAME +
                          "\" -o \"" + PROGRAM_ID + ".o\" && " +
                          "ld \"" + PROGRAM_ID + ".o\" -o " + FILE_NAME;

    if (const int res = system(command.c_str()); res != 0)
        std::cerr << "Compilation failed" << std::endl;

    command = "rm -f \"" + PROGRAM_ID + ".o\"";
    system(command.c_str());
    command = "rm -f \"" + ASM_FILENAME + "\"";
    gen_asm?
    // pass
    :system(command.c_str());
    return 0;
}
