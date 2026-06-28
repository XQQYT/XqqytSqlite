#include "event_bus_manager.h"

namespace remote_sqlite_qt {

EventBusManager& EventBusManager::instance() {
    static EventBusManager mgr;
    return mgr;
}

EventBusManager::~EventBusManager() {
    if (initialized_) {
        shutdown();
    }
}

void EventBusManager::init() {
    if (initialized_) return;

    bus_ = std::make_unique<EventBus>();

    EventBus::EventBusConfig config{
        EventBus::ThreadModel::DYNAMIC,   // 弹性线程池
        EventBus::TaskModel::NORMAL,      // FIFO 队列
        2,                                // thread_min
        8,                                // thread_max
        1024                              // task_max（队列容量）
    };
    bus_->initEventBus(config);
    initialized_ = true;
}

void EventBusManager::shutdown() {
    if (!initialized_) return;
    // EventBus 析构时自动等待线程池退出
    bus_.reset();
    initialized_ = false;
}

}  // namespace remote_sqlite_qt
