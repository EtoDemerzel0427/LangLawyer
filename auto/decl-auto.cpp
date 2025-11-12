// an example that demonstrates the difference between decltype(auto) and auto
// decltype(auto) can preserve the reference type of the initializer expression and cv-qualifiers
// auto will always strip references and cv-qualifiers from the initializer expression

// decltype(auto) is very useful when you need perfect forwarding in a function template:
// template<typename T>
// decltype(auto) forward_wrapper(T&& value) {
//     return std::forward<T>(value);
// }
// or in a lambda:
// auto forward_lambda = [](auto&& value) -> decltype(auto) {
//     return std::forward<decltype(value)>(value);
// };

#include <iostream>
#include <type_traits>

// preserve reference type
int x = 0;
int& foo() { return x; }

auto bar1() { return foo(); }           // return int
decltype(auto) bar2() { return foo(); } // return int&

// preserve cv-qualifiers
const int g_value = 42;

// this const actually is stripped off
const int return_const() {
    return g_value;
}

// the const is not preserved even though decltype(g_value) is const int
// because a copy is returned
decltype(auto) return_const_with_decltype_auto() {
    return g_value;  // copy of g_value, so return int
}

decltype(auto) return_const_with_decltype_lvalue() {
    return (g_value);  // we add parentheses to make it an lvalue, so return const int&
}


// this const is preserved
const int& return_const_ref() {
    return g_value;
}

int main() {
    std::cout << std::is_reference_v<decltype(bar1())> << '\n'; //  0
    std::cout << std::is_reference_v<decltype(bar2())> << '\n'; //  1
    std::cout << std::is_reference_v<decltype(return_const_with_decltype_auto())> << '\n'; // 0
    std::cout << std::is_reference_v<decltype(return_const_with_decltype_lvalue())> << '\n'; // 1

    std::cout << std::is_const_v<decltype(return_const())> << '\n';                  // 0
    std::cout << std::is_const_v<decltype(return_const_with_decltype_auto())> << '\n'; // 0
    std::cout << std::is_const_v<std::remove_reference_t<decltype(return_const_with_decltype_lvalue())>> << '\n'; // 1
    std::cout << std::is_const_v<std::remove_reference_t<decltype(return_const_ref())>> << '\n'; // 1
}