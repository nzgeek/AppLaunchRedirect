#include "framework.h"
#include "StringSubstitution.h"
#include "AutoClose.h"

//----------------------------------------------------------------------

struct AutoCloseEnvironment
{
    using ValueType = wchar_t*;

    static void SetInvalid(ValueType& value)
    {
        value = nullptr;
    }

    static bool IsValid(const ValueType& value)
    {
        return value != nullptr;
    }

    static void Release(ValueType& value)
    {
        FreeEnvironmentStrings(value);
    }
};

//----------------------------------------------------------------------

StringSubstitution::StringSubstitution()
{
}

void StringSubstitution::AddEnvironmentVariables()
{
    AutoClose<LPWCH, AutoCloseEnvironment> environment = GetEnvironmentStrings();

    wchar_t* ptr = environment;
    while (*ptr != L'\0')
    {
        size_t len = wcslen(ptr);
        wchar_t* value = wcschr(ptr + 1, L'=');

        if (*ptr != L'=' && value != nullptr)
        {
            *value = L'\0';
            _substitutions.insert_or_assign(ptr, ++value);
        }

        ptr += len + 1;
    }
}

void StringSubstitution::AddSubstitution(const WString& key, const WString& value)
{
    _substitutions.insert_or_assign(key, value);
}

WString StringSubstitution::PerformSubstitution(const WString& str)
{
    auto result = WString(str);

    for (size_t index = result.IndexOf(L"${");
        index != WString::NoIndex;
        index = result.IndexOf(L"${", index))
    {
        size_t end = result.IndexOf(L"}", index + 2);
        if (end == WString::NoIndex)
            break;

        auto length = end + 1 - index;

        auto key = result.Substring(index + 2, length - 3);
        auto value = _substitutions.find(key);

        if (value == _substitutions.end())
            ++index;
        else
            result.Splice(index, length, value->second);
    }

    return result;
}
