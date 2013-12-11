#include "stdafx.h"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


// C/C++ standard headers
// Other external headers
// Windows headers
// Original headers


////////////////////////////////////////////////////////////////////////////////
//
// macro utilities
//


////////////////////////////////////////////////////////////////////////////////
//
// constants and macros
//


////////////////////////////////////////////////////////////////////////////////
//
// types
//


////////////////////////////////////////////////////////////////////////////////
//
// prototypes
//


////////////////////////////////////////////////////////////////////////////////
//
// variables
//


////////////////////////////////////////////////////////////////////////////////
//
// implementations
//

namespace {


std::string GetErrorMessage(
    __in DWORD ErrorCode)
{
    char* message = nullptr;
    if (!::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, ErrorCode, LANG_USER_DEFAULT,
        reinterpret_cast<LPSTR>(&message), 0, nullptr))
    {
        return "";
    }
    auto scoped = make_unique_ptr(message, &::LocalFree);

    const auto length = ::strlen(message);
    if (!length)
    {
        return "";
    }

    if (message[length - 2] == '\r')
    {
        message[length - 2] = '\0';
    }
    return message;
}


enum DesiredAccess
{
    DesiredAccessRead,
    DesiredAccessWrite,
    DesiredAccessReadWrite
};


HANDLE OpenDevice(
    __in const std::wstring& DeviceName,
    __in DesiredAccess Access)
{
    const auto fullName = L"\\\\.\\" + DeviceName;

    DWORD access = 0;
    switch (Access)
    {
    case DesiredAccessRead:
        access = GENERIC_READ;
        break;
    case DesiredAccessWrite:
        access = GENERIC_WRITE;
        break;
    case DesiredAccessReadWrite:
        access = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        throw std::runtime_error("Invalid Parameter");
    }

    return ::CreateFileW(
        fullName.c_str(),
        access,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
}


} // End of namespace {unnamed}


int _tmain()
{
    std::wcout << L"Type a decive name without '\\\\.\\'" << std::endl;
    std::wcout << L"e.g.> MyDriver" << std::endl;
    for (;;)
    {
        std::wcout << L"> ";
        std::wstring name;
        std::wcin >> name;

        auto handle = make_unique_ptr(OpenDevice(name, DesiredAccessRead),
            &::CloseHandle);
        std::cout << GetErrorMessage(::GetLastError()) << std::endl;
        std::cout << std::hex << handle.get()
            << " = CreateFileW(GENERIC_READ)\n" << std::endl;

        handle = make_unique_ptr(OpenDevice(name, DesiredAccessWrite),
            &::CloseHandle);
        std::cout << GetErrorMessage(::GetLastError()) << std::endl;
        std::cout << std::hex << handle.get()
            << " = CreateFileW(GENERIC_WRITE)\n" << std::endl;

        handle = make_unique_ptr(OpenDevice(name, DesiredAccessReadWrite),
            &::CloseHandle);
        std::cout << GetErrorMessage(::GetLastError()) << std::endl;
        std::cout << std::hex << handle.get()
            << " = CreateFileW(GENERIC_READ | GENERIC_WRITE)\n" << std::endl;
    }
}

