#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include "driver_factory.h"
#include "mock_ssh_driver.h"
#include "mock_sftp_driver.h"
#include "mock_sqlite_driver.h"
#include "ssh_session_guard.h"
#include "known_hosts.h"
#include "i_ssh_driver.h"
#include "types.h"
#include "result.h"
using namespace remote_sqlite_qt;

static int testsPassed=0,testsFailed=0;
void check(bool c,const char*n){if(c){testsPassed++;std::cout<<"  PASS: "<<n<<std::endl;}else{testsFailed++;std::cerr<<"  FAIL: "<<n<<std::endl;}}

void testMockSshDriver(){
    MockSshDriver d;
    SshConfig cfg; cfg.host="test.example.com"; cfg.username="testuser"; cfg.port=22;
    cfg.authMethod=AuthMethod::Password; cfg.password="secret";
    auto id=d.connect(cfg);
    check(!id.empty(),"SSH connect");
    check(d.isConnected(id),"isConnected");
    d.setExecResult("output","",0);
    auto r=d.execCommand(id,"echo test");
    check(r.exitCode==0,"execCmd exit=0");
    check(r.stdOut=="output","stdout match");
    d.disconnect(id);
    check(!d.isConnected(id),"disconnected");
}
void testMockSftpDriver(){
    MockSftpDriver d;
    std::vector<FileEntry> e={{"test.db",false,4096,"2026-01-01T00:00:00","-rw-r--r--"},{"backup",true,0,"2026-01-01T00:00:00","drwxr-xr-x"}};
    d.setFileEntries(e);
    auto l=d.listDirectory("/");
    check(l.size()==2,"list 2 entries");
    check(d.fileExists("test.db"),"exists");
    check(!d.fileExists("nope.db"),"!exists");
}
void testDriverFactory(){
    auto& f=DriverFactory::instance();
    check(f.mode()==DriverFactory::Mode::Mock,"default mock");
    auto ssh=f.createSshDriver(); check(ssh!=nullptr,"create ssh");
    auto db=f.createLocalSqliteDriver(); check(db!=nullptr,"create local db");
    f.setKnownHostsPath("/tmp/test"); check(f.knownHostsPath()=="/tmp/test","known hosts path");
    f.setMode(DriverFactory::Mode::Mock);
}
int main(){
    std::cout<<"=== Driver Tests ==="<<std::endl;
    testMockSshDriver(); testMockSftpDriver(); testDriverFactory();
    std::cout<<"\nResults: "<<testsPassed<<" passed, "<<testsFailed<<" failed"<<std::endl;
    return testsFailed>0?1:0;
}
