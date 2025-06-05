#include "compiler.h"
#include <cstring>
#include <iostream>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

bool LEXER_DEBUG;
std::ofstream lexerDebugger;
bool PARSER_DEBUG;
std::ofstream parserDebugger;

int main(int argc, char *argv[]) {
    fs::create_directories("/home/zenyuzien/Downloads/almond/almond/debug_logs");

    LEXER_DEBUG = false;
    PARSER_DEBUG = false;
    // Check command line arguments
    for (int i = 0; i < argc; i++) {

        if (strcmp(argv[i], "-dl") == 0) {
            LEXER_DEBUG = true;
            lexerDebugger.open("/home/zenyuzien/Downloads/almond/almond/debug_logs/lexer.log", std::ios::out);
            if (!lexerDebugger.is_open()) {
                std::cerr << "Failed to open lexer_debug.log\n";
                return 1;
            }
        }

        if (strcmp(argv[i], "-dp") == 0) {
            PARSER_DEBUG = true;
            parserDebugger.open("/home/zenyuzien/Downloads/almond/almond/debug_logs/parser.log", std::ios::out);
            if (!parserDebugger.is_open()) {
                std::cerr << "Failed to open parser_debug.log\n";
                return 1;
            }
        }
    }

    auto c = new compilation();
    int res = c->compileFile("/home/zenyuzien/Downloads/almond/almond/test.c", "/home/zenyuzien/Downloads/almond/almond/test.asm", 0);
    if (res == 1) {
        std::cout << "Compiled succesfully! \n";
    } else if (res == 0) {
        std::cout << "Comp failed ! \n";
    } else {
        std::cout << res << " idk what happened \n";
    }
    return 0;
}