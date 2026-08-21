//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "proc-lib/platform/process.hxx"

#include <cerrno>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

namespace pl
{
    namespace
    {
        auto lastErrorPosix() -> std::error_code
        {
            return std::error_code(errno, std::generic_category());
        }
    } // namespace

    PlatformProcess::~PlatformProcess()
    {
        if (_processId > 0)
        {
            // Do not implicitly terminate the child here. Just forget the PID.
            // The caller should normally wait() or terminate()/kill() explicitly.
            _processId = -1;
        }
    }

    auto PlatformProcess::start(std::filesystem::path const& executablePath, std::vector<std::string> const& arguments) -> std::error_code
    {
        pid_t const pid = ::fork();

        if (pid < 0)
        {
            return lastErrorPosix();
        }

        if (pid == 0)
        {
            // Child process.
            //
            // Build argv:
            //   argv[0] = executable
            //   argv[1..] = arguments
            //   argv[n] = nullptr

            std::vector<char*> argv;
            argv.reserve(arguments.size() + 2);

            argv.push_back(const_cast<char*>(executablePath.generic_string().c_str()));

            for (auto const& argument : arguments)
            {
                argv.push_back(const_cast<char*>(argument.c_str()));
            }

            argv.push_back(nullptr);

            // Use the supplied executable path directly. No shell is involved.
            ::execv(executablePath.c_str(), argv.data());

            // Only reached if execv() failed.
            //
            // We cannot return an error to the parent through the normal
            // return path, so terminate the child with a conventional
            // exec-failure status.
            ::_exit(127);
        }

        // Parent process.
        _processId = pid;

        return {};
    }

    auto PlatformProcess::terminate() -> std::error_code
    {
        if (_processId <= 0)
        {
            return std::make_error_code(std::errc::no_such_process);
        }

        if (::kill(_processId, SIGTERM) != 0)
        {
            return lastErrorPosix();
        }

        return {};
    }

    auto PlatformProcess::kill(uint32_t const exitCode) -> std::error_code
    {
        // POSIX signals carry no exit code; the OS alone decides how a signaled process is reported.
        static_cast<void>(exitCode);

        if (_processId <= 0)
        {
            return std::make_error_code(std::errc::no_such_process);
        }

        if (::kill(_processId, SIGKILL) != 0)
        {
            return lastErrorPosix();
        }

        return {};
    }

    auto PlatformProcess::wait() -> std::expected<PlatformExitStatus, std::error_code>
    {
        if (_processId <= 0)
        {
            return std::unexpected(std::make_error_code(std::errc::no_such_process));
        }

        int32_t status{};

        pid_t const result = ::waitpid(_processId, &status, 0);

        if (result < 0)
        {
            return std::unexpected(lastErrorPosix());
        }

        _processId = -1;

        if (WIFEXITED(status))
        {
            return PlatformExitStatus{ .kind = PlatformExitKind::Exited, .value = WEXITSTATUS(status) };
        }

        if (WIFSIGNALED(status))
        {
            return PlatformExitStatus{ .kind = PlatformExitKind::Signaled, .value = WTERMSIG(status) };
        }

        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
} // namespace pl
