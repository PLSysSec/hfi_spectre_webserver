#include <stdint.h>
#include <iostream>
#include <string.h>
#include "../server_hostcalls.h"

uint32_t fib(uint32_t n) {
    if (n <= 2) {
        return 1;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}


int main(int argc, char const *argv[])
{
    if (argc != 2) {
        std::cerr << "exactly two argument expected, got " << argc << "\n";
        abort();
    }
    unsigned long n = atoi(argv[1]);
    uint32_t results = fib(n);
    char output[100];
    snprintf(output, 100, "%u\n", results);
    server_module_string_result(output, strlen(output));
    return 0;
}

