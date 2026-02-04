/**
 * @file window_manager.cpp
 * @brief 窗口管理器实现
 * @author 云流操作系统开发团队
 * @date 2026-02-04
 * @version 1.0.0
 */

#include "window_manager.h"
#include "window.h"
#include "window_event.h"
#include <algorithm>
#include <fstream>
#include <json/json.h>

namespace CloudFlow {
namespace UI {

// 窗口管理器实现类
class WindowManager::Impl {
public:
    Impl() : next_window_id_(1), focused_window_id_(-1) {}
    
    ~Impl() {
        // 清理所有窗口
        windows_.clear();
    }
    
    int createWindow(const std::string& title, int width, int height, WindowType type) {
        try {
            // 验证参数
            if (width <= 0 || height <= 0) {
                throw std::invalid_argument("窗口尺寸必须为正数");
            }
            
            if (title.empty()) {
                throw std::invalid_argument("窗口标题不能为空");
            }
            
            // 创建窗口
            int window_id = next_window_id_++;
            auto window = std::make_unique<Window>(window_id, title, width, height, type);
            
            // 设置窗口事件回调
            window->setEventCallback([this](const WindowEvent& event) {
                handleWindowEvent(event);
            });
            
            // 添加到窗口列表
            windows_[window_id] = std::move(window);
            
            // 设置焦点
            setFocus(window_id);
            
            // 发送窗口创建事件
            WindowEvent event;
            event.type = WindowEventType::Created;
            event.window_id = window_id;
            notifyEventCallback(event);
            
            return window_id;
        } catch (const std::exception& e) {
            // 记录错误日志
            // LOG_ERROR << "创建窗口失败: " << e.what();
            return -1;
        }
    }
    
    bool closeWindow(int window_id) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        // 发送窗口关闭事件
        WindowEvent event;
        event.type = WindowEventType::Closing;
        event.window_id = window_id;
        notifyEventCallback(event);
        
        // 移除窗口
        windows_.erase(it);
        
        // 如果关闭的是焦点窗口，重新设置焦点
        if (focused_window_id_ == window_id) {
            setFocusToNextWindow();
        }
        
        // 发送窗口销毁事件
        event.type = WindowEventType::Destroyed;
        notifyEventCallback(event);
        
        return true;
    }
    
    bool setFocus(int window_id) {
        if (window_id == focused_window_id_) {
            return true; // 已经是焦点窗口
        }
        
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        // 发送焦点丢失事件给当前焦点窗口
        if (focused_window_id_ != -1) {
            auto focused_it = windows_.find(focused_window_id_);
            if (focused_it != windows_.end()) {
                WindowEvent event;
                event.type = WindowEventType::FocusLost;
                event.window_id = focused_window_id_;
                notifyEventCallback(event);
            }
        }
        
        // 设置新焦点窗口
        focused_window_id_ = window_id;
        
        // 发送焦点获得事件
        WindowEvent event;
        event.type = WindowEventType::FocusGained;
        event.window_id = window_id;
        notifyEventCallback(event);
        
        return true;
    }
    
    int getFocusedWindow() const {
        return focused_window_id_;
    }
    
    bool minimizeWindow(int window_id) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        return it->second->minimize();
    }
    
    bool maximizeWindow(int window_id) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        return it->second->maximize();
    }
    
    bool restoreWindow(int window_id) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        return it->second->restore();
    }
    
    bool moveWindow(int window_id, int x, int y) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        return it->second->move(x, y);
    }
    
    bool resizeWindow(int window_id, int width, int height) {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return false;
        }
        
        return it->second->resize(width, height);
    }
    
    size_t getWindowCount() const {
        return windows_.size();
    }
    
    std::vector<int> getWindowIds() const {
        std::vector<int> ids;
        ids.reserve(windows_.size());
        
        for (const auto& pair : windows_) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
    WindowGeometry getWindowGeometry(int window_id) const {
        auto it = windows_.find(window_id);
        if (it == windows_.end()) {
            return WindowGeometry{};
        }
        
        return it->second->getGeometry();
    }
    
    void setEventCallback(std::function<void(const WindowEvent&)> callback) {
        event_callback_ = std::move(callback);
    }
    
    void handleEvent(const WindowEvent& event) {
        auto it = windows_.find(event.window_id);
        if (it != windows_.end()) {
            it->second->handleEvent(event);
        }
    }
    
    bool saveWindowState(const std::string& filename) const {
        try {
            Json::Value root;
            Json::Value windows_array(Json::arrayValue);
            
            for (const auto& pair : windows_) {
                const auto& window = pair.second;
                Json::Value window_obj;
                
                window_obj["id"] = pair.first;
                window_obj["title"] = window->getTitle();
                
                auto geometry = window->getGeometry();
                window_obj["x"] = geometry.x;
                window_obj["y"] = geometry.y;
                window_obj["width"] = geometry.width;
                window_obj["height"] = geometry.height;
                window_obj["state"] = static_cast<int>(window->getState());
                
                windows_array.append(window_obj);
            }
            
            root["windows"] = windows_array;
            root["focused_window"] = focused_window_id_;
            
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(root, &file);
            
            return true;
        } catch (const std::exception& e) {
            // LOG_ERROR << "保存窗口状态失败: " << e.what();
            return false;
        }
    }
    
    bool restoreWindowState(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            Json::Value root;
            file >> root;
            
            // 清空当前窗口
            windows_.clear();
            
            // 恢复窗口
            const auto& windows_array = root["windows"];
            for (const auto& window_obj : windows_array) {
                int id = window_obj["id"].asInt();
                std::string title = window_obj["title"].asString();
                int x = window_obj["x"].asInt();
                int y = window_obj["y"].asInt();
                int width = window_obj["width"].asInt();
                int height = window_obj["height"].asInt();
                WindowState state = static_cast<WindowState>(window_obj["state"].asInt());
                
                auto window = std::make_unique<Window>(id, title, width, height, WindowType::Normal);
                window->move(x, y);
                
                // 恢复窗口状态
                switch (state) {
                    case WindowState::Minimized:
                        window->minimize();
                        break;
                    case WindowState::Maximized:
                        window->maximize();
                        break;
                    case WindowState::Fullscreen:
                        window->setFullscreen(true);
                        break;
                    default:
                        break;
                }
                
                windows_[id] = std::move(window);
                next_window_id_ = std::max(next_window_id_, id + 1);
            }
            
            // 恢复焦点窗口
            focused_window_id_ = root["focused_window"].asInt();
            
            return true;
        } catch (const std::exception& e) {
            // LOG_ERROR << "恢复窗口状态失败: " << e.what();
            return false;
        }
    }

private:
    void handleWindowEvent(const WindowEvent& event) {
        notifyEventCallback(event);
    }
    
    void notifyEventCallback(const WindowEvent& event) {
        if (event_callback_) {
            event_callback_(event);
        }
    }
    
    void setFocusToNextWindow() {
        if (windows_.empty()) {
            focused_window_id_ = -1;
            return;
        }
        
        // 选择第一个窗口作为焦点窗口
        focused_window_id_ = windows_.begin()->first;
        
        WindowEvent event;
        event.type = WindowEventType::FocusGained;
        event.window_id = focused_window_id_;
        notifyEventCallback(event);
    }
    
    std::unordered_map<int, std::unique_ptr<Window>> windows_;
    int next_window_id_;
    int focused_window_id_;
    std::function<void(const WindowEvent&)> event_callback_;
};

// WindowManager 公共接口实现
WindowManager::WindowManager() : impl_(std::make_unique<Impl>()) {}

WindowManager::~WindowManager() = default;

int WindowManager::createWindow(const std::string& title, int width, int height, WindowType type) {
    return impl_->createWindow(title, width, height, type);
}

bool WindowManager::closeWindow(int window_id) {
    return impl_->closeWindow(window_id);
}

bool WindowManager::setFocus(int window_id) {
    return impl_->setFocus(window_id);
}

int WindowManager::getFocusedWindow() const {
    return impl_->getFocusedWindow();
}

bool WindowManager::minimizeWindow(int window_id) {
    return impl_->minimizeWindow(window_id);
}

bool WindowManager::maximizeWindow(int window_id) {
    return impl_->maximizeWindow(window_id);
}

bool WindowManager::restoreWindow(int window_id) {
    return impl_->restoreWindow(window_id);
}

bool WindowManager::moveWindow(int window_id, int x, int y) {
    return impl_->moveWindow(window_id, x, y);
}

bool WindowManager::resizeWindow(int window_id, int width, int height) {
    return impl_->resizeWindow(window_id, width, height);
}

size_t WindowManager::getWindowCount() const {
    return impl_->getWindowCount();
}

std::vector<int> WindowManager::getWindowIds() const {
    return impl_->getWindowIds();
}

WindowGeometry WindowManager::getWindowGeometry(int window_id) const {
    return impl_->getWindowGeometry(window_id);
}

void WindowManager::setEventCallback(std::function<void(const WindowEvent&)> callback) {
    impl_->setEventCallback(std::move(callback));
}

void WindowManager::handleEvent(const WindowEvent& event) {
    impl_->handleEvent(event);
}

bool WindowManager::saveWindowState(const std::string& filename) const {
    return impl_->saveWindowState(filename);
}

bool WindowManager::restoreWindowState(const std::string& filename) {
    return impl_->restoreWindowState(filename);
}

} // namespace UI
} // namespace CloudFlow