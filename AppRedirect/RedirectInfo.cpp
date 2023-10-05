#include "framework.h"
#include "RedirectInfo.h"
#include "WindowsPath.h"
#include "IniFile.h"
#include "StringSubstitution.h"
#include "AutoClose.h"
#include <queue>
#include <map>
#include <Shlwapi.h>

//----------------------------------------------------------------------

static const wchar_t* DefaultIniName = L"_redirect.ini";

static const wchar_t* DefaultSectionName = L"*";

static const wchar_t* SettingExeName = L"exe";
static const wchar_t* SettingExtraParams = L"args";
static const wchar_t* SettingWait = L"wait";
static const wchar_t* SettingCreateFlags = L"flags";
static const wchar_t* SettingTemplate = L"template";

//----------------------------------------------------------------------

bool IsKnownSection(const std::list<WString>& sectionNames, const WString& checkSection)
{
    for (auto item = sectionNames.begin();
        item != sectionNames.end();
        ++item)
    {
        if (_wcsicmp(checkSection, *item) == 0)
            return true;
    }

    return false;
}

//----------------------------------------------------------------------

struct RedirectSettings
{
    std::optional<WString> ExePath;
    std::optional<WString> ExtraParams;
    std::optional<bool> Wait;
    std::optional<uint32_t> CreateFlags;

    std::optional<WString> Template;
};

//----------------------------------------------------------------------

bool RedirectInfo::LoadSettings()
{
    auto exeFile = WindowsPath::CurrentExecutable();
    std::list<WString> sectionNames;

    std::stack<WindowsPath> settingsPaths;
    FindSettingsLocations(exeFile, settingsPaths, sectionNames);

    while (!settingsPaths.empty())
    {
        WindowsPath settingsPath = settingsPaths.top();
        settingsPaths.pop();

        RedirectSettings settings;
        LoadSettings(settingsPath, sectionNames, settings);
        
        if (!_exePath.has_value() && settings.ExePath.has_value())
            _exePath = settings.ExePath;
        if (!_extraParams.has_value() && settings.ExtraParams.has_value())
            _extraParams = settings.ExtraParams;
        if (!_wait.has_value() && settings.Wait.has_value())
            _wait = settings.Wait;
        if (!_createFlags.has_value() && settings.CreateFlags.has_value())
            _createFlags = settings.CreateFlags;

        if (_exePath.has_value() && _extraParams.has_value() && _wait.has_value() && _createFlags.has_value())
            break;

        if (settings.Template.has_value() && !IsKnownSection(sectionNames, settings.Template.value()))
        {
            settingsPaths.emplace(settingsPath);
            sectionNames.push_front(settings.Template.value());
        }
    }

    SubstituteEnvVars();
    
    return _exePath.has_value() && _exePath.value().Length() > 0;
}

void RedirectInfo::FindSettingsLocations(const WindowsPath& exeFile,
    std::stack<WindowsPath>& settingsPaths,
    std::list<WString>& sectionNames)
{
    WindowsPath currentExe(exeFile);

    do
    {
        sectionNames.push_back(currentExe.BaseName());

        WindowsPath iniFile(currentExe);

        iniFile.ReplaceFileExtension(L".ini");
        settingsPaths.push(iniFile);

        iniFile.ReplaceFileName(DefaultIniName);
        settingsPaths.push(iniFile);

    } while (currentExe.ResolveSymlink());
}

void RedirectInfo::LoadSettings(const WindowsPath& settingsPath, std::list<WString>& sectionNames, RedirectSettings& settings)
{
    std::list<WString> sections(sectionNames);
    sections.push_back(DefaultSectionName);

    for (auto iter = sections.begin(); iter != sections.end(); ++iter)
    {
        if (!settings.ExePath.has_value())
            settings.ExePath = ReadSettingString(settingsPath, *iter, SettingExeName);

        if (!settings.ExtraParams.has_value())
            settings.ExtraParams = ReadSettingString(settingsPath, *iter, SettingExtraParams);

        if (!settings.Wait.has_value())
            settings.Wait = ReadSettingString(settingsPath, *iter, SettingWait);

        if (!settings.CreateFlags.has_value())
            settings.CreateFlags = ReadSettingInt(settingsPath, *iter, SettingCreateFlags);

        if (!settings.Template.has_value())
            settings.Template = ReadSettingString(settingsPath, *iter, SettingTemplate);
    }
}

void RedirectInfo::SubstituteEnvVars()
{
    StringSubstitution subst;
    subst.AddEnvironmentVariables();

    if (_exePath.has_value())
    {
        WindowsPath exePath(_exePath.value());
        subst.AddSubstitution(L"ExePath", exePath.Path());
        subst.AddSubstitution(L"ExeName", exePath.BaseName());

        exePath.RemoveFileName();
        subst.AddSubstitution(L"ExeDir", exePath.Path());
    }

    if (_extraParams.has_value())
        _extraParams = subst.PerformSubstitution(_extraParams.value());
}

void RedirectInfo::RunProgram(const wchar_t* originalArguments)
{
    WString commandLine(32768);
    commandLine.Concatenate(L"\"");
    commandLine.Concatenate(_exePath.value());
    commandLine.Concatenate(L"\" ");
    
    if (_extraParams.has_value()) {
        commandLine.Concatenate(_extraParams.value());
        commandLine.Concatenate(L" ");
    }

    commandLine.Concatenate(originalArguments);

    STARTUPINFO startupInfo = { sizeof(STARTUPINFO), };
    PROCESS_INFORMATION processInfo;

    DWORD flags = 0;
    if (_createFlags.has_value())
        flags = _createFlags.value();

    CreateProcess(
        _exePath.value(),
        commandLine,
        NULL,
        NULL,
        TRUE,
        flags,
        NULL,
        NULL,
        &startupInfo,
        &processInfo);

    AutoClose<HANDLE> hThread = (HANDLE)processInfo.hThread;
    AutoClose<HANDLE> hProcess = (HANDLE)processInfo.hProcess;

    bool wait = true;
    if (_wait.has_value())
        wait = _wait.value();
    if (wait)
        WaitForSingleObject(processInfo.hProcess, INFINITE);
}
