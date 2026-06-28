#pragma once

#include <memory>
#include <string>

namespace remote_sqlite_qt {

class ISshDriver;
class ISftpDriver;
class ILocalSqliteDriver;
class IRemoteSqliteDriver;

// 驱动工厂——根据配置创建真实驱动或 Mock 驱动
class DriverFactory {
public:
    enum class Mode {
        Mock,    // Mock 驱动（开发/测试）
        Real,    // 真实 LibSSH2 驱动（生产环境）
    };

    static DriverFactory& instance();

    DriverFactory(const DriverFactory&) = delete;
    DriverFactory& operator=(const DriverFactory&) = delete;

    void setMode(Mode mode);
    Mode mode() const { return mode_; }

    // 创建驱动实例
    std::shared_ptr<ISshDriver> createSshDriver();
    std::unique_ptr<ISftpDriver> createSftpDriver();
    std::unique_ptr<ILocalSqliteDriver> createLocalSqliteDriver();
    std::unique_ptr<IRemoteSqliteDriver> createRemoteSqliteDriver(
        std::shared_ptr<ISshDriver> ssh);

    // SSH 驱动配置（仅在 Real 模式下有效）
    void setKnownHostsPath(const std::string& path);
    std::string knownHostsPath() const { return knownHostsPath_; }

private:
    DriverFactory() = default;
    ~DriverFactory() = default;

    Mode mode_{Mode::Mock};
    std::string knownHostsPath_;
};

}  // namespace remote_sqlite_qt
