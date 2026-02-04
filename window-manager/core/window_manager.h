/**
 * @file window_manager.h
 * @brief 窗口管理器核心类定义
 * @author 云流操作系统开发团队
 * @date 2026-02-04
 * @version 1.0.0
 * 
 * 负责管理桌面环境中的窗口创建、销毁、布局、焦点管理等核心功能
 */

#ifndef CLOUDFLOW_WINDOW_MANAGER_H
#define CLOUDFLOW_WINDOW_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace CloudFlow {
namespace UI {

// 前置声明
class Window;
class WindowEvent;

/**
 * @brief 窗口状态枚举
 */
enum class WindowState {
    Normal,     ///< 正常状态
    Minimized,  ///< 最小化
    Maximized,  ///< 最大化
    Fullscreen, ///< 全屏
    Hidden      ///< 隐藏
};

/**
 * @brief 窗口类型枚举
 */
enum class WindowType {
    Normal,     ///< 普通窗口
    Dialog,     ///< 对话框
    Tooltip,    ///< 工具提示
    Popup,      ///< 弹出窗口
    Utility     ///< 工具窗口
};

/**
 * @brief 窗口几何信息结构体
 */
struct WindowGeometry {
    int x;          ///< X坐标
    int y;          ///< Y坐标
    int width;      ///< 宽度
    int height;     ///< 高度
    int min_width;  ///< 最小宽度
    int min_height; ///< 最小高度
    int max_width;  ///< 最大宽度
    int max_height; ///< 最大高度
};

/**
 * @brief 窗口管理器类
 * 
 * 采用PIMPL模式实现，隐藏实现细节，提供稳定的API接口
 */
class WindowManager {
public:
    /**
     * @brief 构造函数
     */
    WindowManager();
    
    /**
     * @brief 析构函数
     */
    ~WindowManager();
    
    // 禁用拷贝和赋值
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    /**
     * @brief 创建新窗口
     * @param title 窗口标题
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param type 窗口类型
     * @return 窗口ID，创建失败返回-1
     */
    int createWindow(const std::string& title, int width, int height, WindowType type = WindowType::Normal);
    
    /**
     * @brief 关闭窗口
     * @param window_id 窗口ID
     * @return 成功返回true，失败返回false
     */
    bool closeWindow(int window_id);
    
    /**
     * @brief 设置窗口焦点
     * @param window_id 窗口ID
     * @return 成功返回true，失败返回false
     */
    bool setFocus(int window_id);
    
    /**
     * @brief 获取当前焦点窗口ID
     * @return 焦点窗口ID，无焦点窗口返回-1
     */
    int getFocusedWindow() const;
    
    /**
     * @brief 最小化窗口
     * @param window_id 窗口ID
     * @return 成功返回true，失败返回false
     */
    bool minimizeWindow(int window_id);
    
    /**
     * @brief 最大化窗口
     * @param window_id 窗口ID
     * @return 成功返回true，失败返回false
     */
    bool maximizeWindow(int window_id);
    
    /**
     * @brief 恢复窗口
     * @param window_id 窗口ID
     * @return 成功返回true，失败返回false
     */
    bool restoreWindow(int window_id);
    
    /**
     * @brief 移动窗口
     * @param window_id 窗口ID
     * @param x 新的X坐标
     * @param y 新的Y坐标
     * @return 成功返回true，失败返回false
     */
    bool moveWindow(int window_id, int x, int y);
    
    /**
     * @brief 调整窗口大小
     * @param window_id 窗口ID
     * @param width 新的宽度
     * @param height 新的高度
     * @return 成功返回true，失败返回false
     */
    bool resizeWindow(int window_id, int width, int height);
    
    /**
     * @brief 获取窗口数量
     * @return 当前管理的窗口数量
     */
    size_t getWindowCount() const;
    
    /**
     * @brief 获取所有窗口ID
     * @return 窗口ID列表
     */
    std::vector<int> getWindowIds() const;
    
    /**
     * @brief 获取窗口几何信息
     * @param window_id 窗口ID
     * @return 窗口几何信息，窗口不存在返回空结构体
     */
    WindowGeometry getWindowGeometry(int window_id) const;
    
    /**
     * @brief 设置窗口事件回调
     * @param callback 事件回调函数
     */
    void setEventCallback(std::function<void(const WindowEvent&)> callback);
    
    /**
     * @brief 处理窗口事件
     * @param event 窗口事件
     */
    void handleEvent(const WindowEvent& event);
    
    /**
     * @brief 保存窗口状态
     * @param filename 保存文件名
     * @return 成功返回true，失败返回false
     */
    bool saveWindowState(const std::string& filename) const;
    
    /**
     * @brief 恢复窗口状态
     * @param filename 状态文件名
     * @return 成功返回true，失败返回false
     */
    bool restoreWindowState(const std::string& filename);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace UI
} // namespace CloudFlow

#endif // CLOUDFLOW_WINDOW_MANAGER_H