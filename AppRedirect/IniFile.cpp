#include "framework.h"
#include "IniFile.h"
#include "WString.h"

//----------------------------------------------------------------------

static const wchar_t* StringNotSet1 = L"\x01\x02\x03\x04\x05\x06\x07\x08";
static const wchar_t* StringNotSet2 = L"\x08\x07\x06\x05\x04\x03\x02\x01";
static const DWORD StringNotSetLength = 8;

static const DWORD IntegerNotSet1 = 0xFEDCBA98;
static const DWORD IntegerNotSet2 = 0x98BADCFE;

//----------------------------------------------------------------------

bool CheckSettingSectionExists(const wchar_t* iniPath, const wchar_t* section)
{
    const DWORD bufferLength = StringNotSetLength + 8;
    wchar_t buffer[bufferLength];

    auto length = GetPrivateProfileString(section, NULL, NULL, buffer, bufferLength, iniPath);
    return length > 0;
}

std::optional<WString> ReadSettingString(const wchar_t* iniPath, const wchar_t* section, const wchar_t* setting)
{
    DWORD capacity = 64;
    WString value(capacity);

    auto length = GetPrivateProfileString(section, setting, StringNotSet1, value, capacity, iniPath);
    if (length == StringNotSetLength && wcsncmp(value, StringNotSet1, StringNotSetLength) == 0)
    {
        length = GetPrivateProfileString(section, setting, StringNotSet2, value, capacity, iniPath);
        if (length == StringNotSetLength && wcsncmp(value, StringNotSet2, StringNotSetLength) == 0)
            return std::nullopt;
    }

    while (length + 1 == capacity) {
        capacity *= 2;
        value.SetCapacity(capacity);

        length = GetPrivateProfileString(section, setting, NULL, value, capacity, iniPath);
    }

    return value;
}

std::optional<int> ReadSettingInt(const wchar_t* iniPath, const wchar_t* section, const wchar_t* setting)
{
    auto value = GetPrivateProfileInt(section, setting, IntegerNotSet1, iniPath);
    if (value == IntegerNotSet1)
    {
        auto checkValue = GetPrivateProfileInt(section, setting, IntegerNotSet2, iniPath);
        if (checkValue == IntegerNotSet2)
            return std::nullopt;
    }

    return static_cast<int>(value);
}

std::optional<bool> ReadSettingBool(const wchar_t* iniPath, const wchar_t* exeName, const wchar_t* setting)
{
    auto intValue = ReadSettingInt(iniPath, exeName, setting);
    if (!intValue.has_value())
        return std::nullopt;

    return intValue.value() != 0;
}
