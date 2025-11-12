// example that demonstrates the use of concepts with auto

#include <iostream>
#include <concepts>
#include <vector>


template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
void printSum(const std::vector<T>& v) {
    Numeric auto sum = T{};  // type-constraint auto
    for (const auto& element : v) {
        sum += element;
    }
    std::cout << "Sum: " << sum << std::endl;
}

int main() {
    std::vector<int> ints = {1, 2, 3, 4, 5};
    std::vector<double> doubles = {1.1, 2.2, 3.3, 4.4, 5.5};
    std::vector<std::string> strings = {"Hello", " ", "World"};

    std::cout << "Sum of ints: ";
    printSum(ints);

    std::cout << "Sum of doubles: ";
    printSum(doubles);

//     std::cout << "Sum of strings: ";
//     printSum(strings);  // this will not compile

    return 0;
}