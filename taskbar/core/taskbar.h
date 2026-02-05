/**
 * @file taskbar.h
 * @brief 任务栏管理器头文件
 * 
 * 负责管理任务栏的显示、组件布局、交互和系统集成
 */

#ifndef CLOUDFLOW_TASKBAR_H
#define CLOUDFLOW_TASKBAR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <map>

namespace CloudFlow {
namespace Desktop {

/**
 * @enum TaskbarPosition
 * @brief 任务栏位置枚举
 */
enum class TaskbarPosition {
    Bottom,     ///< 底部
    Top,        ///< 顶部
    Left,       ///< 左侧
    Right       ///< 右侧
};

/**
 * @enum TaskbarStyle
 * @brief 任务栏样式枚举
 */
enum class TaskbarStyle {
    Classic,    ///< 经典样式
    Modern,     ///< 现代样式
    Compact     ///< 紧凑样式
};

/**
 * @enum TaskbarComponent
 * @brief 任务栏组件枚举
 */
enum class TaskbarComponent {
    StartMenu,      ///< 开始菜单
    QuickLaunch,    ///< 快速启动栏
    WindowList,     ///< 窗口列表
    SystemTray,     ///< 系统托盘
    Clock           ///< 时钟
};

/**
 * @struct TaskbarAppearance
 * @brief 任务栏外观设置
 */
struct TaskbarAppearance {
    TaskbarPosition position;      ///< 任务栏位置
    TaskbarStyle style;           ///< 任务栏样式
    int height;                   ///< 任务栏高度（像素）
    bool auto_hide;               ///< 是否自动隐藏
    bool always_on_top;           ///< 是否始终置顶
    bool show_clock;              ///< 是否显示时钟
    bool show_system_tray;        ///< 是否显示系统托盘
    
    TaskbarAppearance() : position(TaskbarPosition::Bottom), 
                          style(TaskbarStyle::Modern), 
                          height(40), 
                          auto_hide(false), 
                          always_on_top(true), 
                          show_clock(true), 
                          show_system_tray(true) {}
};

/**
 * @struct QuickLaunchItem
 * @brief 快速启动项信息
 */
struct QuickLaunchItem {
    std::string id;               ///< 项目唯一标识
    std::string name;            ///< 显示名称
    std::string icon_path;        ///< 图标路径
    std::string executable_path;  ///< 可执行文件路径
    std::vector<std::string> arguments; ///< 启动参数
    int launch_count;            ///< 启动次数（用于排序）
    
    QuickLaunchItem() : launch_count(0) {}
};

/**
 * @struct SystemTrayItem
 * @brief 系统托盘项信息
 */
struct SystemTrayItem {
    std::string id;               ///< 项目唯一标识
    std::string name;            ///< 显示名称
    std::string icon_path;        ///< 图标路径
    std::string tooltip;          ///< 工具提示
    bool visible;                 ///< 是否可见
    bool active;                  ///< 是否激活状态
    
    SystemTrayItem() : visible(true), active(false) {}
};

/**
 * @struct ClockFormat
 * @brief 时钟格式设置
 */
struct ClockFormat {
    bool show_date;               ///< 是否显示日期
    bool show_seconds;            ///< 是否显示秒数
    std::string time_format;      ///< 时间格式
    std::string date_format;      ///< 日期格式
    
    ClockFormat() : show_date(true), show_seconds(true), 
                    time_format("%H:%M:%S"), date_format("%Y-%m-%d") {}
};

/**
 * @struct TaskbarEvent
 * @brief 任务栏事件
 */
struct TaskbarEvent {
    enum class Type {
        StartMenuClicked,         ///< 开始菜单点击
        QuickLaunchItemClicked,   ///< 快速启动项点击
        WindowMinimized,          ///< 窗口最小化
        WindowRestored,           ///< 窗口恢复
        SystemTrayItemClicked,    ///< 系统托盘项点击
        ClockClicked,             ///< 时钟点击
        TaskbarResized,           ///< 任务栏大小改变
        AutoHideToggled           ///< 自动隐藏切换
    };
    
    Type type;                    ///< 事件类型
    std::string item_id;          ///< 相关项目ID
    void* user_data;              ///< 用户数据
    
    TaskbarEvent(Type t = Type::StartMenuClicked) : type(t), user_data(nullptr) {}
};

/**
 * @class ITaskbarRenderer
 * @brief 任务栏渲染器接口
 */
class ITaskbarRenderer {
public:
    virtual ~ITaskbarRenderer() = default;
    
    /**
     * @brief 渲染任务栏背景
     * @param appearance 外观设置
     */
    virtual void renderBackground(const TaskbarAppearance& appearance) = 0;
    
    /**
     * @brief 渲染开始菜单按钮
     * @param appearance 外观设置
     * @param is_active 是否激活状态
     */
    virtual void renderStartMenuButton(const TaskbarAppearance& appearance, bool is_active) = 0;
    
    /**
     * @brief 渲染快速启动项
     * @param item 快速启动项
     * @param appearance 外观设置
     */
    virtual void renderQuickLaunchItem(const QuickLaunchItem& item, const TaskbarAppearance& appearance) = 0;
    
    /**
     * @brief 渲染窗口列表项
     * @param window_id 窗口ID
     * @param window_title 窗口标题
     * @param is_active 是否激活状态
     * @param is_minimized 是否最小化
     * @param appearance 外观设置
     */
    virtual void renderWindowListItem(const std::string& window_id, 
                                     const std::string& window_title, 
                                     bool is_active, 
                                     bool is_minimized, 
                                     const TaskbarAppearance& appearance) = 0;
    
    /**
     * @brief 渲染系统托盘项
     * @param item 系统托盘项
     * @param appearance 外观设置
     */
    virtual void renderSystemTrayItem(const SystemTrayItem& item, const TaskbarAppearance& appearance) = 0;
    
    /**
     * @brief 渲染时钟
     * @param current_time 当前时间
     * @param format 时钟格式
     * @param appearance 外观设置
     */
    virtual void renderClock(const std::chrono::system_clock::time_point& current_time, 
                            const ClockFormat& format, 
                            const TaskbarAppearance& appearance) = 0;
    
    /**
     * @brief 获取任务栏尺寸
     * @param appearance 外观设置
     * @return 任务栏尺寸（宽度，高度）
     */
    virtual std::pair<int, int> getTaskbarSize(const TaskbarAppearance& appearance) = 0;
};

/**
 * @class TaskbarManager
 * @brief 任务栏管理器
 * 
 * 负责管理任务栏的显示、组件布局、交互和系统集成
 */
class TaskbarManager {
public:
    /**
     * @brief 构造函数
     */
    TaskbarManager();
    
    /**
     * @brief 析构函数
     */
    ~TaskbarManager();
    
    /**
     * @brief 初始化任务栏管理器
     * @param renderer 任务栏渲染器
     * @return 初始化是否成功
     */
    bool initialize(std::shared_ptr<ITaskbarRenderer> renderer);
    
    /**
     * @brief 显示任务栏
     */
    void show();
    
    /**
     * @brief 隐藏任务栏
     */
    void hide();
    
    /**
     * @brief 刷新任务栏显示
     */
    void refresh();
    
    /**
     * @brief 设置任务栏外观
     * @param appearance 外观设置
     */
    void setAppearance(const TaskbarAppearance& appearance);
    
    /**
     * @brief 获取当前外观设置
     * @return 外观设置
     */
    TaskbarAppearance getAppearance() const;
    
    /**
     * @brief 添加快速启动项
     * @param item 快速启动项
     * @return 添加是否成功
     */
    bool addQuickLaunchItem(const QuickLaunchItem& item);
    
    /**
     * @brief 移除快速启动项
     * @param item_id 项目ID
     * @return 移除是否成功
     */
    bool removeQuickLaunchItem(const std::string& item_id);
    
    /**
     * @brief 获取所有快速启动项
     * @return 快速启动项列表
     */
    std::vector<QuickLaunchItem> getQuickLaunchItems() const;
    
    /**
     * @brief 添加系统托盘项
     * @param item 系统托盘项
     * @return 添加是否成功
     */
    bool addSystemTrayItem(const SystemTrayItem& item);
    
    /**
     * @brief 移除系统托盘项
     * @param item_id 项目ID
     * @return 移除是否成功
     */
    bool removeSystemTrayItem(const std::string& item_id);
    
    /**
     * @brief 获取所有系统托盘项
     * @return 系统托盘项列表
     */
    std::vector<SystemTrayItem> getSystemTrayItems() const;
    
    /**
     * @brief 设置时钟格式
     * @param format 时钟格式
     */
    void setClockFormat(const ClockFormat& format);
    
    /**
     * @brief 获取当前时钟格式
     * @return 时钟格式
     */
    ClockFormat getClockFormat() const;
    
    /**
     * @brief 添加窗口到窗口列表
     * @param window_id 窗口ID
     * @param window_title 窗口标题
     * @return 添加是否成功
     */
    bool addWindowToList(const std::string& window_id, const std::string& window_title);
    
    /**
     * @brief 从窗口列表移除窗口
     * @param window_id 窗口ID
     * @return 移除是否成功
     */
    bool removeWindowFromList(const std::string& window_id);
    
    /**
     * @brief 设置窗口激活状态
     * @param window_id 窗口ID
     * @param is_active 是否激活
     */
    void setWindowActive(const std::string& window_id, bool is_active);
    
    /**
     * @brief 设置窗口最小化状态
     * @param window_id 窗口ID
     * @param is_minimized 是否最小化
     */
    void setWindowMinimized(const std::string& window_id, bool is_minimized);
    
    /**
     * @brief 获取窗口列表
     * @return 窗口ID和标题的映射
     */
    std::map<std::string, std::string> getWindowList() const;
    
    /**
     * @brief 处理鼠标点击事件
     * @param x 鼠标X坐标
     * @param y 鼠标Y坐标
     * @param button 鼠标按钮（1=左键，2=右键，3=中键）
     */
    void handleMouseClick(int x, int y, int button);
    
    /**
     * @brief 处理鼠标移动事件
     * @param x 鼠标X坐标
     * @param y 鼠标Y坐标
     */
    void handleMouseMove(int x, int y);
    
    /**
     * @brief 处理键盘事件
     * @param key_code 键码
     * @param ctrl_pressed Ctrl键状态
     * @param shift_pressed Shift键状态
     */
    void handleKeyboardEvent(int key_code, bool ctrl_pressed, bool shift_pressed);
    
    /**
     * @brief 添加事件监听器
     * @param callback 回调函数
     */
    void addEventListener(std::function<void(const TaskbarEvent&)> callback);
    
    /**
     * @brief 保存任务栏配置
     * @param config_path 配置文件路径
     * @return 保存是否成功
     */
    bool saveConfig(const std::string& config_path);
    
    /**
     * @brief 加载任务栏配置
     * @param config_path 配置文件路径
     * @return 加载是否成功
     */
    bool loadConfig(const std::string& config_path);
    
    /**
     * @brief 获取错误信息
     * @return 错误描述
     */
    std::string getLastError() const;
    
    /**
     * @brief 获取任务栏是否可见
     * @return 是否可见
     */
    bool isVisible() const;
    
    /**
     * @brief 切换自动隐藏状态
     */
    void toggleAutoHide();
    
    /**
     * @brief 获取任务栏统计信息
     * @return 统计信息字符串
     */
    std::string getStatistics() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Desktop
} // namespace CloudFlow

#endif // CLOUDFLOW_TASKBAR_H