#include "framework.h"
#include "WindowsPath.h"
#include "Buffer.h"
#include "AutoClose.h"
#include "WString.h"
#include <winioctl.h>
#include <shlwapi.h>
#include <PathCch.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "pathcch.lib")

//----------------------------------------------------------------------

WString JoinPath(const wchar_t* basePath, const wchar_t* extraPath)
{
    WString path(8192);
    auto hr = PathCchCombineEx(path, path.Capacity(), basePath, extraPath, PATHCCH_ALLOW_LONG_PATHS);
    return path;
}

//----------------------------------------------------------------------

struct ReparseDataHeader
{
    DWORD ReparseTag;
    WORD ReparseDataLength;
    WORD Reserved;
};

struct ReparseSymlink
{
    WORD SubstituteNameOffset;
    WORD SubstituteNameLength;
    WORD PrintNameOffset;
    WORD PrintNameLength;
    ULONG Flags;
    wchar_t NameData[1];
};

struct ReparseMountPoint
{
    WORD SubstituteNameOffset;
    WORD SubstituteNameLength;
    WORD PrintNameOffset;
    WORD PrintNameLength;
    wchar_t NameData[1];
};

struct ReparseData
{
    ReparseDataHeader Header;
    union {
        ReparseSymlink Symlink;
        ReparseMountPoint MountPoint;
        BYTE RawData[1];
    };
};

const size_t MaxReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;

//----------------------------------------------------------------------

bool GetReparseData(const wchar_t* path, Buffer<unsigned char>& reparseData)
{
    AutoClose<HANDLE> hFile = CreateFile(path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL);

    if (!hFile.IsValid())
        return false;

    DWORD reparseLength = 0;
    BOOL ioctlResult = DeviceIoControl(hFile,
        FSCTL_GET_REPARSE_POINT,
        NULL, 0,
        reparseData, (DWORD)reparseData.Count(),
        &reparseLength,
        NULL);

    return ioctlResult != FALSE;
}

//----------------------------------------------------------------------

static const size_t MaxWindowsPathLength = 16384;

WindowsPath WindowsPath::CurrentExecutable()
{
    WindowsPath result;
    GetModuleFileName(NULL, result._path, MaxWindowsPathLength);
    result._pathType = PathType::File;
    return result;
}

WindowsPath WindowsPath::CurrentDirectory()
{
    WindowsPath result;
    GetCurrentDirectory(static_cast<DWORD>(MaxWindowsPathLength), result._path);
    result._pathType = PathType::Directory;
    return result;
}

WindowsPath::WindowsPath()
{
    _path = new wchar_t[MaxWindowsPathLength];
    _pathType = PathType::UnknownPathType;
}

WindowsPath::WindowsPath(const wchar_t* path)
    : WindowsPath()
{
    SetPath(path, PathType::UnknownPathType);
}

WindowsPath::WindowsPath(const wchar_t* path, PathType pathType)
    : WindowsPath()
{
    SetPath(path, pathType);
}

WindowsPath::WindowsPath(const WindowsPath& other)
    : WindowsPath()
{
    SetPath(other._path, other._pathType);
}

WindowsPath::WindowsPath(WindowsPath&& other) noexcept
    : _path(nullptr)
    , _pathType(PathType::UnknownPathType)
{
    std::swap(_path, other._path);
    std::swap(_pathType, other._pathType);
}

WindowsPath::~WindowsPath()
{
    if (_path != nullptr)
    {
        delete[] _path;
        _path = nullptr;
    }
}

WindowsPath::operator const wchar_t* () const
{
    return _path;
}

const wchar_t* WindowsPath::Path() const
{
    return _path;
}

PathType WindowsPath::Type() const
{
    return _pathType;
}

WString WindowsPath::BaseName() const
{
    auto fileName = FileName();
    auto lastDot = wcsrchr(fileName, L'.');
    if (lastDot != nullptr)
        *lastDot = L'\0';
    return fileName;
}

WString WindowsPath::FileName() const
{
    const wchar_t* fileName = PathFindFileName(_path);
    return WString(fileName);
}

void WindowsPath::SetPath(const wchar_t* path, PathType pathType)
{
    wcscpy_s(_path, MaxWindowsPathLength, path);
    _pathType = pathType;

    if (_pathType == PathType::UnknownPathType)
        GuessPathType();
}

void WindowsPath::CombineWith(const wchar_t* extraPath)
{
    if (_pathType == PathType::File)
        RemoveFileName();

    WString pathCopy(_path);
    PathCchCombineEx(_path, MaxWindowsPathLength, pathCopy, extraPath, PATHCCH_ALLOW_LONG_PATHS);
    GuessPathType();
}

void WindowsPath::RemoveFileName()
{
    if (_pathType != PathType::Directory)
    {
        PathCchRemoveFileSpec(_path, MaxWindowsPathLength);
        _pathType = PathType::Directory;
    }
}

void WindowsPath::ReplaceFileName(const wchar_t* newFileName)
{
    RemoveFileName();
    CombineWith(newFileName);
}

void WindowsPath::ReplaceFileExtension(const wchar_t* newExtension)
{
    PathCchRenameExtension(_path, MaxWindowsPathLength, newExtension);
}

bool WindowsPath::IsSymlink() const
{
    Buffer buffer(MaxReparseDataLength);
    if (!GetReparseData(_path, buffer))
        return false;

    return buffer.Cast<ReparseDataHeader>()->ReparseTag == IO_REPARSE_TAG_SYMLINK;
}

bool WindowsPath::ResolveSymlink()
{
    Buffer buffer(MaxReparseDataLength);
    if (!GetReparseData(_path, buffer))
        return false;

    auto reparseData = buffer.Cast<ReparseData>();
    if (reparseData->Header.ReparseTag != IO_REPARSE_TAG_SYMLINK)
        return false;

    auto targetStart = reparseData->Symlink.NameData + (reparseData->Symlink.SubstituteNameOffset / sizeof(wchar_t));
    WString target(targetStart, reparseData->Symlink.SubstituteNameLength / sizeof(wchar_t));

    CombineWith(target);
    return true;
}

void WindowsPath::GuessPathType()
{
    auto length = wcsnlen_s(_path, MaxWindowsPathLength);
    if (length == 0 || _path[length - 1] == L'\\' || PathIsDirectory(_path))
    {
        _pathType = PathType::Directory;
        return;
    }

    const wchar_t* fileName = PathFindFileName(_path);
    if (wcschr(fileName, L'.') == nullptr)
    {
        _pathType = PathType::Directory;
    }
    else
    {
        _pathType = PathType::File;
    }
}
