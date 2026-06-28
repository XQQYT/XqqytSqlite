#pragma once

#include <memory>
#include <string>

#include "EventBus/EventBus.hpp"

namespace remote_sqlite_qt {

// EventBus 单例封装
// 管理 EventBus 实例的生命周期、初始化和销毁
class EventBusManager {
public:
    static EventBusManager& instance();

    // 禁止拷贝/移动
    EventBusManager(const EventBusManager&) = delete;
    EventBusManager& operator=(const EventBusManager&) = delete;

    // 初始化 EventBus（应用启动时调用一次）
    void init();

    // 关闭 EventBus（应用退出时调用，等待所有任务完成）
    void shutdown();

    // 获取底层 EventBus 引用
    EventBus& bus() { return *bus_; }

    // 便捷方法：注册并订阅事件（调用 subscribeSafe）
    template <typename... Args>
    size_t subscribe(const std::string& eventName,
                                    std::function<void(Args...)> callback) {
        return bus_->subscribeSafe(eventName, callback);
    }

    // 便捷方法：发布事件
    template <typename... Args>
    void publish(const std::string& eventName, Args&&... args) {
        bus_->publish(eventName, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void publish(std::string_view eventName, Args&&... args) {
        bus_->publish(std::string(eventName), std::forward<Args>(args)...);
    }

private:
    EventBusManager() = default;
    ~EventBusManager();

    std::unique_ptr<EventBus> bus_;
    bool initialized_{false};
};

}  // namespace remote_sqlite_qt
