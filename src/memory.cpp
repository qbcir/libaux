#include "memory.hpp"
#include <cstring>
#include <coda/logger.h>

namespace aux {

static inline char* align_ptr(char* p, uintptr_t a)
{
    return (char*)(((uintptr_t) (p) + (a - 1)) & ~(a - 1));
}

AuxMemPool::AuxMemPool(size_t sz)
{
    _begin = (char*)::malloc(sz);
    if (_begin == NULL)
    {
        log_error("bad alloc: %zu", sz);
        throw std::bad_alloc();
    }
    _data = _begin;
    _end = _begin + sz;
}

AuxMemPool::~AuxMemPool()
{
    ::free(_begin);
}

size_t AuxMemPool::capacity() const
{
    return size_t(_end - _begin);
}

void* AuxMemPool::malloc(size_t sz)
{
    if (sz > capacity())
    {
        return huge(sz);
    }
    for (auto curr = current(); curr; curr = curr->_next.get())
    {
        auto dest = align_ptr(curr->_data, sizeof(uintptr_t));
        if (dest + sz <= curr->_end)
        {
            curr->_data = dest + sz;
            return dest;
        }
    }
    return page(sz);
}

void* AuxMemPool::calloc(size_t sz)
{
    auto dest = this->malloc(sz);
    memset(dest, 0, sz);
    return dest;
}

void* AuxMemPool::nalloc(size_t sz)
{
    if (sz > capacity())
    {
        return huge(sz);
    }
    for (auto curr = current(); curr; curr = curr->_next.get())
    {
        auto dest = curr->_data;
        if (dest + sz <= curr->_end)
        {
            curr->_data = dest + sz;
            return (void*)dest;
        }
    }
    return page(sz);
}

void AuxMemPool::free(void *p)
{

}

void AuxMemPool::free(const AuxString& s)
{
    this->free(s.data);
}

size_t AuxMemPool::estimate_capacity() const
{
    size_t cap = capacity();
    for (auto next = _next; next; next = next->_next)
    {
        cap += next->capacity();
    }
    return cap;
}

AuxString AuxMemPool::strdup(const std::string& s)
{
    AuxString res;
    res.data = (char*)this->nalloc(s.size());
    memcpy(res.data, s.c_str(), s.size());
    res.size = s.size();
    return res;
}

AuxString AuxMemPool::strdup(const char* data, size_t sz)
{
    AuxString res;
    res.data = (char*)this->nalloc(sz);
    memcpy(res.data, data, sz);
    res.size = sz;
    return res;
}

AuxString AuxMemPool::strdup(const char* data)
{
    if (data == nullptr)
    {
        AuxString res;
        res.size = 0;
        res.data = nullptr;
        return res;
    }
    return this->strdup(data, strlen(data));
}

void* AuxMemPool::page(size_t sz)
{
    std::shared_ptr<AuxMemPool> next = std::make_shared<AuxMemPool>(capacity());
    void* dest = next->_data;
    next->_data += sz;
    while(current()->_next)
    {
        _curr = current()->_next;
    }
    current()->_next = next;
    _curr = next;
    return dest;
}

void* AuxMemPool::huge(size_t sz)
{
    std::shared_ptr<AuxMemPool> next = std::make_shared<AuxMemPool>(sz);
    void* dest = next->_data;
    next->_data += sz;
    next->_next = _next; /* huge pages are allocated just after the first page */
    _next = next;
    return dest;
}

AuxMemPool* AuxMemPool::current()
{
    return _curr ? _curr.get() : this;
}

}
