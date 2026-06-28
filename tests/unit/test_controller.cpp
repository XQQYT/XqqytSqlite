#include <cassert>
#include <iostream>
#include <memory>
#include "connection_controller.h"
#include "config_manager.h"
#include "database_controller.h"
#include "sftp_controller.h"
#include "sync_controller.h"
#include "mock_ssh_driver.h"
#include "mock_sftp_driver.h"
#include "mock_sqlite_driver.h"
#include "remote_sqlite_driver.h"
#include "event_bus_manager.h"
using namespace remote_sqlite_qt;

static int testsPassed = 0, testsFailed = 0;
void check(bool c, const char* n) { if(c){testsPassed++;std::cout<<"  PASS: "<<n<<std::endl;}else{testsFailed++;std::cerr<<"  FAIL: "<<n<<std::endl;} }

void testConnectionController() {
    auto& bm = EventBusManager::instance(); bm.init();
    auto ssh = std::make_shared<MockSshDriver>();
    ConnectionController c(ssh, bm);
    SshConfig cfg; cfg.host="test.example.com"; cfg.username="testuser";
    auto id = c.connect(cfg);
    check(!id.empty(), "connect returns ID");
    check(c.isConnected(id), "connected");
    check(c.getConnections().size()==1, "1 connection");
    c.disconnect(id);
    check(!c.isConnected(id), "disconnected");
    bm.shutdown();
}
void testConfigManager() {
    auto& cfg = ConfigManager::instance();
    cfg.setTheme("light"); check(cfg.theme()=="light", "theme");
    cfg.setLanguage("en"); check(cfg.language()=="en", "language");
}
void testSyncController() {
    auto& bm = EventBusManager::instance(); bm.init();
    auto sftp = std::make_unique<MockSftpDriver>();
    auto db = std::make_unique<MockLocalSqliteDriver>();
    SyncController sync(std::move(sftp), std::move(db), bm);
    check(!sync.isLocalMode(), "not local mode");
    bm.shutdown();
}
int main() {
    std::cout<<"=== Controller Tests ==="<<std::endl;
    testConnectionController(); testConfigManager(); testSyncController();
    std::cout<<"\nResults: "<<testsPassed<<" passed, "<<testsFailed<<" failed"<<std::endl;
    return testsFailed>0?1:0;
}
