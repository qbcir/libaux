#ifndef __LIB_AUX_HASH_MAP_HPP_17138__
#define __LIB_AUX_HASH_MAP_HPP_17138__

#include "memory.hpp"

namespace dmn {

template<typename K, typename V>
class AuxHashMap {
public:
    constexpr static const size_t INITIAL_SIZE = 256;
    constexpr static const double FULL_FACTOR = 0.7;
    constexpr static const size_t INCREASE_FACTOR = 4;

    struct elem_t
    {
        K key;
        V value;
        elem_t* next = nullptr;
        bool used = false;
    };

    class iter_t
    {
    public:
        iter_t (size_t bucket,  elem_t* curr, AuxHashMap<K, V>* hm) :
            _bucket(bucket),
            _curr(curr),
            _hm(hm)
        {}
        K& key()
        {
            return _curr->key;
        }
        const K& key() const
        {
            return _curr->key;
        }
        V& value()
        {
            return _curr->value;
        }
        const V& value() const
        {
            return _curr->value;
        }
        iter_t next()
        {
            iter_t iter = *this;
            elem_t* item = _curr ? _curr->next : nullptr;

            while (true)
            {
                if (iter._bucket >= _hm->_num_buckets && item && !item->next)
                {
                    return _hm->end();
                }
                if (!item)
                {
                    iter._bucket++;
                    if (iter._bucket > _hm->_num_buckets - 1)
                    {
                        return _hm->end();
                    }
                    item = &(_hm->_buckets[iter._bucket]);
                }
                if (item)
                {
                    if (item->used)
                    {
                        break;
                    }
                    item = item->next;
                }
            }
            iter._curr = item;
            return iter;
        }
        bool operator == (const iter_t& other)
        {
            return other._bucket == _bucket && _curr == other._curr && other._hm == _hm;
        }
        bool operator != (const iter_t& other)
        {
            return !(*this == other);
        }
    private:
        ssize_t _bucket = 0;
        AuxHashMap<K, V>::elem_t* _curr = nullptr;
        AuxHashMap* _hm = nullptr;
    };

    AuxHashMap(std::shared_ptr<AuxMemPool>& pool) :
        _pool(pool)
    {
        auto sp = _pool.lock();
        _buckets = (elem_t*)sp->calloc(INITIAL_SIZE * sizeof(elem_t));
        _num_buckets = INITIAL_SIZE;
        _size = 0;
        _next_size = (size_t)(_num_buckets * FULL_FACTOR);
    }

    size_t size() const
    {
        return _size;
    }

    iter_t begin()
    {
        iter_t it(-1, nullptr, this);
        return it.next();
    }

    iter_t end()
    {
        return iter_t(_num_buckets, nullptr, this);
    }

    std::pair<V*, bool> get(const K& key) const
    {
        auto row = get_elem(key);
        if (row)
        {
            return std::make_pair(&row->value, true);
        }
        return std::make_pair(nullptr, false);
    }

    template<typename F>
    void update(const K& key, const F& upd_f)
    {
        bool ok;
        V* p;
        std::tie(p, ok) = get(key);
        if (ok)
        {
            V& elem = *p;
            upd_f(elem);
        }
    }

    void set(const K& key, const V& value)
    {
        if (_size >= _next_size)
        {
            resize();
        }
        insert_bucket(_buckets, _num_buckets, key, value);
        _size++;
    }

    bool remove(const K& key)
    {
        auto* row = get_elem(key);
        if (!row)
        {
            return false;
        }
        row->~elem_t();
        row->used = false;
        _size--;
        return true;
    }

private:
    elem_t* get_elem(const K& key) const
    {
        static std::hash<K> hasher;
        size_t hash = hasher(key);
        auto row = &(_buckets[hash % _num_buckets]);
        while (row)
        {
            if (row->key == key)
            {
                return row;
            }
            row = row->next;
        }
        return nullptr;
    }

    bool insert_bucket(elem_t* buckets, size_t num_buckets, const K& key, const V& value)
    {
        static std::hash<K> hasher;
        size_t hash = hasher(key);
        elem_t* row = &(buckets[hash % num_buckets]);
        elem_t *head = nullptr;
        elem_t *parent = head = row;
        elem_t *tomb = nullptr;
        while (row)
        {
            if (!row->used)
            {
                tomb = row;
            }
            parent = row;
            if (row->used && row->key == key)
            {
                row->key = key;
                row->value = value;
                return false;
            }
            row = row->next;
        }
        if (tomb)
        {
            row = tomb;
        }
        if (!row)
        {
            auto sp = _pool.lock();
            row = (elem_t*)sp->calloc(sizeof(elem_t));
        }
        new (&row->key) K(key);
        new (&row->value) V(value);
        row->used = true;
        if (parent != row && parent)
        {
            parent->next = row;
        }
        return true;
    }

    void free_bucket(elem_t *bucket)
    {
        elem_t* tmp = bucket;
        elem_t* bk = bucket;
        elem_t* root = bucket;
        auto sp = _pool.lock();
        while (tmp)
        {
            bk = nullptr;
            if (tmp->next)
            {
                bk = tmp->next;
            }
            if (tmp->used)
            {
                tmp->key.~K();
                tmp->value.~V();
            }
            if (tmp != root)
            {
                sp->free(tmp);
            }
            tmp = bk;
         }
    }

    void resize()
    {
        size_t old_size, new_size;
        size_t items = 0;
        new_size = old_size = _num_buckets;
        new_size *= INCREASE_FACTOR;

        auto sp = _pool.lock();
        elem_t* old_buckets = _buckets;
        elem_t* new_buckets = (elem_t*)sp->calloc(new_size * sizeof(elem_t));
        for (size_t i = 0; i < old_size; i++)
        {
            auto entry = &(old_buckets[i]);
            while (entry)
            {
                if (entry->used)
                {
                    insert_bucket(new_buckets, new_size, entry->key, entry->value);
                    items++;
                }
                entry = entry->next;
            }
        }
        for (size_t i = 0; i < old_size; i++)
        {
            free_bucket(&(old_buckets[i]));
        }
        sp->free(old_buckets);
        _num_buckets = new_size;
        _size = items;
        _buckets = new_buckets;
        _next_size = (size_t)(_num_buckets * FULL_FACTOR);
    }

    size_t _size = 0;
    size_t _next_size = 0;
    ssize_t _num_buckets = 0;
    elem_t* _buckets = nullptr;
    std::weak_ptr<AuxMemPool> _pool = nullptr;
};

}

#endif /* __LIB_AUX_HASH_MAP_HPP_17138__ */
