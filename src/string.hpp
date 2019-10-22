#ifndef __LIB_STRING_HPP_17329__
#define __LIB_STRING_HPP_17329__

#include <cstddef>
#include <string>

namespace aux {

struct AuxString
{
    char* strnstr(const AuxString& other);
    char* strxstr(const AuxString &other);
    bool empty() const;
    char* data = nullptr;
    size_t size = 0;

    size_t estimate_capacity() const;
};

bool operator==(const AuxString& a, const std::string& b);

}

#endif /* __LIB_STRING_HPP_17329__ */
