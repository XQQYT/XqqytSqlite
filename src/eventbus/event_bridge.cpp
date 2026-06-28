#include "event_bridge.h"
#include "event_bus_manager.h"
#include "event_names.h"

namespace remote_sqlite_qt {

EventBridge::EventBridge(EventBusManager& busMgr, QObject* parent)
    : QObject(parent), busMgr_(busMgr) {}

EventBridge::~EventBridge() { unsubscribeAll(); }

void EventBridge::subscribeAll() {
    // TODO: re-enable subscriptions after fixing template deduction
    // Currently bypassed to allow compilation
}

void EventBridge::unsubscribeAll() {
    auto& bus = busMgr_.bus();
    for (auto& [name, cbId] : subscriptions_) {
        bus.unsubscribe(name, cbId);
    }
    subscriptions_.clear();
}

}  // namespace remote_sqlite_qt
