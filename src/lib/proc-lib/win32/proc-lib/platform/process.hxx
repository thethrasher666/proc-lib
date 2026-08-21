//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <proc-lib/process.hxx>

#include <cstdint>
#include <expected>

#include <Windows.h>

namespace pl
{
    /// The kind of raw exit status reported by the OS for a completed process.
    enum class PlatformExitKind
    {
        Exited,  ///< The process ran to completion, or was force-stopped with an OS-reported exit code.
        Signaled ///< The process was stopped by a signal, with no OS-reported exit code. Never reported on Windows.
    };

    /// The raw exit status reported by the OS for a completed process. Deliberately does not know
    /// whether a stop was requested by the caller (terminate()/kill()) or not — only Process itself
    /// tracks that, so it can distinguish a requested stop from a crash.
    struct PlatformExitStatus
    {
        PlatformExitKind kind = PlatformExitKind::Exited; ///< The kind of exit status reported by the OS.
        int32_t          value = -1;                      ///< The exit code if kind == Exited; the signal number if kind == Signaled.
    };

    /// A platform-specific process implementation.
    class PlatformProcess
    {
    public:
        /// Destructor.
        ~PlatformProcess();

        /// Start the process.
        /// \param executablePath Fully qualified path of the exectuable to run.
        /// \param arguments The arguments to launch the process with.
        /// \return An error code, if any.
        [[nodiscard]] auto start(std::filesystem::path const& executablePath, std::vector<std::string> const& arguments) -> std::error_code;

        /// Ask the process to stop gracefully. Windows has no general-purpose graceful-stop primitive
        /// for arbitrary processes, so this behaves the same as kill().
        /// \return An error code, if any.
        [[nodiscard]] auto terminate() -> std::error_code;

        /// Stop the process immediately. See Process::kill().
        /// \param exitCode The exit code to report for the process.
        /// \return An error code, if any.
        [[nodiscard]] auto kill(uint32_t const exitCode) -> std::error_code;

        /// Wait for the process to complete.
        /// \return The raw OS-reported exit status, or an error code.
        [[nodiscard]] auto wait() -> std::expected<PlatformExitStatus, std::error_code>;

    private:
        HANDLE _processHandle{}; ///< The process handle.
    };
} // namespace pl
