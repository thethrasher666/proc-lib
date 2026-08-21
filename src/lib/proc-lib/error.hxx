//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <system_error>

namespace pl
{
    /// The possible errors relating to the reading or writing of PAK files.
    enum class ErrorCode
    {
        NoError,               ///< No error has occurred.
        ProcessAlreadyStarted, ///< The process has already been started and cannot be started again.
        ProcessNotStarted      ///< The process has not been started.
    };

    /// Create an error code.
    /// \param code The pak-io related error code.
    /// \return A valid error code.
    [[nodiscard]] auto makeErrorCode(ErrorCode const code) -> std::error_code;
} // namespace pl
