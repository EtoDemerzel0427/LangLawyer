#include "api.hpp"

static int helper(int y) { return y * 2; }       // internal linkage (this TU is visible)
int hidden_func(int z) { return helper(z) + 3; } // external linkage，但稍后我们不导出它

extern "C" int c_api_sum(int a, int b) { // C symbol name "c_api_sum"
    return a + b;
}

int cpp_api_inc(int x) { // C++ symbol name (Itanium mangling)
    // call the inline and template in the header file (will be instantiated in this TU)
    int t = add1(x);
    return square(t); // trigger the instantiation of square<int>
}
