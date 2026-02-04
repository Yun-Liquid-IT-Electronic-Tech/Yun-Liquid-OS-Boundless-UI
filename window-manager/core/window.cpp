/**
 * @file window.cpp
 * @brief 窗口类实现
 * @author 云流操作系统开发团队
 * @date 2026-02-04
 * @version 1.0.0
 */

#include "window.h"
#include <algorithm>
#include <stdexcept>

namespace CloudFlow {
namespace UI {

// 窗口实现类
class WindowImpl {
public:
    WindowImpl(int id, const std::string& title, int width, int height, WindowType type)
        : id_(id)
        , title_(title)
        , geometry_{0, 0, width, height, 100, 100, 4096, 4096}
        , state_(WindowState::Normal)
        , type_(type)
        , visible_(true)
        , has_focus_(false)
        , resizable_(true)
        , movable_(true)
        , always_on_top_(false)
        , opacity_(1.0f) {
        
        // 验证参数
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("窗口尺寸必须为正数");
        }
        
        if (title.empty()) {
            throw std::invalid_argument("窗口标题不能为空");
        }
        
        // 根据窗口类型设置默认属性
        switch (type) {
            case WindowType::Dialog:
                always_on_top_ = true;
                resizable_ = false;
                break;
            case WindowType::Tooltip:
                always_on_top_ = true;
                movable_ = false;
                resizable_ = false;
                break;
            case WindowType::Popup:
                always_on_top_ = true;
                break;
            case WindowType::Utility:
                resizable_ = false;
                break;
            default:
                break;
        }
    }
    
    ~WindowImpl() {
        // 清理资源
    }
    
    int getId() const { return id_; }
    
    std::string getTitle() const { return title_; }
    
    bool setTitle(const std::string& title) {
        if (title.empty()) {
            return false;
        }
        
        title_ = title;
        
        // 发送标题改变事件
        WindowEvent event;
        event.type = WindowEventType::StateChanged;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        notifyEventCallback(event);
        
        return true;
    }
    
    WindowGeometry getGeometry() const { return geometry_; }
    
    WindowState getState() const { return state_; }
    
    WindowType getType() const { return type_; }
    
    bool move(int x, int y) {
        int old_x = geometry_.x;
        int old_y = geometry_.y;
        
        geometry_.x = x;
        geometry_.y = y;
        
        // 发送窗口移动事件
        WindowEvent event = EventUtils::createWindowMovedEvent(id_, x, y, old_x, old_y);
        notifyEventCallback(event);
        
        return true;
    }
    
    bool resize(int width, int height) {
        // 检查尺寸限制
        if (width < geometry_.min_width || height < geometry_.min_height) {
            return false;
        }
        
        if (width > geometry_.max_width || height > geometry_.max_height) {
            return false;
        }
        
        int old_width = geometry_.width;
        int old_height = geometry_.height;
        
        geometry_.width = width;
        geometry_.height = height;
        
        // 发送窗口大小改变事件
        WindowEvent event = EventUtils::createWindowResizedEvent(id_, width, height, old_width, old_height);
        notifyEventCallback(event);
        
        return true;
    }
    
    bool minimize() {
        if (state_ == WindowState::Minimized) {
            return true; // 已经是最小化状态
        }
        
        WindowState old_state = state_;
        state_ = WindowState::Minimized;
        visible_ = false;
        
        // 发送状态改变事件
        sendStateChangeEvent(old_state);
        
        return true;
    }
    
    bool maximize() {
        if (state_ == WindowState::Maximized) {
            return true; // 已经是最大化状态
        }
        
        WindowState old_state = state_;
        state_ = WindowState::Maximized;
        
        // 保存原始大小
        if (old_state == WindowState::Normal) {
            normal_geometry_ = geometry_;
        }
        
        // 设置最大化尺寸（这里简化实现，实际应获取屏幕尺寸）
        geometry_.width = 1920;
        geometry_.height = 1080;
        geometry_.x = 0;
        geometry_.y = 0;
        
        // 发送状态改变事件
        sendStateChangeEvent(old_state);
        
        return true;
    }
    
    bool restore() {
        if (state_ == WindowState::Normal) {
            return true; // 已经是正常状态
        }
        
        WindowState old_state = state_;
        state_ = WindowState::Normal;
        visible_ = true;
        
        // 恢复原始大小
        if (old_state == WindowState::Maximized || old_state == WindowState::Fullscreen) {
            geometry_ = normal_geometry_;
        }
        
        // 发送状态改变事件
        sendStateChangeEvent(old_state);
        
        return true;
    }
    
    bool setFullscreen(bool fullscreen) {
        if (fullscreen && state_ == WindowState::Fullscreen) {
            return true; // 已经是全屏状态
        }
        
        if (!fullscreen && state_ != WindowState::Fullscreen) {
            return true; // 已经是非全屏状态
        }
        
        WindowState old_state = state_;
        
        if (fullscreen) {
            state_ = WindowState::Fullscreen;
            // 保存原始大小
            normal_geometry_ = geometry_;
            // 设置全屏尺寸
            geometry_.width = 1920;
            geometry_.height = 1080;
            geometry_.x = 0;
            geometry_.y = 0;
        } else {
            state_ = WindowState::Normal;
            // 恢复原始大小
            geometry_ = normal_geometry_;
        }
        
        // 发送状态改变事件
        sendStateChangeEvent(old_state);
        
        return true;
    }
    
    bool show() {
        if (visible_) {
            return true; // 已经可见
        }
        
        visible_ = true;
        
        // 发送显示事件
        WindowEvent event;
        event.type = WindowEventType::StateChanged;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        notifyEventCallback(event);
        
        return true;
    }
    
    bool hide() {
        if (!visible_) {
            return true; // 已经隐藏
        }
        
        visible_ = false;
        
        // 发送隐藏事件
        WindowEvent event;
        event.type = WindowEventType::StateChanged;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        notifyEventCallback(event);
        
        return true;
    }
    
    bool close() {
        // 发送关闭请求事件
        WindowEvent event;
        event.type = WindowEventType::CloseRequest;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        notifyEventCallback(event);
        
        return true;
    }
    
    void setEventCallback(std::function<void(const WindowEvent&)> callback) {
        event_callback_ = std::move(callback);
    }
    
    void handleEvent(const WindowEvent& event) {
        // 处理窗口事件
        switch (event.type) {
            case WindowEventType::FocusGained:
                has_focus_ = true;
                break;
            case WindowEventType::FocusLost:
                has_focus_ = false;
                break;
            default:
                break;
        }
        
        // 转发事件给回调
        notifyEventCallback(event);
    }
    
    bool setResizable(bool resizable) {
        resizable_ = resizable;
        return true;
    }
    
    bool setMovable(bool movable) {
        movable_ = movable;
        return true;
    }
    
    bool setAlwaysOnTop(bool always_on_top) {
        always_on_top_ = always_on_top;
        return true;
    }
    
    bool setMinimumSize(int min_width, int min_height) {
        if (min_width <= 0 || min_height <= 0) {
            return false;
        }
        
        geometry_.min_width = min_width;
        geometry_.min_height = min_height;
        
        // 如果当前尺寸小于最小尺寸，调整到最小尺寸
        if (geometry_.width < min_width) {
            geometry_.width = min_width;
        }
        if (geometry_.height < min_height) {
            geometry_.height = min_height;
        }
        
        return true;
    }
    
    bool setMaximumSize(int max_width, int max_height) {
        if (max_width <= 0 || max_height <= 0) {
            return false;
        }
        
        geometry_.max_width = max_width;
        geometry_.max_height = max_height;
        
        // 如果当前尺寸大于最大尺寸，调整到最大尺寸
        if (geometry_.width > max_width) {
            geometry_.width = max_width;
        }
        if (geometry_.height > max_height) {
            geometry_.height = max_height;
        }
        
        return true;
    }
    
    bool setOpacity(float opacity) {
        if (opacity < 0.0f || opacity > 1.0f) {
            return false;
        }
        
        opacity_ = opacity;
        return true;
    }
    
    bool isVisible() const { return visible_; }
    
    bool hasFocus() const { return has_focus_; }
    
    bool setFocus() {
        if (has_focus_) {
            return true; // 已经有焦点
        }
        
        // 发送焦点请求事件
        WindowEvent event;
        event.type = WindowEventType::FocusGained;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        notifyEventCallback(event);
        
        return true;
    }
    
    bool update() {
        // 实现窗口内容更新
        return true;
    }
    
    bool repaint() {
        // 实现窗口重绘
        return true;
    }

private:
    void notifyEventCallback(const WindowEvent& event) {
        if (event_callback_) {
            event_callback_(event);
        }
    }
    
    void sendStateChangeEvent(WindowState old_state) {
        WindowEvent event;
        event.type = WindowEventType::StateChanged;
        event.window_id = id_;
        event.timestamp = EventUtils::getCurrentTimestamp();
        event.data.int_value = static_cast<int>(old_state);
        notifyEventCallback(event);
    }
    
    int id_;
    std::string title_;
    WindowGeometry geometry_;
    WindowGeometry normal_geometry_; // 用于保存正常状态下的几何信息
    WindowState state_;
    WindowType type_;
    bool visible_;
    bool has_focus_;
    bool resizable_;
    bool movable_;
    bool always_on_top_;
    float opacity_;
    std::function<void(const WindowEvent&)> event_callback_;
};

// Window 公共接口实现
Window::Window(int id, const std::string& title, int width, int height, WindowType type)
    : impl_(std::make_unique<WindowImpl>(id, title, width, height, type)) {}

Window::~Window() = default;

int Window::getId() const { return impl_->getId(); }

std::string Window::getTitle() const { return impl_->getTitle(); }

bool Window::setTitle(const std::string& title) { return impl_->setTitle(title); }

WindowGeometry Window::getGeometry() const { return impl_->getGeometry(); }

WindowState Window::getState() const { return impl_->getState(); }

WindowType Window::getType() const { return impl_->getType(); }

bool Window::move(int x, int y) { return impl_->move(x, y); }

bool Window::resize(int width, int height) { return impl_->resize(width, height); }

bool Window::minimize() { return impl_->minimize(); }

bool Window::maximize() { return impl_->maximize(); }

bool Window::restore() { return impl_->restore(); }

bool Window::setFullscreen(bool fullscreen) { return impl_->setFullscreen(fullscreen); }

bool Window::show() { return impl_->show(); }

bool Window::hide() { return impl_->hide(); }

bool Window::close() { return impl_->close(); }

void Window::setEventCallback(std::function<void(const WindowEvent&)> callback) {
    impl_->setEventCallback(std::move(callback));
}

void Window::handleEvent(const WindowEvent& event) {
    impl_->handleEvent(event);
}

bool Window::setResizable(bool resizable) { return impl_->setResizable(resizable); }

bool Window::setMovable(bool movable) { return impl_->setMovable(movable); }

bool Window::setAlwaysOnTop(bool always_on_top) { return impl_->setAlwaysOnTop(always_on_top); }

bool Window::setMinimumSize(int min_width, int min_height) {
    return impl_->setMinimumSize(min_width, min_height);
}

bool Window::setMaximumSize(int max_width, int max_height) {
    return impl_->setMaximumSize(max_width, max_height);
}

bool Window::setOpacity(float opacity) { return impl_->setOpacity(opacity); }

bool Window::isVisible() const { return impl_->isVisible(); }

bool Window::hasFocus() const { return impl_->hasFocus(); }

bool Window::setFocus() { return impl_->setFocus(); }

bool Window::update() { return impl_->update(); }

bool Window::repaint() { return impl_->repaint(); }

} // namespace UI
} // namespace CloudFlow