#include "error_code.h"

namespace remote_sqlite_qt {

const char* errorCodeToString(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::Ok:
        return "OK";
    case ErrorCode::SshConnectionFailed:
        return "SSH connection failed";
    case ErrorCode::SshAuthenticationFailed:
        return "SSH authentication failed";
    case ErrorCode::SshChannelError:
        return "SSH channel error";
    case ErrorCode::SshTimeout:
        return "SSH operation timed out";
    case ErrorCode::SshUnknownHost:
        return "SSH unknown host";
    case ErrorCode::SftpFileNotFound:
        return "SFTP file not found";
    case ErrorCode::SftpPermissionDenied:
        return "SFTP permission denied";
    case ErrorCode::SftpTransferFailed:
        return "SFTP transfer failed";
    case ErrorCode::SftpAlreadyExists:
        return "SFTP file already exists";
    case ErrorCode::SqliteError:
        return "SQLite error";
    case ErrorCode::SqliteBusy:
        return "SQLite database is busy";
    case ErrorCode::SqliteConstraintViolation:
        return "SQLite constraint violation";
    case ErrorCode::SyncConflict:
        return "Sync conflict detected";
    case ErrorCode::SyncChecksumMismatch:
        return "Sync checksum mismatch";
    case ErrorCode::InvalidArgument:
        return "Invalid argument";
    case ErrorCode::NotFound:
        return "Not found";
    case ErrorCode::NotSupported:
        return "Operation not supported";
    case ErrorCode::InternalError:
        return "Internal error";
    }
    return "Unknown error";
}

}  // namespace remote_sqlite_qt
