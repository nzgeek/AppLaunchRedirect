#pragma once
#include "WString.h"

typedef enum _PathType
{
    UnknownPathType = 0,
    Directory = 1,
    File = 2,
} PathType;

class WindowsPath
{
public:
    static WindowsPath CurrentExecutable();
    static WindowsPath CurrentDirectory();

private:
    WindowsPath();
public:
    WindowsPath(const wchar_t* path);
    WindowsPath(const wchar_t* path, PathType pathType);
    WindowsPath(const WindowsPath& other);
    WindowsPath(WindowsPath&& other) noexcept;
    ~WindowsPath();

public:
    operator const wchar_t* () const;
    const wchar_t* Path() const;
    PathType Type() const;

    WString BaseName() const;
    WString FileName() const;

    void SetPath(const wchar_t* path, PathType pathType);

    void CombineWith(const wchar_t* extraPath);

    void RemoveFileName();
    void ReplaceFileName(const wchar_t* newFileName);
    void ReplaceFileExtension(const wchar_t* newExtension);

    bool IsSymlink() const;
    bool ResolveSymlink();

private:
    void GuessPathType();

private:
    wchar_t* _path;
    PathType _pathType;
};


