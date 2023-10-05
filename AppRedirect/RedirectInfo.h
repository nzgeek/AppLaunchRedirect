#pragma once
#include "WString.h"
#include <optional>
#include <list>
#include <stack>

class WindowsPath;

struct RedirectSettingsLocation;
struct RedirectSettings;

class RedirectInfo
{
public:
    RedirectInfo() = default;
    ~RedirectInfo() = default;

public:
    bool LoadSettings();
private:
    void FindSettingsLocations(const WindowsPath& exeFile,
        std::stack<WindowsPath>& settingsPaths,
        std::list<WString>& sectionNames);

    void LoadSettings(const WindowsPath& settingsPath,
        std::list<WString>& sectionNames,
        RedirectSettings& settings);

    void SubstituteEnvVars();

public:
    void RunProgram(const wchar_t* originalArguments);

private:
    std::optional<WString> _exePath;
    std::optional<WString> _extraParams;
    std::optional<bool> _wait;
    std::optional<DWORD> _createFlags;
};
