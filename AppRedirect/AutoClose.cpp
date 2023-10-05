#include "framework.h"
#include "AutoClose.h"

//----------------------------------------------------------------------

template<>
static void AutoCloseTraits<HANDLE>::SetInvalid(HANDLE& value)
{
    value = INVALID_HANDLE_VALUE;
}

template<>
static bool AutoCloseTraits<HANDLE>::IsValid(const HANDLE& value)
{
    return value != nullptr && value != INVALID_HANDLE_VALUE;
}

template<>
static void AutoCloseTraits<HANDLE>::Release(HANDLE& value)
{
    if (IsValid(value))
    {
        CloseHandle(value);
        SetInvalid(value);
    }
}
