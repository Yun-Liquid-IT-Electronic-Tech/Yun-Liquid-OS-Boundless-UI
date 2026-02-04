/**
 * @file window_event.h
 * @brief 窗口事件定义
 * @author 云流操作系统开发团队
 * @date 2026-02-04
 * @version 1.0.0
 * 
 * 定义窗口系统中使用的各种事件类型和数据结构
 */

#ifndef CLOUDFLOW_WINDOW_EVENT_H
#define CLOUDFLOW_WINDOW_EVENT_H

#include <cstdint>
#include <string>

namespace CloudFlow {
namespace UI {

/**
 * @brief 窗口事件类型枚举
 */
enum class WindowEventType {
    Created,        ///< 窗口创建
    Closing,        ///< 窗口正在关闭
    Destroyed,      ///< 窗口销毁
    FocusGained,    ///< 获得焦点
    FocusLost,      ///< 失去焦点
    Moved,          ///< 窗口移动
    Resized,        ///< 窗口大小改变
    Minimized,      ///< 窗口最小化
    Maximized,      ///< 窗口最大化
    Restored,       ///< 窗口恢复
    StateChanged,   ///< 窗口状态改变
    MouseEnter,     ///< 鼠标进入窗口
    MouseLeave,     ///< 鼠标离开窗口
    MouseMove,      ///< 鼠标移动
    MousePress,     ///< 鼠标按下
    MouseRelease,   ///< 鼠标释放
    MouseWheel,     ///< 鼠标滚轮
    KeyPress,       ///< 按键按下
    KeyRelease,     ///< 按键释放
    CloseRequest,   ///< 关闭请求
    DragBegin,      ///< 拖拽开始
    DragMove,       ///< 拖拽移动
    DragEnd         ///< 拖拽结束
};

/**
 * @brief 鼠标按钮枚举
 */
enum class MouseButton {
    None,   ///< 无按钮
    Left,   ///< 左键
    Right,  ///< 右键
    Middle, ///< 中键
    Extra1, ///< 额外按钮1
    Extra2  ///< 额外按钮2
};

/**
 * @brief 键盘修饰键标志
 */
enum class KeyModifier {
    None      = 0,      ///< 无修饰键
    Shift     = 1 << 0, ///< Shift键
    Control   = 1 << 1, ///< Ctrl键
    Alt       = 1 << 2, ///< Alt键
    Meta      = 1 << 3, ///< Meta键(Windows键/Command键)
    CapsLock  = 1 << 4, ///< CapsLock键
    NumLock   = 1 << 5  ///< NumLock键
};

/**
 * @brief 窗口事件结构体
 */
struct WindowEvent {
    WindowEventType type;        ///< 事件类型
    int window_id;              ///< 窗口ID
    uint64_t timestamp;         ///< 时间戳(毫秒)
    
    // 鼠标事件数据
    struct {
        int x;                  ///< 鼠标X坐标(相对于窗口)
        int y;                  ///< 鼠标Y坐标(相对于窗口)
        int global_x;           ///< 全局X坐标
        int global_y;           ///< 全局Y坐标
        MouseButton button;     ///< 鼠标按钮
        int buttons;            ///< 当前按下的按钮掩码
        KeyModifier modifiers;  ///< 键盘修饰键
        int delta_x;            ///< 鼠标移动X增量
        int delta_y;            ///< 鼠标移动Y增量
        int wheel_delta;        ///< 滚轮增量
    } mouse;
    
    // 键盘事件数据
    struct {
        int key_code;           ///< 键码
        std::string key_text;   ///< 按键文本
        KeyModifier modifiers;  ///< 键盘修饰键
        bool is_auto_repeat;    ///< 是否为自动重复
    } keyboard;
    
    // 窗口事件数据
    struct {
        int x;                  ///< 窗口X坐标
        int y;                  ///< 窗口Y坐标
        int width;              ///< 窗口宽度
        int height;             ///< 窗口高度
        int old_x;              ///< 旧X坐标
        int old_y;              ///< 旧Y坐标
        int old_width;          ///< 旧宽度
        int old_height;         ///< 旧高度
    } window;
    
    // 拖拽事件数据
    struct {
        int start_x;            ///< 拖拽起始X坐标
        int start_y;            ///< 拖拽起始Y坐标
        int current_x;          ///< 当前X坐标
        int current_y;          ///< 当前Y坐标
        void* drag_data;        ///< 拖拽数据指针
        size_t data_size;       ///< 数据大小
    } drag;
    
    // 通用数据
    union {
        int int_value;          ///< 整数值
        float float_value;      ///< 浮点数值
        void* pointer_value;    ///< 指针值
    } data;
    
    /**
     * @brief 构造函数
     */
    WindowEvent() 
        : type(WindowEventType::Created)
        , window_id(-1)
        , timestamp(0) {
        // 初始化所有成员
        mouse.x = 0;
        mouse.y = 0;
        mouse.global_x = 0;
        mouse.global_y = 0;
        mouse.button = MouseButton::None;
        mouse.buttons = 0;
        mouse.modifiers = KeyModifier::None;
        mouse.delta_x = 0;
        mouse.delta_y = 0;
        mouse.wheel_delta = 0;
        
        keyboard.key_code = 0;
        keyboard.modifiers = KeyModifier::None;
        keyboard.is_auto_repeat = false;
        
        window.x = 0;
        window.y = 0;
        window.width = 0;
        window.height = 0;
        window.old_x = 0;
        window.old_y = 0;
        window.old_width = 0;
        window.old_height = 0;
        
        drag.start_x = 0;
        drag.start_y = 0;
        drag.current_x = 0;
        drag.current_y = 0;
        drag.drag_data = nullptr;
        drag.data_size = 0;
        
        data.int_value = 0;
    }
};

/**
 * @brief 键盘修饰键操作符重载
 */
inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<int>(a) | static_cast<int>(b));
}

inline KeyModifier operator&(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<int>(a) & static_cast<int>(b));
}

inline KeyModifier& operator|=(KeyModifier& a, KeyModifier b) {
    a = a | b;
    return a;
}

inline KeyModifier& operator&=(KeyModifier& a, KeyModifier b) {
    a = a & b;
    return a;
}

/**
 * @brief 事件工具函数
 */
namespace EventUtils {
    
    /**
     * @brief 检查修饰键是否包含特定键
     */
    inline bool hasModifier(KeyModifier modifiers, KeyModifier modifier) {
        return (modifiers & modifier) != KeyModifier::None;
    }
    
    /**
     * @brief 获取当前时间戳
     */
    inline uint64_t getCurrentTimestamp() {
        // 实现获取当前时间戳的逻辑
        // 这里使用简单实现，实际项目中应使用系统时间API
        static uint64_t counter = 0;
        return ++counter;
    }
    
    /**
     * @brief 创建窗口创建事件
     */
    inline WindowEvent createWindowCreatedEvent(int window_id) {
        WindowEvent event;
        event.type = WindowEventType::Created;
        event.window_id = window_id;
        event.timestamp = getCurrentTimestamp();
        return event;
    }
    
    /**
     * @brief 创建窗口移动事件
     */
    inline WindowEvent createWindowMovedEvent(int window_id, int x, int y, int old_x, int old_y) {
        WindowEvent event;
        event.type = WindowEventType::Moved;
        event.window_id = window_id;
        event.timestamp = getCurrentTimestamp();
        event.window.x = x;
        event.window.y = y;
        event.window.old_x = old_x;
        event.window.old_y = old_y;
        return event;
    }
    
    /**
     * @brief 创建窗口大小改变事件
     */
    inline WindowEvent createWindowResizedEvent(int window_id, int width, int height, int old_width, int old_height) {
        WindowEvent event;
        event.type = WindowEventType::Resized;
        event.window_id = window_id;
        event.timestamp = getCurrentTimestamp();
        event.window.width = width;
        event.window.height = height;
        event.window.old_width = old_width;
        event.window.old_height = old_height;
        return event;
    }
    
    /**
     * @brief 创建鼠标移动事件
     */
    inline WindowEvent createMouseMoveEvent(int window_id, int x, int y, int global_x, int global_y, KeyModifier modifiers) {
        WindowEvent event;
        event.type = WindowEventType::MouseMove;
        event.window_id = window_id;
        event.timestamp = getCurrentTimestamp();
        event.mouse.x = x;
        event.mouse.y = y;
        event.mouse.global_x = global_x;
        event.mouse.global_y = global_y;
        event.mouse.modifiers = modifiers;
        return event;
    }
    
} // namespace EventUtils

} // namespace UI
} // namespace CloudFlow

#endif // CLOUDFLOW_WINDOW_EVENT_H