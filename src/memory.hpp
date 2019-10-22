#ifndef __LIB_MEMORY_HPP_4129__
#define __LIB_MEMORY_HPP_4129__

#include <cstddef>
#include <string>
#include <memory>
#include "string.hpp"

namespace aux {

class AuxMemPool
{
public:
    explicit AuxMemPool(size_t sz);
    virtual ~AuxMemPool();
    template<typename T, typename... Args>
    T* create(Args&&... args)
    {
        auto ptr = (T*)this->malloc(sizeof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }
    size_t capacity() const;
    void* malloc(size_t sz);
    void* calloc(size_t sz);
    void* nalloc(size_t sz);
    void free(void *p);
    AuxString strdup(const std::string& s);
    AuxString strdup(const char* data, size_t sz);
    AuxString strdup(const char* data);
    void free(const AuxString& s);
    size_t estimate_capacity() const;
private:
    void* page(size_t sz);
    void *huge(size_t sz);
    AuxMemPool* current();

    char* _data = nullptr;
    char* _begin = nullptr;
    char* _end = nullptr;
    std::shared_ptr<AuxMemPool> _curr = nullptr;
    std::shared_ptr<AuxMemPool> _next = nullptr;
};

typedef std::shared_ptr<AuxMemPool> AuxMemPoolPtr;

}

#endif /* __LIB_MEMORY_HPP_4129__ */
