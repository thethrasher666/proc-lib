//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

namespace pl
{
    /// Describes the outcome of launching and running a process.
    struct ProcessResult
    {
        /// The final status of the process.
        enum class Status
        {
            Success,       ///< The process ran to completion and exited with code 0.
            NonZeroExit,   ///< The process ran to completion but exited with a non-zero code.
            FailedToStart, ///< The process could not be started.
            Terminated,    ///< The process was asked to stop gracefully, via terminate(), and did.
            Killed,        ///< The process was stopped forcibly, via kill().
            Crashed,       ///< The process ended unexpectedly (e.g. an unrequested signal or fault), not via terminate()/kill().
            Error          ///< An unexpected error occurred while managing the process.
        };

        Status      status = Status::Error; ///< The process completion status.
        int32_t     exitCode = -1;          ///< The exit code returned by the process. A value of -1 indicates that no exit code was available.
        std::string message;                ///< Additional information about a process failure.

        /// Determines whether the process completed successfully.
        /// \return true if the process exited successfully; otherwise false.
        [[nodiscard]] auto succeeded() const -> bool
        {
            return status == Status::Success;
        }
    };

    class PlatformProcess; ///< Forward declaration.

    /// Represents a platform-independent process.
    class Process
    {
    public:
        /// Constructor.
        Process();

        /// Destructor.
        ~Process();

        /// Delete copy operation.
        Process(Process const&) = delete;

        /// Delete copy operation.
        Process& operator=(Process const&) = delete;

        /// Delete copy operation.
        Process(Process&&) = delete;

        /// Delete copy operation.
        Process& operator=(Process&&) = delete;

        /// Start the process.
        /// \param executablePath Fully qualified path of the exectuable to run.
        /// \param arguments The arguments to launch the process with.
        /// \return An error code, if any.
        [[nodiscard]] auto start(std::filesystem::path const& executablePath, std::vector<std::string> const& arguments) -> std::error_code;

        /// Ask the process to stop gracefully, giving it a chance to shut down cleanly (e.g. SIGTERM on POSIX).
        /// On platforms with no such mechanism (e.g. Windows), this behaves the same as kill().
        /// \return An error code, if any.
        [[nodiscard]] auto terminate() -> std::error_code;

        /// Stop the process immediately, without giving it a chance to shut down cleanly (e.g. SIGKILL on POSIX,
        /// TerminateProcess on Windows).
        /// \param exitCode The exit code to report for the process. Only honored on platforms where the OS lets
        ///                 the caller dictate it (e.g. Windows); ignored elsewhere (e.g. POSIX signals carry no exit code).
        /// \return An error code, if any.
        [[nodiscard]] auto kill(uint32_t const exitCode = 1) -> std::error_code;

        /// Wait for the process to complete.
        /// \return The outcome of the process.
        [[nodiscard]] auto wait() -> ProcessResult;

    private:
        /// Which stop method, if any, was last requested by the caller. Recorded so that wait() can
        /// distinguish a requested stop (Terminated/Killed) from an unrequested one (Crashed) once the
        /// platform layer reports the process died from a signal/fault.
        enum class StopRequest
        {
            None,
            Terminate,
            Kill
        };

        std::unique_ptr<PlatformProcess> _platformProcess;             ///< Pointer to the platform-specific implementation.
        StopRequest                      _stopRequest = StopRequest::None; ///< The last stop method requested, if any.
    };
} // namespace pl
