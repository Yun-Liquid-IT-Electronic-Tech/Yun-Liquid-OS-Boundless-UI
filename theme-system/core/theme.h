/**
 * @file theme.h
 * @brief 主题系统管理器头文件
 * 
 * 负责管理桌面环境的视觉主题，包括颜色、图标、字体、窗口装饰等
 */

#ifndef CLOUDFLOW_THEME_H
#define CLOUDFLOW_THEME_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <unordered_map>

namespace CloudFlow {
namespace Desktop {

/**
 * @enum ThemeType
 * @brief 主题类型枚举
 */
enum class ThemeType {
    Light,         ///< 浅色主题
    Dark,          ///< 深色主题
    HighContrast,  ///< 高对比度主题
    Custom         ///< 自定义主题
};

/**
 * @enum ColorScheme
 * @brief 颜色方案枚举
 */
enum class ColorScheme {
    Default,       ///< 默认配色
    Blue,          ///< 蓝色配色
    Green,         ///< 绿色配色
    Purple,        ///< 紫色配色
    Orange,        ///< 橙色配色
    Red            ///< 红色配色
};

/**
 * @enum AnimationStyle
 * @brief 动画样式枚举
 */
enum class AnimationStyle {
    None,          ///< 无动画
    Minimal,       ///< 最小动画
    Smooth,        ///< 平滑动画
    Bouncy         ///< 弹性动画
};

/**
 * @struct RGBColor
 * @brief RGB颜色值
 */
struct RGBColor {
    int red;       ///< 红色分量 (0-255)
    int green;     ///< 绿色分量 (0-255)
    int blue;      ///< 蓝色分量 (0-255)
    
    RGBColor(int r = 0, int g = 0, int b = 0) : red(r), green(g), blue(b) {}
    
    std::string toHex() const {
        char hex[8];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", red, green, blue);
        return std::string(hex);
    }
    
    static RGBColor fromHex(const std::string& hex) {
        if (hex.length() != 7 || hex[0] != '#') {
            return RGBColor();
        }
        
        int r, g, b;
        sscanf(hex.c_str(), "#%02X%02X%02X", &r, &g, &b);
        return RGBColor(r, g, b);
    }
};

/**
 * @struct ColorPalette
 * @brief 颜色调色板
 */
struct ColorPalette {
    RGBColor primary;           ///< 主色调
    RGBColor secondary;         ///< 辅助色调
    RGBColor accent;           ///< 强调色调
    RGBColor background;       ///< 背景色
    RGBColor surface;          ///< 表面色
    RGBColor text_primary;     ///< 主要文本色
    RGBColor text_secondary;   ///< 次要文本色
    RGBColor error;            ///< 错误色
    RGBColor warning;          ///< 警告色
    RGBColor success;          ///< 成功色
    RGBColor info;             ///< 信息色
    
    ColorPalette() : 
        primary(33, 150, 243),     // 蓝色
        secondary(156, 39, 176),   // 紫色
        accent(255, 193, 7),       // 黄色
        background(255, 255, 255), // 白色
        surface(245, 245, 245),    // 浅灰色
        text_primary(33, 33, 33),  // 深灰色
        text_secondary(117, 117, 117), // 中灰色
        error(244, 67, 54),        // 红色
        warning(255, 152, 0),      // 橙色
        success(76, 175, 80),      // 绿色
        info(3, 169, 244) {}       // 浅蓝色
};

/**
 * @struct FontSettings
 * @brief 字体设置
 */
struct FontSettings {
    std::string family;        ///< 字体家族
    int size;                  ///< 字体大小
    bool bold;                 ///< 是否粗体
    bool italic;               ///< 是否斜体
    int weight;                ///< 字体权重 (100-900)
    
    FontSettings() : family("Noto Sans"), size(12), bold(false), italic(false), weight(400) {}
};

/**
 * @struct IconTheme
 * @brief 图标主题设置
 */
struct IconTheme {
    std::string name;          ///< 图标主题名称
    std::string path;          ///< 图标路径
    int size_small;           ///< 小图标尺寸
    int size_medium;          ///< 中图标尺寸
    int size_large;           ///< 大图标尺寸
    bool symbolic;             ///< 是否使用符号图标
    
    IconTheme() : name("default"), path("/usr/share/icons/default"), 
                  size_small(16), size_medium(24), size_large(32), symbolic(false) {}
};

/**
 * @struct WindowDecoration
 * @brief 窗口装饰设置
 */
struct WindowDecoration {
    RGBColor border_color;     ///< 边框颜色
    int border_width;          ///< 边框宽度
    RGBColor title_bar_color;  ///< 标题栏颜色
    int title_bar_height;      ///< 标题栏高度
    bool rounded_corners;      ///< 是否圆角
    int corner_radius;         ///< 圆角半径
    bool shadows;              ///< 是否显示阴影
    RGBColor shadow_color;     ///< 阴影颜色
    int shadow_blur;           ///< 阴影模糊半径
    
    WindowDecoration() : 
        border_color(200, 200, 200), border_width(1),
        title_bar_color(240, 240, 240), title_bar_height(30),
        rounded_corners(true), corner_radius(8),
        shadows(true), shadow_color(0, 0, 0), shadow_blur(10) {}
};

/**
 * @struct AnimationSettings
 * @brief 动画设置
 */
struct AnimationSettings {
    AnimationStyle style;      ///< 动画样式
    int duration;              ///< 动画时长 (毫秒)
    bool enable_transitions;   ///< 是否启用过渡动画
    bool enable_effects;       ///< 是否启用特效
    float easing_factor;       ///< 缓动因子
    
    AnimationSettings() : style(AnimationStyle::Smooth), duration(300), 
                         enable_transitions(true), enable_effects(true), easing_factor(0.8f) {}
};

/**
 * @struct ThemeSettings
 * @brief 主题设置
 */
struct ThemeSettings {
    ThemeType type;            ///< 主题类型
    ColorScheme scheme;        ///< 颜色方案
    ColorPalette palette;     ///< 颜色调色板
    FontSettings font;         ///< 字体设置
    IconTheme icons;           ///< 图标主题
    WindowDecoration window;  ///< 窗口装饰
    AnimationSettings animation; ///< 动画设置
    std::string name;         ///< 主题名称
    std::string version;      ///< 主题版本
    std::string author;       ///< 主题作者
    std::string description;  ///< 主题描述
    
    ThemeSettings() : type(ThemeType::Light), scheme(ColorScheme::Default), 
                      name("默认主题"), version("1.0.0"), author("云流操作系统"), 
                      description("默认桌面主题") {}
};

/**
 * @struct ThemeEvent
 * @brief 主题事件
 */
struct ThemeEvent {
    enum class Type {
        ThemeChanged,          ///< 主题改变
        ColorSchemeChanged,    ///< 颜色方案改变
        FontChanged,           ///< 字体改变
        IconThemeChanged,      ///< 图标主题改变
        WindowDecorationChanged, ///< 窗口装饰改变
        AnimationChanged       ///< 动画设置改变
    };
    
    Type type;                ///< 事件类型
    std::string theme_name;   ///< 主题名称
    void* user_data;          ///< 用户数据
    
    ThemeEvent(Type t = Type::ThemeChanged) : type(t), user_data(nullptr) {}
};

/**
 * @class IThemeRenderer
 * @brief 主题渲染器接口
 */
class IThemeRenderer {
public:
    virtual ~IThemeRenderer() = default;
    
    /**
     * @brief 应用颜色调色板
     * @param palette 颜色调色板
     */
    virtual void applyColorPalette(const ColorPalette& palette) = 0;
    
    /**
     * @brief 应用字体设置
     * @param font 字体设置
     */
    virtual void applyFontSettings(const FontSettings& font) = 0;
    
    /**
     * @brief 应用图标主题
     * @param icon_theme 图标主题
     */
    virtual void applyIconTheme(const IconTheme& icon_theme) = 0;
    
    /**
     * @brief 应用窗口装饰
     * @param decoration 窗口装饰设置
     */
    virtual void applyWindowDecoration(const WindowDecoration& decoration) = 0;
    
    /**
     * @brief 应用动画设置
     * @param animation 动画设置
     */
    virtual void applyAnimationSettings(const AnimationSettings& animation) = 0;
    
    /**
     * @brief 获取当前主题的预览图像
     * @param width 预览宽度
     * @param height 预览高度
     * @return 预览图像数据
     */
    virtual std::vector<uint8_t> getThemePreview(int width, int height) = 0;
    
    /**
     * @brief 验证主题设置的兼容性
     * @param settings 主题设置
     * @return 兼容性错误信息，空字符串表示兼容
     */
    virtual std::string validateTheme(const ThemeSettings& settings) = 0;
};

/**
 * @class ThemeManager
 * @brief 主题管理器
 * 
 * 负责管理桌面环境的视觉主题，包括主题的加载、应用、切换和自定义
 */
class ThemeManager {
public:
    /**
     * @brief 构造函数
     */
    ThemeManager();
    
    /**
     * @brief 析构函数
     */
    ~ThemeManager();
    
    /**
     * @brief 初始化主题管理器
     * @param renderer 主题渲染器
     * @return 初始化是否成功
     */
    bool initialize(std::shared_ptr<IThemeRenderer> renderer);
    
    /**
     * @brief 加载系统默认主题
     * @return 加载是否成功
     */
    bool loadDefaultThemes();
    
    /**
     * @brief 应用主题
     * @param theme_name 主题名称
     * @return 应用是否成功
     */
    bool applyTheme(const std::string& theme_name);
    
    /**
     * @brief 获取当前主题设置
     * @return 当前主题设置
     */
    ThemeSettings getCurrentTheme() const;
    
    /**
     * @brief 获取所有可用主题
     * @return 主题名称列表
     */
    std::vector<std::string> getAvailableThemes() const;
    
    /**
     * @brief 获取主题设置
     * @param theme_name 主题名称
     * @return 主题设置，如果不存在则返回默认设置
     */
    ThemeSettings getTheme(const std::string& theme_name) const;
    
    /**
     * @brief 创建自定义主题
     * @param settings 主题设置
     * @return 创建是否成功
     */
    bool createCustomTheme(const ThemeSettings& settings);
    
    /**
     * @brief 修改现有主题
     * @param theme_name 主题名称
     * @param new_settings 新设置
     * @return 修改是否成功
     */
    bool modifyTheme(const std::string& theme_name, const ThemeSettings& new_settings);
    
    /**
     * @brief 删除主题
     * @param theme_name 主题名称
     * @return 删除是否成功
     */
    bool deleteTheme(const std::string& theme_name);
    
    /**
     * @brief 导出主题到文件
     * @param theme_name 主题名称
     * @param file_path 文件路径
     * @return 导出是否成功
     */
    bool exportTheme(const std::string& theme_name, const std::string& file_path);
    
    /**
     * @brief 从文件导入主题
     * @param file_path 文件路径
     * @return 导入是否成功
     */
    bool importTheme(const std::string& file_path);
    
    /**
     * @brief 切换主题类型（浅色/深色）
     * @param type 主题类型
     * @return 切换是否成功
     */
    bool switchThemeType(ThemeType type);
    
    /**
     * @brief 切换颜色方案
     * @param scheme 颜色方案
     * @return 切换是否成功
     */
    bool switchColorScheme(ColorScheme scheme);
    
    /**
     * @brief 设置字体
     * @param font 字体设置
     * @return 设置是否成功
     */
    bool setFont(const FontSettings& font);
    
    /**
     * @brief 设置图标主题
     * @param icon_theme 图标主题
     * @return 设置是否成功
     */
    bool setIconTheme(const IconTheme& icon_theme);
    
    /**
     * @brief 设置窗口装饰
     * @param decoration 窗口装饰设置
     * @return 设置是否成功
     */
    bool setWindowDecoration(const WindowDecoration& decoration);
    
    /**
     * @brief 设置动画效果
     * @param animation 动画设置
     * @return 设置是否成功
     */
    bool setAnimation(const AnimationSettings& animation);
    
    /**
     * @brief 生成主题预览
     * @param theme_name 主题名称
     * @param width 预览宽度
     * @param height 预览高度
     * @return 预览图像数据
     */
    std::vector<uint8_t> generateThemePreview(const std::string& theme_name, int width, int height);
    
    /**
     * @brief 验证主题设置
     * @param settings 主题设置
     * @return 验证错误信息，空字符串表示有效
     */
    std::string validateThemeSettings(const ThemeSettings& settings);
    
    /**
     * @brief 添加事件监听器
     * @param callback 回调函数
     */
    void addEventListener(std::function<void(const ThemeEvent&)> callback);
    
    /**
     * @brief 保存当前主题配置
     * @param config_path 配置文件路径
     * @return 保存是否成功
     */
    bool saveConfig(const std::string& config_path);
    
    /**
     * @brief 加载主题配置
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
     * @brief 获取主题统计信息
     * @return 统计信息字符串
     */
    std::string getStatistics() const;
    
    /**
     * @brief 重置为默认主题
     * @return 重置是否成功
     */
    bool resetToDefault();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Desktop
} // namespace CloudFlow

#endif // CLOUDFLOW_THEME_H