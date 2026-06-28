#pragma once

#include <string>
#include <variant>

namespace remote_sqlite_qt {

// 错误码枚举
enum class ErrorCode {
    Ok = 0,

    // SSH 错误
    SshConnectionFailed,
    SshAuthenticationFailed,
    SshChannelError,
    SshTimeout,
    SshUnknownHost,

    // SFTP 错误
    SftpFileNotFound,
    SftpPermissionDenied,
    SftpTransferFailed,
    SftpAlreadyExists,

    // SQLite 错误
    SqliteError,
    SqliteBusy,
    SqliteConstraintViolation,

    // 同步错误
    SyncConflict,
    SyncChecksumMismatch,

    // 通用错误
    InvalidArgument,
    NotFound,
    NotSupported,
    InternalError,
};

// 获取 ErrorCode 的可读描述
const char* errorCodeToString(ErrorCode code) noexcept;

}  // namespace remote_sqlite_qt
