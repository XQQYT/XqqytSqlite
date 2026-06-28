#include "driver_factory.h"

#include "driver/mock/mock_ssh_driver.h"
#include "driver/mock/mock_sftp_driver.h"
#include "driver/mock/mock_sqlite_driver.h"
#include "driver/sqlite/remote_sqlite_driver.h"
#include "driver/sqlite/local_sqlite_driver.h"
#include "driver/ssh/libssh2_ssh_driver.h"
#include "driver/sftp/libssh2_sftp_driver.h"

namespace remote_sqlite_qt {

DriverFactory& DriverFactory::instance() {
    static DriverFactory factory;
    return factory;
}

void DriverFactory::setMode(Mode mode) {
    mode_ = mode;
}

std::shared_ptr<ISshDriver> DriverFactory::createSshDriver() {
    if (mode_ == Mode::Real) {
        auto driver = std::make_shared<LibSsh2SshDriver>();
        if (!knownHostsPath_.empty()) {
            driver->setKnownHostsPath(knownHostsPath_);
        }
        return driver;
    }
    return std::make_shared<MockSshDriver>();
}

std::unique_ptr<ISftpDriver> DriverFactory::createSftpDriver() {
    if (mode_ == Mode::Real) {
        // SFTP 驱动由 SSH 驱动的 createSftpDriver() 创建
        // 这里返回 nullptr——调用方应通过 ISshDriver::createSftpDriver() 获取
        return nullptr;
    }
    return std::make_unique<MockSftpDriver>();
}

std::unique_ptr<ILocalSqliteDriver> DriverFactory::createLocalSqliteDriver() {
    if (mode_ == Mode::Real) {
        return std::make_unique<LocalSqliteDriver>();
    }
    return std::make_unique<MockLocalSqliteDriver>();
}

std::unique_ptr<IRemoteSqliteDriver> DriverFactory::createRemoteSqliteDriver(
    std::shared_ptr<ISshDriver> ssh) {
    return std::make_unique<RemoteSqliteDriver>(std::move(ssh));
}

void DriverFactory::setKnownHostsPath(const std::string& path) {
    knownHostsPath_ = path;
}

}  // namespace remote_sqlite_qt
