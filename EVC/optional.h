/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
//#if __cplusplus >= 201703L
#include <optional>
using std::optional;
/*#else
#include <initializer_list>
template<class T>
class optional
{
    bool assigned;
    T *value;
    public:
    optional()
    {
        assigned = false;
        value = nullptr;
    }
    operator bool()
    {
        return assigned;
    }
    optional(const optional<T> &o)
    {
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
    }
    optional(optional<T> &&o)
    {
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
    }
    optional(const T &o)
    {
        assigned = true;
        value = new T(o);
    }
    optional(T &&o)
    {
        assigned = true;
        value = new T(o);
    }
    ~optional()
    {
        if (assigned)
            delete value;
    }
    optional &operator = (const optional<T>& o)
    {
        if (assigned)
            delete value;
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
        return *this;
    }
    optional &operator = (optional<T>&& o)
    {
        if (assigned)
            delete value;
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
        return *this;
    }
    optional<T> &operator=(T newval)
    {
        value = new T(newval);
        assigned = true;
        return *this;
    }
    optional<T> &operator=(std::initializer_list<int> l)
    {
        if (assigned)
            delete value;
        assigned = false;
        return *this;
    }
    T &operator*()
    {
        return *value;
    }
    T* operator->()
    {
        return value;
    }
};
#endif*/