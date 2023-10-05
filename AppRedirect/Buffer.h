#pragma once
#include <utility>

//----------------------------------------------------------------------

template<typename T = unsigned char>
class Buffer
{
public:
    Buffer(size_t count);
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    ~Buffer();

    operator T* ();
    operator const T* () const;

    T& operator [] (size_t index);
    const T& operator [] (size_t index) const;

    size_t Count() const;

    void Resize(size_t count, bool keepData = false);

    template<typename TOther> TOther* Cast();
    template<typename TOther> const TOther* Cast() const;

protected:
    T* _ptr;
    size_t _count;
};

//----------------------------------------------------------------------

template<typename T>
inline
Buffer<T>::Buffer(size_t count)
    : _count(count)
{
    _ptr = new T[count];
}

template<typename T>
inline
Buffer<T>::Buffer(Buffer&& other) noexcept
    : _ptr(nullptr)
    , _count(0)
{
    std::swap(_ptr, other._ptr);
    std::swap(_count, other._count);
}

template<typename T>
inline
Buffer<T>::~Buffer()
{
    if (_ptr != nullptr)
    {
        delete[] _ptr;
        _ptr = nullptr;
    }
}

template<typename T>
inline
Buffer<T>::operator T* ()
{
    return _ptr;
}

template<typename T>
inline
Buffer<T>::operator const T* () const
{
    return _ptr;
}

template<typename T>
inline
T& Buffer<T>::operator [] (size_t index)
{
    return _ptr[index];
}

template<typename T>
inline
const T& Buffer<T>::operator [] (size_t index) const
{
    return _ptr[index];
}

template<typename T>
inline
size_t Buffer<T>::Count() const
{
    return _count;
}

template<typename T>
inline
void Buffer<T>::Resize(size_t count, bool keepData)
{
    T* ptr = new T[count];
    std::swap(_ptr, ptr);
    std::swap(_count, count);

    if (ptr != nullptr && keepData)
    {
        for (size_t i = 0; i < _count && i < count; ++i)
            _ptr[i] = ptr[i];

        delete[] ptr;
    }
}

template<typename T>
template<typename TOther>
TOther* Buffer<T>::Cast()
{
    return reinterpret_cast<TOther*>(_ptr);
}

template<typename T>
template<typename TOther>
const TOther* Buffer<T>::Cast() const
{
    return reinterpret_cast<TOther*>(_ptr);
}

//----------------------------------------------------------------------

