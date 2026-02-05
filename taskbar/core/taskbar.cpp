/**
 * @file taskbar.cpp
 * @brief 任务栏管理器实现文件
 */

#include "taskbar.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <json/json.h>

namespace CloudFlow {
namespace Desktop {

class TaskbarManager::Impl {
public:
    Impl() : is_visible_(false), 
              is_start_menu_active_(false),
              last_error_(""),
              total_launches_(0),
              total_clicks_(0) {}
    
    ~Impl() = default;
    
    bool initialize(std::shared_ptr<ITaskbarRenderer> renderer) {
        if (!renderer) {
            last_error_ = "渲染器不能为空";
            return false;
        }
        
        renderer_ = renderer;
        
        // 创建默认快速启动项
        createDefaultQuickLaunchItems();
        
        // 创建默认系统托盘项
        createDefaultSystemTrayItems();
        
        // 启动时钟更新线程
        startClockThread();
        
        return true;
    }
    
    void show() {
        is_visible_ = true;
        refresh();
    }
    
    void hide() {
        is_visible_ = false;
    }
    
    void refresh() {
        if (!is_visible_ || !renderer_) return;
        
        // 渲染任务栏背景
        renderer_->renderBackground(appearance_);
        
        // 渲染开始菜单按钮
        renderer_->renderStartMenuButton(appearance_, is_start_menu_active_);
        
        // 渲染快速启动项
        for (const auto& item : quick_launch_items_) {
            if (item.visible) {
                renderer_->renderQuickLaunchItem(item, appearance_);
            }
        }
        
        // 渲染窗口列表
        for (const auto& window : window_list_) {
            bool is_active = (active_window_id_ == window.first);
            bool is_minimized = (minimized_windows_.find(window.first) != minimized_windows_.end());
            renderer_->renderWindowListItem(window.first, window.second, is_active, is_minimized, appearance_);
        }
        
        // 渲染系统托盘项
        if (appearance_.show_system_tray) {
            for (const auto& item : system_tray_items_) {
                if (item.visible) {
                    renderer_->renderSystemTrayItem(item, appearance_);
                }
            }
        }
        
        // 渲染时钟
        if (appearance_.show_clock) {
            renderer_->renderClock(current_time_, clock_format_, appearance_);
        }
    }
    
    void setAppearance(const TaskbarAppearance& appearance) {
        appearance_ = appearance;
        refresh();
        
        // 触发外观改变事件
        TaskbarEvent event(TaskbarEvent::Type::TaskbarResized);
        notifyEventListeners(event);
    }
    
    TaskbarAppearance getAppearance() const {
        return appearance_;
    }
    
    bool addQuickLaunchItem(const QuickLaunchItem& item) {
        // 检查项目ID是否已存在
        if (std::find_if(quick_launch_items_.begin(), quick_launch_items_.end(),
                        [&item](const QuickLaunchItem& existing) {
                            return existing.id == item.id;
                        }) != quick_launch_items_.end()) {
            last_error_ = "快速启动项ID已存在: " + item.id;
            return false;
        }
        
        quick_launch_items_.push_back(item);
        refresh();
        return true;
    }
    
    bool removeQuickLaunchItem(const std::string& item_id) {
        auto it = std::find_if(quick_launch_items_.begin(), quick_launch_items_.end(),
                              [&item_id](const QuickLaunchItem& item) {
                                  return item.id == item_id;
                              });
        
        if (it == quick_launch_items_.end()) {
            last_error_ = "快速启动项不存在: " + item_id;
            return false;
        }
        
        quick_launch_items_.erase(it);
        refresh();
        return true;
    }
    
    std::vector<QuickLaunchItem> getQuickLaunchItems() const {
        return quick_launch_items_;
    }
    
    bool addSystemTrayItem(const SystemTrayItem& item) {
        // 检查项目ID是否已存在
        if (std::find_if(system_tray_items_.begin(), system_tray_items_.end(),
                        [&item](const SystemTrayItem& existing) {
                            return existing.id == item.id;
                        }) != system_tray_items_.end()) {
            last_error_ = "系统托盘项ID已存在: " + item.id;
            return false;
        }
        
        system_tray_items_.push_back(item);
        refresh();
        return true;
    }
    
    bool removeSystemTrayItem(const std::string& item_id) {
        auto it = std::find_if(system_tray_items_.begin(), system_tray_items_.end(),
                              [&item_id](const SystemTrayItem& item) {
                                  return item.id == item_id;
                              });
        
        if (it == system_tray_items_.end()) {
            last_error_ = "系统托盘项不存在: " + item_id;
            return false;
        }
        
        system_tray_items_.erase(it);
        refresh();
        return true;
    }
    
    std::vector<SystemTrayItem> getSystemTrayItems() const {
        return system_tray_items_;
    }
    
    void setClockFormat(const ClockFormat& format) {
        clock_format_ = format;
        refresh();
    }
    
    ClockFormat getClockFormat() const {
        return clock_format_;
    }
    
    bool addWindowToList(const std::string& window_id, const std::string& window_title) {
        if (window_list_.find(window_id) != window_list_.end()) {
            last_error_ = "窗口已存在: " + window_id;
            return false;
        }
        
        window_list_[window_id] = window_title;
        refresh();
        return true;
    }
    
    bool removeWindowFromList(const std::string& window_id) {
        auto it = window_list_.find(window_id);
        if (it == window_list_.end()) {
            last_error_ = "窗口不存在: " + window_id;
            return false;
        }
        
        window_list_.erase(it);
        minimized_windows_.erase(window_id);
        
        if (active_window_id_ == window_id) {
            active_window_id_.clear();
        }
        
        refresh();
        return true;
    }
    
    void setWindowActive(const std::string& window_id, bool is_active) {
        if (is_active) {
            active_window_id_ = window_id;
        } else if (active_window_id_ == window_id) {
            active_window_id_.clear();
        }
        refresh();
    }
    
    void setWindowMinimized(const std::string& window_id, bool is_minimized) {
        if (is_minimized) {
            minimized_windows_.insert(window_id);
        } else {
            minimized_windows_.erase(window_id);
        }
        refresh();
    }
    
    std::map<std::string, std::string> getWindowList() const {
        return window_list_;
    }
    
    void handleMouseClick(int x, int y, int button) {
        total_clicks_++;
        
        // 检测点击位置并处理相应事件
        if (isStartMenuButtonClicked(x, y)) {
            handleStartMenuClick(button);
        } else if (isQuickLaunchItemClicked(x, y)) {
            handleQuickLaunchItemClick(x, y, button);
        } else if (isWindowListItemClicked(x, y)) {
            handleWindowListItemClick(x, y, button);
        } else if (isSystemTrayItemClicked(x, y)) {
            handleSystemTrayItemClick(x, y, button);
        } else if (isClockClicked(x, y)) {
            handleClockClick(button);
        }
    }
    
    void handleMouseMove(int x, int y) {
        // 处理鼠标悬停效果
        if (appearance_.auto_hide) {
            // 自动隐藏逻辑：鼠标靠近任务栏区域时显示
            auto taskbar_size = renderer_->getTaskbarSize(appearance_);
            int screen_width = 1920; // 假设屏幕宽度
            int screen_height = 1080; // 假设屏幕高度
            
            bool should_show = false;
            switch (appearance_.position) {
                case TaskbarPosition::Bottom:
                    should_show = (y >= screen_height - 10);
                    break;
                case TaskbarPosition::Top:
                    should_show = (y <= 10);
                    break;
                case TaskbarPosition::Left:
                    should_show = (x <= 10);
                    break;
                case TaskbarPosition::Right:
                    should_show = (x >= screen_width - 10);
                    break;
            }
            
            if (should_show != is_visible_) {
                is_visible_ = should_show;
                refresh();
            }
        }
    }
    
    void handleKeyboardEvent(int key_code, bool ctrl_pressed, bool shift_pressed) {
        // 处理键盘快捷键
        switch (key_code) {
            case 91: // Windows键 - 打开开始菜单
                toggleStartMenu();
                break;
            case 77: // M键 + Windows键 - 最小化所有窗口
                if (ctrl_pressed) {
                    minimizeAllWindows();
                }
                break;
            case 68: // D键 + Windows键 - 显示桌面
                if (ctrl_pressed) {
                    showDesktop();
                }
                break;
        }
    }
    
    void addEventListener(std::function<void(const TaskbarEvent&)> callback) {
        event_listeners_.push_back(callback);
    }
    
    bool saveConfig(const std::string& config_path) {
        try {
            Json::Value root;
            
            // 保存外观设置
            Json::Value appearance_obj;
            appearance_obj["position"] = static_cast<int>(appearance_.position);
            appearance_obj["style"] = static_cast<int>(appearance_.style);
            appearance_obj["height"] = appearance_.height;
            appearance_obj["auto_hide"] = appearance_.auto_hide;
            appearance_obj["always_on_top"] = appearance_.always_on_top;
            appearance_obj["show_clock"] = appearance_.show_clock;
            appearance_obj["show_system_tray"] = appearance_.show_system_tray;
            root["appearance"] = appearance_obj;
            
            // 保存快速启动项
            Json::Value quick_launch_array(Json::arrayValue);
            for (const auto& item : quick_launch_items_) {
                Json::Value item_obj;
                item_obj["id"] = item.id;
                item_obj["name"] = item.name;
                item_obj["icon_path"] = item.icon_path;
                item_obj["executable_path"] = item.executable_path;
                item_obj["launch_count"] = item.launch_count;
                quick_launch_array.append(item_obj);
            }
            root["quick_launch_items"] = quick_launch_array;
            
            // 保存时钟格式
            Json::Value clock_format_obj;
            clock_format_obj["show_date"] = clock_format_.show_date;
            clock_format_obj["show_seconds"] = clock_format_.show_seconds;
            clock_format_obj["time_format"] = clock_format_.time_format;
            clock_format_obj["date_format"] = clock_format_.date_format;
            root["clock_format"] = clock_format_obj;
            
            std::ofstream file(config_path);
            if (!file.is_open()) {
                last_error_ = "无法打开配置文件: " + config_path;
                return false;
            }
            
            file << root;
            file.close();
            
            return true;
        } catch (const std::exception& e) {
            last_error_ = std::string("保存配置失败: ") + e.what();
            return false;
        }
    }
    
    bool loadConfig(const std::string& config_path) {
        try {
            std::ifstream file(config_path);
            if (!file.is_open()) {
                last_error_ = "无法打开配置文件: " + config_path;
                return false;
            }
            
            Json::Value root;
            file >> root;
            file.close();
            
            // 加载外观设置
            const Json::Value& appearance_obj = root["appearance"];
            appearance_.position = static_cast<TaskbarPosition>(appearance_obj["position"].asInt());
            appearance_.style = static_cast<TaskbarStyle>(appearance_obj["style"].asInt());
            appearance_.height = appearance_obj["height"].asInt();
            appearance_.auto_hide = appearance_obj["auto_hide"].asBool();
            appearance_.always_on_top = appearance_obj["always_on_top"].asBool();
            appearance_.show_clock = appearance_obj["show_clock"].asBool();
            appearance_.show_system_tray = appearance_obj["show_system_tray"].asBool();
            
            // 加载快速启动项
            quick_launch_items_.clear();
            const Json::Value& quick_launch_array = root["quick_launch_items"];
            for (const auto& item_obj : quick_launch_array) {
                QuickLaunchItem item;
                item.id = item_obj["id"].asString();
                item.name = item_obj["name"].asString();
                item.icon_path = item_obj["icon_path"].asString();
                item.executable_path = item_obj["executable_path"].asString();
                item.launch_count = item_obj["launch_count"].asInt();
                quick_launch_items_.push_back(item);
            }
            
            // 加载时钟格式
            const Json::Value& clock_format_obj = root["clock_format"];
            clock_format_.show_date = clock_format_obj["show_date"].asBool();
            clock_format_.show_seconds = clock_format_obj["show_seconds"].asBool();
            clock_format_.time_format = clock_format_obj["time_format"].asString();
            clock_format_.date_format = clock_format_obj["date_format"].asString();
            
            refresh();
            return true;
        } catch (const std::exception& e) {
            last_error_ = std::string("加载配置失败: ") + e.what();
            return false;
        }
    }
    
    std::string getLastError() const {
        return last_error_;
    }
    
    bool isVisible() const {
        return is_visible_;
    }
    
    void toggleAutoHide() {
        appearance_.auto_hide = !appearance_.auto_hide;
        
        TaskbarEvent event(TaskbarEvent::Type::AutoHideToggled);
        notifyEventListeners(event);
        
        refresh();
    }
    
    std::string getStatistics() const {
        std::stringstream stats;
        stats << "=== 任务栏统计信息 ===\n";
        stats << "总启动次数: " << total_launches_ << "\n";
        stats << "总点击次数: " << total_clicks_ << "\n";
        stats << "快速启动项数量: " << quick_launch_items_.size() << "\n";
        stats << "系统托盘项数量: " << system_tray_items_.size() << "\n";
        stats << "窗口列表数量: " << window_list_.size() << "\n";
        stats << "最小化窗口数量: " << minimized_windows_.size() << "\n";
        
        return stats.str();
    }

private:
    void createDefaultQuickLaunchItems() {
        // 文件管理器
        QuickLaunchItem file_manager;
        file_manager.id = "file_manager";
        file_manager.name = "文件管理器";
        file_manager.icon_path = "/usr/share/icons/file-manager.png";
        file_manager.executable_path = "/usr/bin/cloudflow-file-manager";
        quick_launch_items_.push_back(file_manager);
        
        // 浏览器
        QuickLaunchItem browser;
        browser.id = "browser";
        browser.name = "浏览器";
        browser.icon_path = "/usr/share/icons/browser.png";
        browser.executable_path = "/usr/bin/cloudflow-browser";
        quick_launch_items_.push_back(browser);
        
        // 终端
        QuickLaunchItem terminal;
        terminal.id = "terminal";
        terminal.name = "终端";
        terminal.icon_path = "/usr/share/icons/terminal.png";
        terminal.executable_path = "/usr/bin/cloudflow-terminal";
        quick_launch_items_.push_back(terminal);
    }
    
    void createDefaultSystemTrayItems() {
        // 网络状态
        SystemTrayItem network;
        network.id = "network";
        network.name = "网络";
        network.icon_path = "/usr/share/icons/network.png";
        network.tooltip = "网络连接状态";
        system_tray_items_.push_back(network);
        
        // 音量控制
        SystemTrayItem volume;
        volume.id = "volume";
        volume.name = "音量";
        volume.icon_path = "/usr/share/icons/volume.png";
        volume.tooltip = "音量控制";
        system_tray_items_.push_back(volume);
        
        // 电池状态
        SystemTrayItem battery;
        battery.id = "battery";
        battery.name = "电池";
        battery.icon_path = "/usr/share/icons/battery.png";
        battery.tooltip = "电池状态";
        system_tray_items_.push_back(battery);
    }
    
    void startClockThread() {
        clock_thread_ = std::thread([this]() {
            while (true) {
                current_time_ = std::chrono::system_clock::now();
                
                if (is_visible_ && appearance_.show_clock) {
                    refresh();
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
        clock_thread_.detach();
    }
    
    bool isStartMenuButtonClicked(int x, int y) const {
        // 简化实现：假设开始菜单按钮在任务栏最左侧
        auto taskbar_size = renderer_->getTaskbarSize(appearance_);
        return (x >= 0 && x <= 40 && y >= 0 && y <= taskbar_size.second);
    }
    
    bool isQuickLaunchItemClicked(int x, int y) const {
        // 简化实现：快速启动栏在开始菜单按钮右侧
        auto taskbar_size = renderer_->getTaskbarSize(appearance_);
        return (x >= 50 && x <= 200 && y >= 0 && y <= taskbar_size.second);
    }
    
    bool isWindowListItemClicked(int x, int y) const {
        // 简化实现：窗口列表在快速启动栏右侧
        auto taskbar_size = renderer_->getTaskbarSize(appearance_);
        return (x >= 210 && x <= taskbar_size.first - 200 && y >= 0 && y <= taskbar_size.second);
    }
    
    bool isSystemTrayItemClicked(int x, int y) const {
        // 简化实现：系统托盘在时钟左侧
        auto taskbar_size = renderer_->getTaskbarSize(appearance_);
        return (x >= taskbar_size.first - 150 && x <= taskbar_size.first - 50 && y >= 0 && y <= taskbar_size.second);
    }
    
    bool isClockClicked(int x, int y) const {
        // 简化实现：时钟在任务栏最右侧
        auto taskbar_size = renderer_->getTaskbarSize(appearance_);
        return (x >= taskbar_size.first - 40 && x <= taskbar_size.first && y >= 0 && y <= taskbar_size.second);
    }
    
    void handleStartMenuClick(int button) {
        if (button == 1) { // 左键
            toggleStartMenu();
        } else if (button == 2) { // 右键
            // 显示开始菜单上下文菜单
            TaskbarEvent event(TaskbarEvent::Type::StartMenuClicked);
            notifyEventListeners(event);
        }
    }
    
    void handleQuickLaunchItemClick(int x, int y, int button) {
        if (button == 1) { // 左键
            // 查找点击的快速启动项
            // 简化实现：假设每个快速启动项宽度为40像素
            int item_index = (x - 50) / 40;
            if (item_index >= 0 && item_index < static_cast<int>(quick_launch_items_.size())) {
                const auto& item = quick_launch_items_[item_index];
                
                // 增加启动计数
                quick_launch_items_[item_index].launch_count++;
                total_launches_++;
                
                TaskbarEvent event(TaskbarEvent::Type::QuickLaunchItemClicked);
                event.item_id = item.id;
                notifyEventListeners(event);
            }
        }
    }
    
    void handleWindowListItemClick(int x, int y, int button) {
        if (button == 1) { // 左键
            // 查找点击的窗口列表项
            // 简化实现：假设每个窗口列表项宽度为200像素
            int item_index = (x - 210) / 200;
            if (item_index >= 0 && item_index < static_cast<int>(window_list_.size())) {
                auto it = window_list_.begin();
                std::advance(it, item_index);
                
                TaskbarEvent event(TaskbarEvent::Type::WindowRestored);
                event.item_id = it->first;
                notifyEventListeners(event);
            }
        } else if (button == 2) { // 右键
            // 显示窗口上下文菜单
        }
    }
    
    void handleSystemTrayItemClick(int x, int y, int button) {
        if (button == 1) { // 左键
            // 查找点击的系统托盘项
            // 简化实现：假设每个系统托盘项宽度为30像素
            int item_index = (x - (renderer_->getTaskbarSize(appearance_).first - 150)) / 30;
            if (item_index >= 0 && item_index < static_cast<int>(system_tray_items_.size())) {
                const auto& item = system_tray_items_[item_index];
                
                TaskbarEvent event(TaskbarEvent::Type::SystemTrayItemClicked);
                event.item_id = item.id;
                notifyEventListeners(event);
            }
        }
    }
    
    void handleClockClick(int button) {
        if (button == 1) { // 左键
            TaskbarEvent event(TaskbarEvent::Type::ClockClicked);
            notifyEventListeners(event);
        }
    }
    
    void toggleStartMenu() {
        is_start_menu_active_ = !is_start_menu_active_;
        refresh();
        
        TaskbarEvent event(TaskbarEvent::Type::StartMenuClicked);
        notifyEventListeners(event);
    }
    
    void minimizeAllWindows() {
        for (const auto& window : window_list_) {
            minimized_windows_.insert(window.first);
        }
        refresh();
    }
    
    void showDesktop() {
        // 最小化所有窗口
        minimizeAllWindows();
        
        // 取消开始菜单激活状态
        is_start_menu_active_ = false;
        refresh();
    }
    
    void notifyEventListeners(const TaskbarEvent& event) {
        for (const auto& listener : event_listeners_) {
            listener(event);
        }
    }

private:
    std::shared_ptr<ITaskbarRenderer> renderer_;
    std::vector<std::function<void(const TaskbarEvent&)>> event_listeners_;
    
    TaskbarAppearance appearance_;
    ClockFormat clock_format_;
    
    std::vector<QuickLaunchItem> quick_launch_items_;
    std::vector<SystemTrayItem> system_tray_items_;
    std::map<std::string, std::string> window_list_;
    std::set<std::string> minimized_windows_;
    std::string active_window_id_;
    
    bool is_visible_;
    bool is_start_menu_active_;
    std::chrono::system_clock::time_point current_time_;
    std::thread clock_thread_;
    
    std::string last_error_;
    
    // 统计信息
    int total_launches_;
    int total_clicks_;
};

// TaskbarManager 实现
TaskbarManager::TaskbarManager() : impl_(std::make_unique<Impl>()) {}

TaskbarManager::~TaskbarManager() = default;

bool TaskbarManager::initialize(std::shared_ptr<ITaskbarRenderer> renderer) {
    return impl_->initialize(renderer);
}

void TaskbarManager::show() {
    impl_->show();
}

void TaskbarManager::hide() {
    impl_->hide();
}

void TaskbarManager::refresh() {
    impl_->refresh();
}

void TaskbarManager::setAppearance(const TaskbarAppearance& appearance) {
    impl_->setAppearance(appearance);
}

TaskbarAppearance TaskbarManager::getAppearance() const {
    return impl_->getAppearance();
}

bool TaskbarManager::addQuickLaunchItem(const QuickLaunchItem& item) {
    return impl_->addQuickLaunchItem(item);
}

bool TaskbarManager::removeQuickLaunchItem(const std::string& item_id) {
    return impl_->removeQuickLaunchItem(item_id);
}

std::vector<QuickLaunchItem> TaskbarManager::getQuickLaunchItems() const {
    return impl_->getQuickLaunchItems();
}

bool TaskbarManager::addSystemTrayItem(const SystemTrayItem& item) {
    return impl_->addSystemTrayItem(item);
}

bool TaskbarManager::removeSystemTrayItem(const std::string& item_id) {
    return impl_->removeSystemTrayItem(item_id);
}

std::vector<SystemTrayItem> TaskbarManager::getSystemTrayItems() const {
    return impl_->getSystemTrayItems();
}

void TaskbarManager::setClockFormat(const ClockFormat& format) {
    impl_->setClockFormat(format);
}

ClockFormat TaskbarManager::getClockFormat() const {
    return impl_->getClockFormat();
}

bool TaskbarManager::addWindowToList(const std::string& window_id, const std::string& window_title) {
    return impl_->addWindowToList(window_id, window_title);
}

bool TaskbarManager::removeWindowFromList(const std::string& window_id) {
    return impl_->removeWindowFromList(window_id);
}

void TaskbarManager::setWindowActive(const std::string& window_id, bool is_active) {
    impl_->setWindowActive(window_id, is_active);
}

void TaskbarManager::setWindowMinimized(const std::string& window_id, bool is_minimized) {
    impl_->setWindowMinimized(window_id, is_minimized);
}

std::map<std::string, std::string> TaskbarManager::getWindowList() const {
    return impl_->getWindowList();
}

void TaskbarManager::handleMouseClick(int x, int y, int button) {
    impl_->handleMouseClick(x, y, button);
}

void TaskbarManager::handleMouseMove(int x, int y) {
    impl_->handleMouseMove(x, y);
}

void TaskbarManager::handleKeyboardEvent(int key_code, bool ctrl_pressed, bool shift_pressed) {
    impl_->handleKeyboardEvent(key_code, ctrl_pressed, shift_pressed);
}

void TaskbarManager::addEventListener(std::function<void(const TaskbarEvent&)> callback) {
    impl_->addEventListener(callback);
}

bool TaskbarManager::saveConfig(const std::string& config_path) {
    return impl_->saveConfig(config_path);
}

bool TaskbarManager::loadConfig(const std::string& config_path) {
    return impl_->loadConfig(config_path);
}

std::string TaskbarManager::getLastError() const {
    return impl_->getLastError();
}

bool TaskbarManager::isVisible() const {
    return impl_->isVisible();
}

void TaskbarManager::toggleAutoHide() {
    impl_->toggleAutoHide();
}

std::string TaskbarManager::getStatistics() const {
    return impl_->getStatistics();
}

} // namespace Desktop
} // namespace CloudFlow