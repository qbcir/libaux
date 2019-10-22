#include "string.hpp"
#include <cstring>
#include <cctype>

namespace aux {

char* AuxString::strnstr(const AuxString &other)
{
    if (!data || !*data) return NULL;
    if (!other.data || !*other.data) return (char*)data;

    if (size < other.size) return NULL;

    const char *es = data + size;
    const char *en = other.data + other.size;
    const char* s = data;

    for (; s < es; ++s)
    {
        if (*s == *other.data)
        {
            auto x = s;
            for (auto w = s; w < es && x < en; ++w, ++x)
            {
                if (*w != *x) break;
            }

            if (x == en) return (char *) s;
        }
    }

    return NULL;
}

char* AuxString::strxstr(const AuxString &other)
{
    if (!data || !*data) return NULL;
    if (!other.data || !*other.data) return (char*)data;
    if (size < other.size) return NULL;

    const char* s = data;
    auto es = s + size;
    auto en = other.data + other.size;

    for (; s < es; ++s)
    {
        if (tolower(*s) == tolower(*other.data))
        {
            auto x = other.data;
            for (auto w = s; w < es && x < en; ++w, ++x)
            {
                if (tolower(*w) != tolower(*x)) break;
            }

            if (x == en) return (char *) s;
        }
    }

    return NULL;
}

bool AuxString::empty() const
{
    return size == 0;
}

size_t AuxString::estimate_capacity() const
{
    return size;
}

bool operator==(const AuxString& a, const std::string& b)
{
    return a.size == b.size() && strncmp(a.data, b.c_str(), a.size) == 0;
}

}
