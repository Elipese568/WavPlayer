#pragma once

#include <iostream>
#include <fstream>

#define address_as(v,t) ((t*)&v)
#define address_to(v,t) ((t*)v)

#ifdef _UNICODE
typedef LPWSTR GLPSTR;
typedef LPCWSTR GLPCSTR;
#else
typedef LPSTR GLPSTR;
typedef LPCSTR GLPCSTR;
#endif

#define nocopy(vt) const vt&

template<class T>
T readOf(std::istream& stream){
    T result;
    stream.read(address_as(result, char), sizeof(T));
    return result;
}

template<size_t Size>
std::ostream& operator<<(std::ostream& os, char (&str)[Size])
{
    return os.write(str, Size);
}

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };
template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;
