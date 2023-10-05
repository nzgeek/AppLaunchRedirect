#pragma once

class WString
{
public:
    static const size_t NoIndex;

public:
    WString();
    WString(size_t capacity);
    WString(const wchar_t* string);
    WString(const wchar_t* string, size_t length);
    WString(const wchar_t* firstChar, const wchar_t* lastChar);
    WString(const WString& other);
    WString(WString&& other) noexcept;
    ~WString();

    operator wchar_t* ();
    operator const wchar_t* () const;

    wchar_t& operator [](size_t index);
    const wchar_t& operator [](size_t index) const;

    WString& operator = (const wchar_t* str);
    WString& operator = (const WString& str);

    size_t Capacity() const;
    void SetCapacity();
    void SetCapacity(size_t capacity);
    void SetCapacity(size_t capacity, bool keepContent);

    size_t Length() const;

    void Concatenate(const wchar_t* string);

    void CopyFrom(const wchar_t* string);
    void CopyFrom(const wchar_t* string, size_t length);
    void CopyFrom(const wchar_t* firstChar, const wchar_t* lastChar);

    size_t IndexOf(const wchar_t* value, const size_t startIndex = 0);

    WString Substring(size_t index, size_t length);

    void Splice(size_t index, size_t removeLength, const WString& insert);

private:
    size_t _capacity;
    wchar_t* _string;
};

inline
WString::operator wchar_t* ()
{
    return _string;
}

inline
WString::operator const wchar_t* () const
{
    return _string;
}

inline
wchar_t& WString::operator[](size_t index)
{
    return _string[index];
}

inline
const wchar_t& WString::operator[](size_t index) const
{
    return _string[index];
}
