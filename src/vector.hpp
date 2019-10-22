#ifndef __LIB_AUX_VECTOR_HPP_20415__
#define __LIB_AUX_VECTOR_HPP_20415__

#include "memory.hpp"
#include <algorithm>

namespace dmn {

template<typename T>
class AuxVector {
public:
    AuxVector(std::shared_ptr<AuxMemPool>& pool) :
        _pool(pool)
    {}
    bool empty() const
    {
        return _size == 0;
    }
    size_t size() const
    {
        return _size;
    }
    size_t capacity() const
    {
        return _capacity;
    }
    T* begin()
    {
        return _data;
    }
    T* end()
    {
        return _capacity ? _data + _size : nullptr;
    }
    T& operator[] (size_t idx)
    {
        return _data[idx];
    }
    const T& operator[] (size_t idx) const
    {
        return _data[idx];
    }
    void push_back(const T& val)
    {
        if (_capacity <= _size)
        {
            reserve(_capacity ? _capacity * 2 : 1);
        }
        new (&_data[_size++]) T(val);
    }
    void remove(const T& val)
    {
        size_t n = 0;
        T* tmp = _data;
        T* first = _data;
        T* last = _data + _size;
        for (; first != last; ++first)
        {
            if (!(*first == val))
            {
                *tmp++ = *first;
            }
            else
            {
                n++;
            }
        }
        _size -= n;
    }
    template<typename UnaryPredicate>
    void remove_if(UnaryPredicate p)
    {
        size_t n = 0;
        T* tmp = _data;
        T* first = _data;
        T* last = _data + _size;
        for (; first != last; ++first)
        {
            if (!p(*first))
            {
                *tmp++ = *first;
            }
            else
            {
                n++;
            }
        }
        _size -= n;
    }
    void pop_back()
    {
        _data[_size--].~T();
    }
    void reserve(size_t sz)
    {
        auto sp = _pool.lock();
        if (!_data)
        {
            _data = (T*)sp->malloc(sz*sizeof(T));
            _size = 0;
            _capacity = sz;
        }
        else
        {
            auto new_data = (T*)sp->malloc(sz*sizeof(T));
            for (size_t i = 0; i < _size; i++)
            {
                new (new_data + i) T(_data[i]);
                _data[i].~T();
            }
            sp->free(_data);
            _capacity = sz;
            _data = new_data;
        }
    }
    void sort()
    {
        std::sort(_data, _data + _size);
    }
    template<typename P, typename F>
    bool update(const P& pred, const F& upd_f)
    {
        for (size_t i = 0; i < _size; i++)
        {
            T& elem = _data[i];
            if (pred(elem))
            {
                upd_f(elem);
                return true;
            }
        }
        return false;
    }
private:
    size_t _size = 0;
    size_t _capacity = 0;
    T* _data = nullptr;
    std::weak_ptr<AuxMemPool> _pool = nullptr;
};

}

#endif /* __LIB_AUX_VECTOR_HPP_20415__ */
