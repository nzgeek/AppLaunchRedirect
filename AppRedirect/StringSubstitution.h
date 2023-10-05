#pragma once
#include <map>
#include "WString.h"

struct StringSubstitutionKeyComparer
{
    bool operator ()(const WString& lhs, const WString& rhs) const
    {
        return _wcsicmp(lhs, rhs) < 0;
    }
};

class StringSubstitution
{
public:
    StringSubstitution();

public:
    void AddEnvironmentVariables();
    void AddSubstitution(const WString& key, const WString& value);

    WString PerformSubstitution(const WString& str);

private:
    std::map<WString, WString, StringSubstitutionKeyComparer> _substitutions;
};

