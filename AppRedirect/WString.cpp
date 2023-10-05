#include "framework.h"
#include "WString.h"
#include <utility>

static const size_t CapacityBlockSize = 8;
static const size_t CapacityShrinkThreshold = 32;

size_t ActualCapacity(size_t requestedCapacity)
{
    if (requestedCapacity <= CapacityBlockSize)
        return CapacityBlockSize;

    if (requestedCapacity % CapacityBlockSize == 0)
        return requestedCapacity;

    return ((requestedCapacity / CapacityBlockSize) + 1) * CapacityBlockSize;
}

const size_t WString::NoIndex = ~0UL;

WString::WString()
    : WString(CapacityBlockSize)
{
}

WString::WString(size_t capacity)
    : _capacity(ActualCapacity(capacity))
    , _string(nullptr)
{
    _string = new wchar_t[_capacity];
    _string[0] = L'\0';
}

WString::WString(const wchar_t* string)
    : _capacity(0)
    , _string(nullptr)
{
    CopyFrom(string);
}

WString::WString(const wchar_t* string, size_t length)
    : _capacity(0)
    , _string(nullptr)
{
    CopyFrom(string, length);
}

WString::WString(const wchar_t* firstChar, const wchar_t* lastChar)
    : _capacity(0)
    , _string(nullptr)
{
    CopyFrom(firstChar, lastChar);
}

WString::WString(const WString& other)
    : _capacity(other._capacity)
    , _string(nullptr)
{
    _string = new wchar_t[_capacity];
    _tcscpy_s(_string, _capacity, other._string);
}

WString::WString(WString&& other) noexcept
    : _capacity(other._capacity)
    , _string(nullptr)
{
    _string = std::move(other._string);
    other._capacity = 0;
    other._string = nullptr;
}

WString::~WString()
{
    if (_string != nullptr) {
        delete[] _string;
        _string = nullptr;
    }
    _capacity = 0;
}

WString& WString::operator = (const wchar_t* str)
{
    CopyFrom(str);
    return *this;
}

WString& WString::operator = (const WString& str)
{
    auto length = str.Length();
    if (_capacity < length + 1)
        SetCapacity(length + 1, false);

    wcscpy_s(_string, _capacity, str._string);
    return *this;
}

size_t WString::Capacity() const
{
    return _capacity;
}

void WString::SetCapacity()
{
    auto length = Length();
    SetCapacity(length + 1);
}

void WString::SetCapacity(size_t capacity)
{
    SetCapacity(capacity, true);
}

void WString::SetCapacity(size_t capacity, bool keepContent)
{
    capacity = ActualCapacity(capacity);
    if (capacity < _capacity && _capacity - capacity <= CapacityShrinkThreshold)
        return;

    wchar_t* string = new wchar_t[capacity];
    std::swap(_string, string);
    std::swap(_capacity, capacity);

    _string[0] = L'\0';

    if (string != nullptr)
    {
        if (keepContent)
        {
            wcscpy_s(_string, _capacity, string);
            _string[_capacity - 1] = L'\0';
        }

        delete[] string;
    }
}

size_t WString::Length() const
{
    if (_capacity == 0 || _string == nullptr)
        return 0;

    return wcsnlen(_string, _capacity);
}

void WString::Concatenate(const wchar_t* string)
{
    if (string == nullptr)
        return;

    size_t ownLen = Length();
    size_t catLen = _tcslen(string);

    SetCapacity(ownLen + catLen + 1);
    wcscat_s(_string, _capacity, string);
}

void WString::CopyFrom(const wchar_t* string)
{
    size_t length = wcslen(string);
    CopyFrom(string, length);
}

void WString::CopyFrom(const wchar_t* string, size_t length)
{
    auto capacity = ActualCapacity(length + 1);
    if (capacity > _capacity)
        SetCapacity(capacity, false);

    wcsncpy_s(_string, _capacity, string, length);
}

void WString::CopyFrom(const wchar_t* firstChar, const wchar_t* lastChar)
{
    size_t length = 1 + lastChar - firstChar;
    CopyFrom(firstChar, length);
}

size_t WString::IndexOf(const wchar_t* value, const size_t startIndex)
{
    const wchar_t* start = _string + startIndex;
    const wchar_t* found = wcsstr(start, value);
    return (found == nullptr)
        ? NoIndex
        : found - _string;
}

WString WString::Substring(size_t index, size_t length)
{
    size_t maxLength = Length();
    if (index >= maxLength)
        return WString();

    if (index + length > maxLength)
        length = maxLength - index;

    return WString(_string + index, length);
}

void WString::Splice(size_t index, size_t removeLength, const WString& insert)
{
    size_t length = Length();
    WString tail;
    if (index + removeLength < length)
        tail.CopyFrom(_string + index + removeLength, length - (index + removeLength));

    if (index < length)
        _string[index] = 0;

    Concatenate(insert);
    Concatenate(tail);
}
