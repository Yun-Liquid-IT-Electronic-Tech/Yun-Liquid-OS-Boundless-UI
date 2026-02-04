/**
 * @file window.h
 * @brief 窗口类定义
 * @author 云流操作系统开发团队
 * @date 2026-02-04
 * @version 1.0.0
 * 
 * 定义窗口的基本属性和行为，包括创建、销毁、移动、调整大小等操作
 */

#ifndef CLOUDFLOW_WINDOW_H
#define CLOUDFLOW_WINDOW_H

#include "window_event.h"
#include <memory>
#include <string>
#include <functional>

namespace CloudFlow {
namespace UI {

// 前置声明
class WindowImpl;

/**
 * @brief 窗口类
 * 
 * 封装窗口的基本操作和属性，采用PIMPL模式隐藏实现细节
 */
class Window {
public:
    /**
     * @brief 构造函数
     * @param id 窗口ID
     * @param title 窗口标题
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param type 窗口类型
     */
    Window(int id, const std::string& title, int width, int height, WindowType type);
    
    /**
     * @brief 析构函数
     */
    ~Window();
    
    // 禁用拷贝和赋值
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    
    /**
     * @brief 获取窗口ID
     * @return 窗口ID
     */
    int getId() const;
    
    /**
     * @brief 获取窗口标题
     * @return 窗口标题
     */
    std::string getTitle() const;
    
    /**
     * @brief 设置窗口标题
     * @param title 新标题
     * @return 成功返回true
     */
    bool setTitle(const std::string& title);
    
    /**
     * @brief 获取窗口几何信息
     * @return 窗口几何信息
     */
    WindowGeometry getGeometry() const;
    
    /**
     * @brief 获取窗口状态
     * @return 窗口状态
     */
    WindowState getState() const;
    
    /**
     * @brief 获取窗口类型
     * @return 窗口类型
     */
    WindowType getType() const;
    
    /**
     * @brief 移动窗口
     * @param x 新的X坐标
     * @param y 新的Y坐标
     * @return 成功返回true
     */
    bool move(int x, int y);
    
    /**
     * @brief 调整窗口大小
     * @param width 新的宽度
     * @param height 新的高度
     * @return 成功返回true
     */
    bool resize(int width, int height);
    
    /**
     * @brief 最小化窗口
     * @return 成功返回true
     */
    bool minimize();
    
    /**
     * @brief 最大化窗口
     * @return 成功返回true
     */
    bool maximize();
    
    /**
     * @brief 恢复窗口
     * @return 成功返回true
     */
    bool restore();
    
    /**
     * @brief 设置全屏状态
     * @param fullscreen 是否全屏
     * @return 成功返回true
     */
    bool setFullscreen(bool fullscreen);
    
    /**
     * @brief 显示窗口
     * @return 成功返回true
     */
    bool show();
    
    /**
     * @brief 隐藏窗口
     * @return 成功返回true
     */
    bool hide();
    
    /**
     * @brief 关闭窗口
     * @return 成功返回true
     */
    bool close();
    
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
     * @brief 设置窗口是否可调整大小
     * @param resizable 是否可调整大小
     * @return 成功返回true
     */
    bool setResizable(bool resizable);
    
    /**
     * @brief 设置窗口是否可移动
     * @param movable 是否可移动
     * @return 成功返回true
     */
    bool setMovable(bool movable);
    
    /**
     * @brief 设置窗口是否总在最前
     * @param always_on_top 是否总在最前
     * @return 成功返回true
     */
    bool setAlwaysOnTop(bool always_on_top);
    
    /**
     * @brief 设置窗口最小尺寸
     * @param min_width 最小宽度
     * @param min_height 最小高度
     * @return 成功返回true
     */
    bool setMinimumSize(int min_width, int min_height);
    
    /**
     * @brief 设置窗口最大尺寸
     * @param max_width 最大宽度
     * @param max_height 最大高度
     * @return 成功返回true
     */
    bool setMaximumSize(int max_width, int max_height);
    
    /**
     * @brief 设置窗口透明度
     * @param opacity 透明度(0.0-1.0)
     * @return 成功返回true
     */
    bool setOpacity(float opacity);
    
    /**
     * @brief 获取窗口是否可见
     * @return 是否可见
     */
    bool isVisible() const;
    
    /**
     * @brief 获取窗口是否获得焦点
     * @return 是否获得焦点
     */
    bool hasFocus() const;
    
    /**
     * @brief 设置窗口焦点
     * @return 成功返回true
     */
    bool setFocus();
    
    /**
     * @brief 更新窗口内容
     * @return 成功返回true
     */
    bool update();
    
    /**
     * @brief 重绘窗口
     * @return 成功返回true
     */
    bool repaint();

private:
    std::unique_ptr<WindowImpl> impl_;
};

} // namespace UI
} // namespace CloudFlow

#endif // CLOUDFLOW_WINDOW_H