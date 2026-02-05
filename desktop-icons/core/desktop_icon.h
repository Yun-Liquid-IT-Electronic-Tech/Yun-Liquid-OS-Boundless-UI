/**
 * @file desktop_icon.h
 * @brief 桌面图标管理器头文件
 * 
 * 负责管理桌面图标的显示、排列、拖拽和交互
 */

#ifndef CLOUDFLOW_DESKTOP_ICON_H
#define CLOUDFLOW_DESKTOP_ICON_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace CloudFlow {
namespace Desktop {

/**
 * @enum IconType
 * @brief 图标类型枚举
 */
enum class IconType {
    Application,    ///< 应用程序
    File,           ///< 文件
    Folder,         ///< 文件夹
    System,         ///< 系统图标
    Network,        ///< 网络图标
    Trash,          ///< 回收站
    Custom          ///< 自定义图标
};

/**
 * @enum IconSize
 * @brief 图标尺寸枚举
 */
enum class IconSize {
    Small,          ///< 小图标 (32x32)
    Medium,         ///< 中图标 (48x48)
    Large,          ///< 大图标 (64x64)
    ExtraLarge      ///< 超大图标 (96x96)
};

/**
 * @enum IconArrangement
 * @brief 图标排列方式枚举
 */
enum class IconArrangement {
    AutoArrange,    ///< 自动排列
    SnapToGrid,     ///< 对齐网格
    FreeArrange     ///< 自由排列
};

/**
 * @struct IconPosition
 * @brief 图标位置信息
 */
struct IconPosition {
    int grid_x;     ///< 网格X坐标
    int grid_y;     ///< 网格Y坐标
    int pixel_x;    ///< 像素X坐标
    int pixel_y;    ///< 像素Y坐标
    
    IconPosition(int x = 0, int y = 0) : grid_x(x), grid_y(y), pixel_x(x * 64), pixel_y(y * 64) {}
};

/**
 * @struct DesktopIcon
 * @brief 桌面图标信息
 */
struct DesktopIcon {
    std::string id;                ///< 图标唯一标识
    std::string name;              ///< 显示名称
    std::string icon_path;          ///< 图标路径
    IconType type;                 ///< 图标类型
    IconPosition position;         ///< 图标位置
    bool selected;                 ///< 是否选中
    bool visible;                  ///< 是否可见
    std::chrono::system_clock::time_point created_time; ///< 创建时间
    std::chrono::system_clock::time_point modified_time; ///< 修改时间
    
    // 应用相关属性
    std::string executable_path;   ///< 可执行文件路径（仅应用程序）
    std::vector<std::string> arguments; ///< 启动参数
    
    // 文件相关属性
    std::string file_path;         ///< 文件路径（仅文件和文件夹）
    uint64_t file_size;            ///< 文件大小
    
    DesktopIcon() : type(IconType::Custom), selected(false), visible(true), file_size(0) {}
};

/**
 * @struct DesktopIconEvent
 * @brief 桌面图标事件
 */
struct DesktopIconEvent {
    enum class Type {
        Click,              ///< 单击
        DoubleClick,        ///< 双击
        RightClick,         ///< 右键点击
        DragStart,          ///< 拖拽开始
        DragMove,           ///< 拖拽移动
        DragEnd,            ///< 拖拽结束
        SelectionChanged,   ///< 选择改变
        ContextMenu         ///< 上下文菜单
    };
    
    Type type;                     ///< 事件类型
    DesktopIcon* icon;              ///< 相关图标
    IconPosition position;          ///< 事件位置
    bool ctrl_pressed;              ///< Ctrl键是否按下
    bool shift_pressed;             ///< Shift键是否按下
    
    DesktopIconEvent(Type t = Type::Click) : type(t), icon(nullptr), ctrl_pressed(false), shift_pressed(false) {}
};

/**
 * @class IDesktopIconRenderer
 * @brief 桌面图标渲染器接口
 */
class IDesktopIconRenderer {
public:
    virtual ~IDesktopIconRenderer() = default;
    
    /**
     * @brief 渲染图标
     * @param icon 图标信息
     * @param size 图标尺寸
     */
    virtual void renderIcon(const DesktopIcon& icon, IconSize size) = 0;
    
    /**
     * @brief 渲染图标标签
     * @param icon 图标信息
     */
    virtual void renderIconLabel(const DesktopIcon& icon) = 0;
    
    /**
     * @brief 渲染选择框
     * @param position 位置
     * @param size 尺寸
     */
    virtual void renderSelectionBox(const IconPosition& position, const IconPosition& size) = 0;
    
    /**
     * @brief 获取图标尺寸
     * @param size 图标尺寸枚举
     * @return 实际像素尺寸
     */
    virtual int getIconSize(IconSize size) const = 0;
};

/**
 * @class DesktopIconManager
 * @brief 桌面图标管理器
 * 
 * 负责管理桌面图标的生命周期、排列、选择和交互
 */
class DesktopIconManager {
public:
    /**
     * @brief 构造函数
     */
    DesktopIconManager();
    
    /**
     * @brief 析构函数
     */
    ~DesktopIconManager();
    
    /**
     * @brief 初始化图标管理器
     * @param renderer 图标渲染器
     * @return 初始化是否成功
     */
    bool initialize(std::shared_ptr<IDesktopIconRenderer> renderer);
    
    /**
     * @brief 添加图标到桌面
     * @param icon 图标信息
     * @return 添加是否成功
     */
    bool addIcon(const DesktopIcon& icon);
    
    /**
     * @brief 从桌面移除图标
     * @param icon_id 图标ID
     * @return 移除是否成功
     */
    bool removeIcon(const std::string& icon_id);
    
    /**
     * @brief 移动图标到新位置
     * @param icon_id 图标ID
     * @param new_position 新位置
     * @return 移动是否成功
     */
    bool moveIcon(const std::string& icon_id, const IconPosition& new_position);
    
    /**
     * @brief 选择图标
     * @param icon_id 图标ID
     * @param multi_select 是否多选
     * @return 选择是否成功
     */
    bool selectIcon(const std::string& icon_id, bool multi_select = false);
    
    /**
     * @brief 取消选择所有图标
     */
    void clearSelection();
    
    /**
     * @brief 获取所有图标
     * @return 图标列表
     */
    std::vector<DesktopIcon> getAllIcons() const;
    
    /**
     * @brief 获取选中的图标
     * @return 选中图标列表
     */
    std::vector<DesktopIcon> getSelectedIcons() const;
    
    /**
     * @brief 根据位置获取图标
     * @param position 位置
     * @return 图标指针，如果不存在则返回nullptr
     */
    DesktopIcon* getIconAtPosition(const IconPosition& position);
    
    /**
     * @brief 根据ID获取图标
     * @param icon_id 图标ID
     * @return 图标指针，如果不存在则返回nullptr
     */
    DesktopIcon* getIconById(const std::string& icon_id);
    
    /**
     * @brief 设置图标排列方式
     * @param arrangement 排列方式
     */
    void setArrangement(IconArrangement arrangement);
    
    /**
     * @brief 设置图标尺寸
     * @param size 图标尺寸
     */
    void setIconSize(IconSize size);
    
    /**
     * @brief 自动排列图标
     */
    void autoArrangeIcons();
    
    /**
     * @brief 刷新桌面图标显示
     */
    void refreshDesktop();
    
    /**
     * @brief 处理鼠标事件
     * @param event 图标事件
     */
    void handleMouseEvent(const DesktopIconEvent& event);
    
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
    void addEventListener(std::function<void(const DesktopIconEvent&)> callback);
    
    /**
     * @brief 保存图标布局到配置文件
     * @param config_path 配置文件路径
     * @return 保存是否成功
     */
    bool saveLayout(const std::string& config_path);
    
    /**
     * @brief 从配置文件加载图标布局
     * @param config_path 配置文件路径
     * @return 加载是否成功
     */
    bool loadLayout(const std::string& config_path);
    
    /**
     * @brief 获取错误信息
     * @return 错误描述
     */
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Desktop
} // namespace CloudFlow

#endif // CLOUDFLOW_DESKTOP_ICON_H