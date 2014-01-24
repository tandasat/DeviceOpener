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
    auto scoped = make_unique_ex(message, &::LocalFree);

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
    DesiredAccessInvalid,
    DesiredAccessRead,
    DesiredAccessWrite,
    DesiredAccessReadWrite,
};


HANDLE OpenDevice(
    __in const std::string& DeviceName,
    __in DesiredAccess Access)
{
    const auto fullName = "\\\\.\\" + DeviceName;

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

    return ::CreateFileA(
        fullName.c_str(),
        access,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
}


DesiredAccess ConvertToDesiredAccess(
    const std::string& DesiredAccess)
{
    if (DesiredAccess == "r")
    {
        return DesiredAccessRead;
    }
    else if (DesiredAccess == "w")
    {
        return DesiredAccessWrite;
    }
    else if (DesiredAccess == "rw")
    {
        return DesiredAccessReadWrite;
    }
    else
    {
        return DesiredAccessInvalid;
    }
}


void IoControlShell(
    const std::string& DeviceName,
    HANDLE DeviceHandle)
{
    std::cout
        << "Type a device IO control code in hex or 'exit' to end the shell.\n"
        << "e.g. "
        << DeviceName
        << "> 5360e0c8" << std::endl;
    for (;;)
    {
        std::cout << DeviceName << "> ";
        std::string input;
        std::cin >> input;
        if (input == "exit")
        {
            return;
        }
        const auto ioCtlCode = std::stoul(input, nullptr, 16);

        std::vector<std::uint8_t> buffer(0x1000, 0x41);
        DWORD returned = 0;
        const auto succeeded = ::DeviceIoControl(
            DeviceHandle, ioCtlCode,
            buffer.data(), buffer.size(),
            buffer.data(), buffer.size(),
            &returned, nullptr);
        std::cout
            << succeeded
            << " : "
            << GetErrorMessage(::GetLastError()) << std::endl;
    }
}


__declspec(noreturn)
void AppMain()
{
    std::cout
        << "Type a decive name without '\\\\.\\' and a code of desired access.\n"
        << " r  = Read\n"
        << " w  = Write\n"
        << " rw = ReadWrite\n"
        << "e.g.> MyDriver\n"
        << "e.g.> w" << std::endl;
    for (;;)
    {
        std::cout << "> ";
        std::string deviceName;
        std::cin >> deviceName;

        std::cout << "> ";
        std::string desiredAccessStr;
        std::cin >> desiredAccessStr;

        const auto desiredAccess = ConvertToDesiredAccess(desiredAccessStr);
        if (desiredAccess == DesiredAccessInvalid)
        {
            std::cout << "Invalid access code" << std::endl;
            continue;
        }

        auto handle = make_unique_ex(OpenDevice(deviceName, desiredAccess),
            &::CloseHandle);
        const auto error = ::GetLastError();
        std::cout << error << " : " << GetErrorMessage(error) << std::endl;
        std::cout << std::hex << handle.get()
            << " = CreateFile()\n" << std::endl;
        if (handle.get() != INVALID_HANDLE_VALUE)
        {
            IoControlShell(deviceName, handle.get());
        }
    }
}


} // End of namespace {unnamed}


int _tmain()
{
    try
    {
        AppMain();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return EXIT_FAILURE;
}

