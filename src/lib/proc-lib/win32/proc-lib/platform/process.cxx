//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "proc-lib/platform/process.hxx"

namespace pl
{
    namespace
    {
        auto lastErrorWin32() -> std::error_code
        {
            return std::error_code(static_cast<int32_t>(::GetLastError()), std::system_category());
        }

        [[nodiscard]] auto utf8ToWide(std::string const& value) -> std::expected<std::wstring, std::error_code>
        {
            if (value.empty())
            {
                return std::wstring{};
            }

            auto const length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int32_t>(value.size()), nullptr, 0);

            if (length == 0)
            {
                return std::unexpected(lastErrorWin32());
            }

            std::wstring result(static_cast<std::size_t>(length), L'\0');

            if (::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int32_t>(value.size()), result.data(), length) == 0)
            {
                return std::unexpected(lastErrorWin32());
            }

            return result;
        }

        auto appendQuotedArgument(std::string& commandLine, std::string const& argument) -> void
        {
            bool const needsQuoting = argument.empty() || argument.find_first_of(" \t\n\v\"") != std::string::npos;

            if (!needsQuoting)
            {
                commandLine.append(argument);
                return;
            }

            commandLine.push_back('"');

            for (auto it = argument.begin();; ++it)
            {
                std::size_t numBackslashes = 0;

                while (it != argument.end() && *it == '\\')
                {
                    ++it;
                    ++numBackslashes;
                }

                if (it == argument.end())
                {
                    commandLine.append(numBackslashes * 2, '\\');
                    break;
                }

                if (*it == '"')
                {
                    commandLine.append(numBackslashes * 2 + 1, '\\');
                    commandLine.push_back('"');
                }
                else
                {
                    commandLine.append(numBackslashes, '\\');
                    commandLine.push_back(*it);
                }
            }

            commandLine.push_back('"');
        }

        [[nodiscard]] auto buildCommandLine(std::string const& executablePath, std::vector<std::string> const& arguments) -> std::string
        {
            std::string commandLine;
            appendQuotedArgument(commandLine, executablePath);

            for (auto const& argument : arguments)
            {
                commandLine.push_back(' ');
                appendQuotedArgument(commandLine, argument);
            }

            return commandLine;
        }
    } // namespace

    PlatformProcess::~PlatformProcess()
    {
        if (_processHandle)
        {
            ::CloseHandle(_processHandle);
        }
    }

    auto PlatformProcess::start(std::filesystem::path const& executablePath, std::vector<std::string> const& arguments) -> std::error_code
    {
        auto const wExecutablePath = utf8ToWide(executablePath.generic_string());

        if (!wExecutablePath)
        {
            return wExecutablePath.error();
        }

        auto const wCommandLine = utf8ToWide(buildCommandLine(executablePath.generic_string(), arguments));

        if (!wCommandLine)
        {
            return wCommandLine.error();
        }

        // CreateProcessW may write into lpCommandLine in place, so it must be a mutable, writable buffer.
        std::wstring commandLineBuffer(*wCommandLine);

        PROCESS_INFORMATION processInfo{};
        STARTUPINFOW        startupInfo{};
        startupInfo.cb = sizeof(startupInfo);

        if (!::CreateProcessW(wExecutablePath->c_str(), commandLineBuffer.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo))
        {
            return lastErrorWin32();
        }

        ::CloseHandle(processInfo.hThread);
        _processHandle = processInfo.hProcess;
        return {};
    }

    auto PlatformProcess::terminate() -> std::error_code
    {
        // Windows has no general-purpose graceful-stop primitive for arbitrary processes.
        return kill(1);
    }

    auto PlatformProcess::kill(uint32_t const exitCode) -> std::error_code
    {
        if (!::TerminateProcess(_processHandle, exitCode))
        {
            return lastErrorWin32();
        }
        return {};
    }

    auto PlatformProcess::wait() -> std::expected<PlatformExitStatus, std::error_code>
    {
        auto const waitResult = ::WaitForSingleObject(_processHandle, INFINITE);

        if (waitResult == WAIT_FAILED)
        {
            return std::unexpected(lastErrorWin32());
        }

        if (waitResult != WAIT_OBJECT_0)
        {
            return std::unexpected(std::make_error_code(std::errc::io_error));
        }

        DWORD exitCode{};

        if (!::GetExitCodeProcess(_processHandle, &exitCode))
        {
            return std::unexpected(lastErrorWin32());
        }

        return PlatformExitStatus{ .kind = PlatformExitKind::Exited, .value = static_cast<int32_t>(exitCode) };
    }
} // namespace pl
