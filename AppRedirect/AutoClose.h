#pragma once
#include <exception>
#include <utility>

//----------------------------------------------------------------------

template<typename T>
struct AutoCloseTraits
{
    static void SetInvalid(T& value);
    static bool IsValid(const T& value);
    static void Release(T& value);
};

//----------------------------------------------------------------------

template<typename T, typename TTraits = AutoCloseTraits<T>>
class AutoClose
{
    using ValueType = T;
    using TraitsType = TTraits;

public:
    AutoClose()
    {
        TTraits::SetInvalid(_value);
    }

    AutoClose(T&& value)
    {
        _value = std::move(value);
    }

    AutoClose(const AutoClose<T>&) = delete;

    ~AutoClose()
    {
        Reset();
    }

    operator const T& () const
    {
        return _value;
    }

    bool IsValid() const
    {
        return TTraits::IsValid(_value);
    }

    void Assign(T&& value)
    {
        Reset();
        _value = std::move(value);
    }

    void Reset()
    {
        if (TTraits::IsValid(_value))
        {
            TTraits::Release(_value);
            TTraits::SetInvalid(_value);
        }
    }

private:
    T _value;
};

//----------------------------------------------------------------------

template<typename T>
struct AutoDeleteTraits
{
    static void SetInvalid(T*& value)
    {
        value = nullptr;
    }

    static bool IsValid(const T*& value)
    {
        return value != nullptr;
    }

    static void Release(T*& value)
    {
        delete[] value;
        value = nullptr;
    }
};

template<typename T>
using AutoDelete = AutoClose<T, AutoDeleteTraits<T>>;
