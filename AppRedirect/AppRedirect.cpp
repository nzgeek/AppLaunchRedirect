// AppRedirect.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "RedirectInfo.h"
#include "WindowsPath.h"
#include <shlwapi.h>

int PerformRedirect(const wchar_t* arguments)
{
    RedirectInfo redirectInfo;
    if (redirectInfo.LoadSettings())
    {
        redirectInfo.RunProgram(arguments);
    }

    return 0;
}

#ifdef _CONSOLE

int wmain(int argc, wchar_t* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    WString commandLine(GetCommandLine());
    auto arguments = PathGetArgs(commandLine);

    return PerformRedirect(arguments);
}

#else

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    return PerformRedirect(lpCmdLine);
}

#endif
