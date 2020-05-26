
#include <iostream>
#include <string>

#define RAPIDXML_NO_EXCEPTIONS
#include "xml2json.hpp"

#include "../server_hostcalls.h"
// #define server_module_string_result(a) std::cout << a << "\n";

using namespace std;

int main(const int argc, const char *const argv[])
{
    if(argc != 2) {
        printf("One argument expected [(blank), quality], got %d args\n", argc);
        abort();
    }

    auto ret = xml2json(argv[1]);
    const char* resp = ret.c_str();
    server_module_string_result(resp);
    return 0;
}