#pragma once

#include <string>
#include <variant>
#include "error_code.h"

namespace remote_sqlite_qt {

// Result<T, ErrorCode> — 类似 Rust Result 的错误处理类型
// 用法:
//   Result<int> ok = 42;
//   Result<int> err = ErrorCode::NotFound;
//   if (ok) { int val = *ok; }
//   if (err.isError()) { ErrorCode c = err.error(); }

template <typename T>
class Result {
public:
    // 从值构造（成功）
    Result(const T& value) : data_(value) {}
    Result(T&& value) : data_(std::move(value)) {}

    // 从 ErrorCode 构造（失败）
    Result(ErrorCode error) : data_(error) {}

    // 判断是否成功
    explicit operator bool() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    bool isError() const noexcept {
        return std::holds_alternative<ErrorCode>(data_);
    }

    // 获取值（调用前需确认 operator bool() 为 true）
    T& operator*() { return std::get<T>(data_); }
    const T& operator*() const { return std::get<T>(data_); }
    T* operator->() { return &std::get<T>(data_); }
    const T* operator->() const { return &std::get<T>(data_); }

    // 获取错误码
    ErrorCode error() const { return std::get<ErrorCode>(data_); }

    // 获取值或默认值
    T valueOr(const T& defaultValue) const {
        return std::holds_alternative<T>(data_) ? std::get<T>(data_) : defaultValue;
    }

private:
    std::variant<T, ErrorCode> data_;
};

// Result<void> 特化（无返回值，仅表示成功/失败）
template <>
class Result<void> {
public:
    Result() = default;  // 成功
    Result(ErrorCode error) : error_(error) {}

    explicit operator bool() const noexcept { return !error_.has_value(); }
    bool isError() const noexcept { return error_.has_value(); }
    ErrorCode error() const { return *error_; }

private:
    std::optional<ErrorCode> error_;
};

}  // namespace remote_sqlite_qt
