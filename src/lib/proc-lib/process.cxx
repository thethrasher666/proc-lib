//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "error.hxx"
#include "process.hxx"
#include "proc-lib/platform/process.hxx"

namespace pl
{
    Process::Process()
    {
    }

    Process::~Process()
    {
    }

    auto Process::start(std::filesystem::path const& executablePath, std::vector<std::string> const& arguments) -> std::error_code
    {
        if (_platformProcess)
        {
            return makeErrorCode(ErrorCode::ProcessAlreadyStarted);
        }

        _platformProcess = std::make_unique<PlatformProcess>();
        return _platformProcess->start(executablePath, arguments);
    }

    auto Process::terminate() -> std::error_code
    {
        if (!_platformProcess)
        {
            return makeErrorCode(ErrorCode::ProcessNotStarted);
        }

        auto const error = _platformProcess->terminate();

        if (!error)
        {
            _stopRequest = StopRequest::Terminate;
        }

        return error;
    }

    auto Process::kill(uint32_t const exitCode) -> std::error_code
    {
        if (!_platformProcess)
        {
            return makeErrorCode(ErrorCode::ProcessNotStarted);
        }

        auto const error = _platformProcess->kill(exitCode);

        if (!error)
        {
            _stopRequest = StopRequest::Kill;
        }

        return error;
    }

    auto Process::wait() -> ProcessResult
    {
        if (!_platformProcess)
        {
            return ProcessResult{ .status = ProcessResult::Status::Error, .message = "The process has not been started." };
        }

        auto const result = _platformProcess->wait();

        if (!result)
        {
            return ProcessResult{ .status = ProcessResult::Status::Error, .message = result.error().message() };
        }

        // Only an OS-reported "exited" status carries a meaningful exit code; a signal has none.
        auto const reportedExitCode = result->kind == PlatformExitKind::Exited ? result->value : -1;

        // Whether we asked the process to stop takes priority over the raw OS status: on Windows a
        // forceful stop is reported the same way as a normal exit, so this is the only way to tell them apart.
        if (_stopRequest == StopRequest::Terminate)
        {
            return ProcessResult{ .status = ProcessResult::Status::Terminated, .exitCode = reportedExitCode };
        }

        if (_stopRequest == StopRequest::Kill)
        {
            return ProcessResult{ .status = ProcessResult::Status::Killed, .exitCode = reportedExitCode };
        }

        if (result->kind == PlatformExitKind::Exited)
        {
            return ProcessResult{
                .status = reportedExitCode == 0 ? ProcessResult::Status::Success : ProcessResult::Status::NonZeroExit,
                .exitCode = reportedExitCode,
            };
        }

        return ProcessResult{ .status = ProcessResult::Status::Crashed };
    }
} // namespace pl
