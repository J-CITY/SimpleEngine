#include "assertion.h"

#include <iostream>

using namespace IKIGAI::UTILS;

void IKIGAI::UTILS::__Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg) {
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}
