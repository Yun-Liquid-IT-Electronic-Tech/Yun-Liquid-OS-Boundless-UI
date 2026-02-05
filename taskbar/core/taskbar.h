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
