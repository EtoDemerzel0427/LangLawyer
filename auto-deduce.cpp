/**
 * This code explores the behavior of C++'s `auto` type deduction in various contexts, focusing on
 * how `auto` strips qualifiers such as `const` and `&` (reference) when deducing types.
 * It examines the type properties for variables deduced with `auto`, `auto&`, and `auto&&` in
 * different situations, as well as the resulting transformations when types undergo decay,
 * particularly array-to-pointer and function-to-pointer decays. This code also uses `std::is_*`
 * type traits to print out type properties, such as whether a type is a reference, a pointer,
 * an array, or a function. The cases cover:
 *
 * 1. `auto x = expr;` where `auto` may strip `const` and `&` qualifiers.
 * 2. `auto& x = expr;` where `auto` deduces `const` references and array references.
 * 3. `auto&& x = expr;` (universal references) where `auto` deduces `T&` for lvalues and `T&&` for rvalues.
 */


#include <iostream>
#include <type_traits>

void foo() {}

int main() {
    /***************************
     * case 1: auto x = expr;
    ***************************/
    const int a = 42;
    auto b = a; // b is int, const is stripped off

    std::cout << std::is_const_v<decltype(a)> << '\n'; // 1
    std::cout << std::is_const_v<decltype(b)> << '\n'; // 0
    std::cout << "-------------------\n";

    const int &c = a;
    auto d = c; // d is int, const and reference are stripped off

    std::cout << std::is_const_v<decltype(c)> << '\n'; // 0, because the const is the int itself not the reference
    std::cout << std::is_const_v<decltype(d)> << '\n'; // 0
    std::cout << std::is_reference_v<decltype(c)> << '\n'; // 1
    std::cout << std::is_reference_v<decltype(d)> << '\n'; // 0
    std::cout << std::is_const_v<std::remove_reference_t<decltype(c)>> << '\n'; // 1
    std::cout << std::is_const_v<std::remove_reference_t<decltype(d)>>
              << '\n'; // 0, btw it is not even a reference before removing it
    std::cout << "-------------------\n";

    int arr[3] = {1, 2, 3};
    auto e = arr; // e is int*, array to pointer decay

    std::cout << std::is_array_v<decltype(arr)> << '\n'; // 1
    std::cout << std::is_array_v<decltype(e)> << '\n'; // 0
    std::cout << std::is_pointer_v<decltype(e)> << '\n'; // 1
    std::cout << "-------------------\n";

    auto f = foo; // f is void(*)(), function to pointer decay

    std::cout << std::is_function_v<decltype(foo)> << '\n'; // 1
    std::cout << std::is_function_v<decltype(f)> << '\n'; // 0
    std::cout << std::is_pointer_v<decltype(f)> << '\n'; // 1
    std::cout << "-------------------\n";

    /***************************
     * case 2: auto& x = expr;
    ***************************/
    auto &ref_b = a; // ref_b is const int&

    std::cout << std::is_reference_v<decltype(ref_b)> << '\n'; // 1
    std::cout << std::is_const_v<std::remove_reference_t<decltype(ref_b)>> << '\n'; // 1
    std::cout << "-------------------\n";

    auto &ref_e = arr; // ref_e is int(&)[3]

    std::cout << std::is_reference_v<decltype(ref_e)> << '\n'; // 1
    std::cout << std::is_array_v<std::remove_reference_t<decltype(ref_e)>> << '\n'; // 1
    std::cout << "-------------------\n";

    auto &ref_f = foo; // ref_f is void(&)()

    std::cout << std::is_reference_v<decltype(ref_f)> << '\n'; // 1
    std::cout << std::is_function_v<std::remove_reference_t<decltype(ref_f)>> << '\n'; // 1

    /***************************
     * case 3: auto&& x = expr;
     ***************************/

    // a1 is lvalue, so auto is int&, r1 type: int& && -> int&
    int a1 = 42;
    auto &&r1 = a1;
    std::cout << std::is_same_v<decltype(r1), int &> << '\n'; // 1

    // b1 is const lvalue, so auto is const int&, r2 type:  const int& && -> const int&
    const int b1 = 42;
    auto &&r2 = b1;
    std::cout << std::is_same_v<decltype(r2), const int &> << '\n'; // 1

    // 42 is rvalue, so auto is int, r3 type: int&&
    auto &&r3 = 42;
    std::cout << std::is_same_v<decltype(r3), int &&> << '\n'; // 1

    // std::move(b1) is const int&&, so auto is const int&&, r4 type: const int&& && -> const int&&
    auto &&r4 = std::move(b1);
    std::cout << std::is_same_v<decltype(r4), const int &&> << '\n'; // 1

    // rref is int&&, but rref itself is lvalue, so auto is int&, r5 type: int& && -> int&
    int &&rref = 42;
    auto &&r5 = rref;
    std::cout << std::is_same_v<decltype(r5), int &> << '\n'; // 1

    return 0;
}
