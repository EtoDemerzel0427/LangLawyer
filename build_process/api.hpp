#pragma once
#if defined(_WIN32)
  #define API __declspec(dllexport)
#else
  #define API __attribute__((visibility("default")))
#endif

// inline + template（will cause COMDAT/ODR/instantiation problem）
inline int add1(int x) { return x + 1; }

template<typename T>
inline T square(T x) { return x * x; }   // template definition is usually put in the header file

extern "C" API int c_api_sum(int a, int b); // C interface: no C++ name mangling
API int cpp_api_inc(int x);                 // C++ interface: has C++ name mangling