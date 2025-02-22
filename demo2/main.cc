#include <cstring>
#include <iostream>
#include "FooLexer.hh"
#include "fooParser.tab.hh"

int main(int argc, char* argv[])
{
    const bool debug = argc > 1 && std::strcmp(argv[1], "--debug") == 0;
    foo::FooLexer lexer(std::cin, debug);
    foo::FooBisonParser parser(lexer, debug);
    return parser();
}