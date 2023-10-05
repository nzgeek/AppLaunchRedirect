#pragma once
#include <optional>

class WString;

bool CheckSettingSectionExists(const wchar_t* iniPath, const wchar_t* section);
std::optional<WString> ReadSettingString(const wchar_t* iniPath, const wchar_t* section, const wchar_t* setting);
std::optional<int> ReadSettingInt(const wchar_t* iniPath, const wchar_t* section, const wchar_t* setting);
std::optional<bool> ReadSettingBool(const wchar_t* iniPath, const wchar_t* exeName, const wchar_t* setting);
