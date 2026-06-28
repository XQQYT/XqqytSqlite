#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>
#include "event_bus_manager.h"
#include "event_names.h"
using namespace remote_sqlite_qt;

static int testsPassed=0,testsFailed=0;
void check(bool c,const char*n){if(c){testsPassed++;std::cout<<"  PASS: "<<n<<std::endl;}else{testsFailed++;std::cerr<<"  FAIL: "<<n<<std::endl;}}

void testInitShutdown(){auto& b=EventBusManager::instance();b.init();check(true,"init ok");b.shutdown();check(true,"shutdown ok");}
void testEventNames(){check(!events::SshConnected.empty(),"SshConnected");check(events::QueryResult=="query:result","QueryResult");check(events::SyncConflict=="sync:conflict","SyncConflict");check(events::ThemeChanged=="theme:changed","ThemeChanged");}

int main(){
    std::cout<<"=== EventBus Tests ==="<<std::endl;
    testInitShutdown();testEventNames();
    std::cout<<"\nResults: "<<testsPassed<<" passed, "<<testsFailed<<" failed"<<std::endl;
    return testsFailed>0?1:0;
}
