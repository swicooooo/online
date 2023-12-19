#include "lexer.hpp"
#include <iostream>

int main() {
    lexer lex("       123  + 456*789  ");

    while (lex.GetNextChar()) {
        std::cout << lex.GetNextToken() <<std::endl;
    }

    return 0;
}