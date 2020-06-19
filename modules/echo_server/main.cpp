#include <stdint.h>
#include <iostream>
#include <string.h>
#include "../server_hostcalls.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        std::cerr << "exactly two argument expected, got " << argc << "\n";
        abort();
    }
    server_module_string_result(argv[1], strlen(argv[1]));
    return 0;
}

