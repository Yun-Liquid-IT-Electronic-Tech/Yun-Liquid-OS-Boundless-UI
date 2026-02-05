/**
 * @file theme.cpp
 * @brief 主题系统管理器实现文件
 */

#include "theme.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

namespace CloudFlow {
namespace Desktop {

// 实现类
class ThemeManager::Impl {
public:
    Impl() : current_theme_("默认主题"), theme_apply_count_(0), last_apply_time_(std::chrono::system_clock::now()) {
        loadDefaultThemes();
    }
    
    ~Impl() {
        // 清理资源
        event_listeners_.clear();
        themes_.clear();
    }
    
    bool initialize(std::shared_ptr<IThemeRenderer> renderer) {
        if (!renderer) {
            last_error_ = "渲染器不能为空";
            return false;
        }
        
        renderer_ = renderer;
        return true;
    }
    
    bool loadDefaultThemes() {
        // 创建默认浅色主题
        ThemeSettings light_theme;
        light_theme.name = "浅色主题";
        light_theme.type = ThemeType::Light;
        light_theme.scheme = ColorScheme::Default;
        light_theme.description = "默认浅色桌面主题";
        themes_["浅色主题"] = light_theme;
        
        // 创建深色主题
        ThemeSettings dark_theme;
        dark_theme.name = "深色主题";
        dark_theme.type = ThemeType::Dark;
        dark_theme.scheme = ColorScheme::Blue;
        dark_theme.palette.background = RGBColor(33, 33, 33);
        dark_theme.palette.surface = RGBColor(48, 48, 48);
        dark_theme.palette.text_primary = RGBColor(255, 255, 255);
        dark_theme.palette.text_secondary = RGBColor(189, 189, 189);
        dark_theme.description = "深色桌面主题，适合夜间使用";
        themes_["深色主题"] = dark_theme;
        
        // 创建高对比度主题
        ThemeSettings high_contrast_theme;
        high_contrast_theme.name = "高对比度主题";
        high_contrast_theme.type = ThemeType::HighContrast;
        high_contrast_theme.scheme = ColorScheme::Red;
        high_contrast_theme.palette.background = RGBColor(0, 0, 0);
        high_contrast_theme.palette.surface = RGBColor(51, 51, 51);
        high_contrast_theme.palette.text_primary = RGBColor(255, 255, 0);
        high_contrast_theme.palette.text_secondary = RGBColor(255, 255, 0);
        high_contrast_theme.description = "高对比度主题，提高可读性";
        themes_["高对比度主题"] = high_contrast_theme;
        
        // 创建蓝色主题
        ThemeSettings blue_theme;
        blue_theme.name = "蓝色主题";
        blue_theme.type = ThemeType::Light;
        blue_theme.scheme = ColorScheme::Blue;
        blue_theme.palette.primary = RGBColor(25, 118, 210);
        blue_theme.palette.secondary = RGBColor(156, 39, 176);
        blue_theme.description = "蓝色配色桌面主题";
        themes_["蓝色主题"] = blue_theme;
        
        // 创建绿色主题
        ThemeSettings green_theme;
        green_theme.name = "绿色主题";
        green_theme.type = ThemeType::Light;
        green_theme.scheme = ColorScheme::Green;
        green_theme.palette.primary = RGBColor(56, 142, 60);
        green_theme.palette.secondary = RGBColor(255, 193, 7);
        green_theme.description = "绿色配色桌面主题";
        themes_["绿色主题"] = green_theme;
        
        return true;
    }
    
    bool applyTheme(const std::string& theme_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(theme_name) == themes_.end()) {
            last_error_ = "主题不存在: " + theme_name;
            return false;
        }
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return false;
        }
        
        const ThemeSettings& theme = themes_[theme_name];
        
        // 验证主题兼容性
        std::string validation_error = renderer_->validateTheme(theme);
        if (!validation_error.empty()) {
            last_error_ = "主题验证失败: " + validation_error;
            return false;
        }
        
        // 应用主题设置
        renderer_->applyColorPalette(theme.palette);
        renderer_->applyFontSettings(theme.font);
        renderer_->applyIconTheme(theme.icons);
        renderer_->applyWindowDecoration(theme.window);
        renderer_->applyAnimationSettings(theme.animation);
        
        // 更新当前主题
        current_theme_ = theme_name;
        theme_apply_count_++;
        last_apply_time_ = std::chrono::system_clock::now();
        
        // 触发主题改变事件
        ThemeEvent event(ThemeEvent::Type::ThemeChanged);
        event.theme_name = theme_name;
        notifyEventListeners(event);
        
        return true;
    }
    
    ThemeSettings getCurrentTheme() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(current_theme_) != themes_.end()) {
            return themes_.at(current_theme_);
        }
        
        // 返回默认主题
        return ThemeSettings();
    }
    
    std::vector<std::string> getAvailableThemes() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<std::string> theme_names;
        for (const auto& pair : themes_) {
            theme_names.push_back(pair.first);
        }
        
        return theme_names;
    }
    
    ThemeSettings getTheme(const std::string& theme_name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(theme_name) != themes_.end()) {
            return themes_.at(theme_name);
        }
        
        // 返回默认主题
        return ThemeSettings();
    }
    
    bool createCustomTheme(const ThemeSettings& settings) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (settings.name.empty()) {
            last_error_ = "主题名称不能为空";
            return false;
        }
        
        if (themes_.find(settings.name) != themes_.end()) {
            last_error_ = "主题已存在: " + settings.name;
            return false;
        }
        
        // 验证主题设置
        std::string validation_error = validateThemeSettings(settings);
        if (!validation_error.empty()) {
            last_error_ = "主题设置无效: " + validation_error;
            return false;
        }
        
        themes_[settings.name] = settings;
        return true;
    }
    
    bool modifyTheme(const std::string& theme_name, const ThemeSettings& new_settings) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(theme_name) == themes_.end()) {
            last_error_ = "主题不存在: " + theme_name;
            return false;
        }
        
        if (new_settings.name != theme_name) {
            last_error_ = "不能修改主题名称";
            return false;
        }
        
        // 验证主题设置
        std::string validation_error = validateThemeSettings(new_settings);
        if (!validation_error.empty()) {
            last_error_ = "主题设置无效: " + validation_error;
            return false;
        }
        
        themes_[theme_name] = new_settings;
        
        // 如果当前正在使用该主题，重新应用
        if (current_theme_ == theme_name) {
            applyTheme(theme_name);
        }
        
        return true;
    }
    
    bool deleteTheme(const std::string& theme_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(theme_name) == themes_.end()) {
            last_error_ = "主题不存在: " + theme_name;
            return false;
        }
        
        // 不能删除当前正在使用的主题
        if (current_theme_ == theme_name) {
            last_error_ = "不能删除当前正在使用的主题";
            return false;
        }
        
        // 不能删除系统默认主题
        if (theme_name == "浅色主题" || theme_name == "深色主题" || theme_name == "高对比度主题") {
            last_error_ = "不能删除系统默认主题";
            return false;
        }
        
        themes_.erase(theme_name);
        return true;
    }
    
    bool exportTheme(const std::string& theme_name, const std::string& file_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (themes_.find(theme_name) == themes_.end()) {
            last_error_ = "主题不存在: " + theme_name;
            return false;
        }
        
        const ThemeSettings& theme = themes_[theme_name];
        
        Json::Value root;
        root["name"] = theme.name;
        root["version"] = theme.version;
        root["author"] = theme.author;
        root["description"] = theme.description;
        root["type"] = static_cast<int>(theme.type);
        root["scheme"] = static_cast<int>(theme.scheme);
        
        // 颜色调色板
        Json::Value palette;
        palette["primary"] = theme.palette.primary.toHex();
        palette["secondary"] = theme.palette.secondary.toHex();
        palette["accent"] = theme.palette.accent.toHex();
        palette["background"] = theme.palette.background.toHex();
        palette["surface"] = theme.palette.surface.toHex();
        palette["text_primary"] = theme.palette.text_primary.toHex();
        palette["text_secondary"] = theme.palette.text_secondary.toHex();
        palette["error"] = theme.palette.error.toHex();
        palette["warning"] = theme.palette.warning.toHex();
        palette["success"] = theme.palette.success.toHex();
        palette["info"] = theme.palette.info.toHex();
        root["palette"] = palette;
        
        // 字体设置
        Json::Value font;
        font["family"] = theme.font.family;
        font["size"] = theme.font.size;
        font["bold"] = theme.font.bold;
        font["italic"] = theme.font.italic;
        font["weight"] = theme.font.weight;
        root["font"] = font;
        
        // 图标主题
        Json::Value icons;
        icons["name"] = theme.icons.name;
        icons["path"] = theme.icons.path;
        icons["size_small"] = theme.icons.size_small;
        icons["size_medium"] = theme.icons.size_medium;
        icons["size_large"] = theme.icons.size_large;
        icons["symbolic"] = theme.icons.symbolic;
        root["icons"] = icons;
        
        // 窗口装饰
        Json::Value window;
        window["border_color"] = theme.window.border_color.toHex();
        window["border_width"] = theme.window.border_width;
        window["title_bar_color"] = theme.window.title_bar_color.toHex();
        window["title_bar_height"] = theme.window.title_bar_height;
        window["rounded_corners"] = theme.window.rounded_corners;
        window["corner_radius"] = theme.window.corner_radius;
        window["shadows"] = theme.window.shadows;
        window["shadow_color"] = theme.window.shadow_color.toHex();
        window["shadow_blur"] = theme.window.shadow_blur;
        root["window"] = window;
        
        // 动画设置
        Json::Value animation;
        animation["style"] = static_cast<int>(theme.animation.style);
        animation["duration"] = theme.animation.duration;
        animation["enable_transitions"] = theme.animation.enable_transitions;
        animation["enable_effects"] = theme.animation.enable_effects;
        animation["easing_factor"] = theme.animation.easing_factor;
        root["animation"] = animation;
        
        // 写入文件
        std::ofstream file(file_path);
        if (!file.is_open()) {
            last_error_ = "无法打开文件: " + file_path;
            return false;
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        
        file.close();
        return true;
    }
    
    bool importTheme(const std::string& file_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            last_error_ = "无法打开文件: " + file_path;
            return false;
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errors;
        
        if (!Json::parseFromStream(builder, file, &root, &errors)) {
            last_error_ = "JSON解析错误: " + errors;
            return false;
        }
        
        ThemeSettings theme;
        
        // 解析基本属性
        if (root.isMember("name")) theme.name = root["name"].asString();
        if (root.isMember("version")) theme.version = root["version"].asString();
        if (root.isMember("author")) theme.author = root["author"].asString();
        if (root.isMember("description")) theme.description = root["description"].asString();
        if (root.isMember("type")) theme.type = static_cast<ThemeType>(root["type"].asInt());
        if (root.isMember("scheme")) theme.scheme = static_cast<ColorScheme>(root["scheme"].asInt());
        
        // 解析颜色调色板
        if (root.isMember("palette")) {
            Json::Value palette = root["palette"];
            if (palette.isMember("primary")) theme.palette.primary = RGBColor::fromHex(palette["primary"].asString());
            if (palette.isMember("secondary")) theme.palette.secondary = RGBColor::fromHex(palette["secondary"].asString());
            if (palette.isMember("accent")) theme.palette.accent = RGBColor::fromHex(palette["accent"].asString());
            if (palette.isMember("background")) theme.palette.background = RGBColor::fromHex(palette["background"].asString());
            if (palette.isMember("surface")) theme.palette.surface = RGBColor::fromHex(palette["surface"].asString());
            if (palette.isMember("text_primary")) theme.palette.text_primary = RGBColor::fromHex(palette["text_primary"].asString());
            if (palette.isMember("text_secondary")) theme.palette.text_secondary = RGBColor::fromHex(palette["text_secondary"].asString());
            if (palette.isMember("error")) theme.palette.error = RGBColor::fromHex(palette["error"].asString());
            if (palette.isMember("warning")) theme.palette.warning = RGBColor::fromHex(palette["warning"].asString());
            if (palette.isMember("success")) theme.palette.success = RGBColor::fromHex(palette["success"].asString());
            if (palette.isMember("info")) theme.palette.info = RGBColor::fromHex(palette["info"].asString());
        }
        
        // 解析字体设置
        if (root.isMember("font")) {
            Json::Value font = root["font"];
            if (font.isMember("family")) theme.font.family = font["family"].asString();
            if (font.isMember("size")) theme.font.size = font["size"].asInt();
            if (font.isMember("bold")) theme.font.bold = font["bold"].asBool();
            if (font.isMember("italic")) theme.font.italic = font["italic"].asBool();
            if (font.isMember("weight")) theme.font.weight = font["weight"].asInt();
        }
        
        // 解析图标主题
        if (root.isMember("icons")) {
            Json::Value icons = root["icons"];
            if (icons.isMember("name")) theme.icons.name = icons["name"].asString();
            if (icons.isMember("path")) theme.icons.path = icons["path"].asString();
            if (icons.isMember("size_small")) theme.icons.size_small = icons["size_small"].asInt();
            if (icons.isMember("size_medium")) theme.icons.size_medium = icons["size_medium"].asInt();
            if (icons.isMember("size_large")) theme.icons.size_large = icons["size_large"].asInt();
            if (icons.isMember("symbolic")) theme.icons.symbolic = icons["symbolic"].asBool();
        }
        
        // 解析窗口装饰
        if (root.isMember("window")) {
            Json::Value window = root["window"];
            if (window.isMember("border_color")) theme.window.border_color = RGBColor::fromHex(window["border_color"].asString());
            if (window.isMember("border_width")) theme.window.border_width = window["border_width"].asInt();
            if (window.isMember("title_bar_color")) theme.window.title_bar_color = RGBColor::fromHex(window["title_bar_color"].asString());
            if (window.isMember("title_bar_height")) theme.window.title_bar_height = window["title_bar_height"].asInt();
            if (window.isMember("rounded_corners")) theme.window.rounded_corners = window["rounded_corners"].asBool();
            if (window.isMember("corner_radius")) theme.window.corner_radius = window["corner_radius"].asInt();
            if (window.isMember("shadows")) theme.window.shadows = window["shadows"].asBool();
            if (window.isMember("shadow_color")) theme.window.shadow_color = RGBColor::fromHex(window["shadow_color"].asString());
            if (window.isMember("shadow_blur")) theme.window.shadow_blur = window["shadow_blur"].asInt();
        }
        
        // 解析动画设置
        if (root.isMember("animation")) {
            Json::Value animation = root["animation"];
            if (animation.isMember("style")) theme.animation.style = static_cast<AnimationStyle>(animation["style"].asInt());
            if (animation.isMember("duration")) theme.animation.duration = animation["duration"].asInt();
            if (animation.isMember("enable_transitions")) theme.animation.enable_transitions = animation["enable_transitions"].asBool();
            if (animation.isMember("enable_effects")) theme.animation.enable_effects = animation["enable_effects"].asBool();
            if (animation.isMember("easing_factor")) theme.animation.easing_factor = animation["easing_factor"].asFloat();
        }
        
        // 验证主题设置
        std::string validation_error = validateThemeSettings(theme);
        if (!validation_error.empty()) {
            last_error_ = "导入的主题设置无效: " + validation_error;
            return false;
        }
        
        // 添加到主题列表
        themes_[theme.name] = theme;
        return true;
    }
    
    bool switchThemeType(ThemeType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        ThemeSettings current_theme = getCurrentTheme();
        current_theme.type = type;
        
        // 根据主题类型调整颜色调色板
        if (type == ThemeType::Dark) {
            current_theme.palette.background = RGBColor(33, 33, 33);
            current_theme.palette.surface = RGBColor(48, 48, 48);
            current_theme.palette.text_primary = RGBColor(255, 255, 255);
            current_theme.palette.text_secondary = RGBColor(189, 189, 189);
        } else if (type == ThemeType::Light) {
            current_theme.palette.background = RGBColor(255, 255, 255);
            current_theme.palette.surface = RGBColor(245, 245, 245);
            current_theme.palette.text_primary = RGBColor(33, 33, 33);
            current_theme.palette.text_secondary = RGBColor(117, 117, 117);
        } else if (type == ThemeType::HighContrast) {
            current_theme.palette.background = RGBColor(0, 0, 0);
            current_theme.palette.surface = RGBColor(51, 51, 51);
            current_theme.palette.text_primary = RGBColor(255, 255, 0);
            current_theme.palette.text_secondary = RGBColor(255, 255, 0);
        }
        
        return applyTheme(current_theme.name);
    }
    
    bool switchColorScheme(ColorScheme scheme) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        ThemeSettings current_theme = getCurrentTheme();
        current_theme.scheme = scheme;
        
        // 根据颜色方案调整主色调
        switch (scheme) {
            case ColorScheme::Blue:
                current_theme.palette.primary = RGBColor(25, 118, 210);
                break;
            case ColorScheme::Green:
                current_theme.palette.primary = RGBColor(56, 142, 60);
                break;
            case ColorScheme::Purple:
                current_theme.palette.primary = RGBColor(123, 31, 162);
                break;
            case ColorScheme::Orange:
                current_theme.palette.primary = RGBColor(245, 124, 0);
                break;
            case ColorScheme::Red:
                current_theme.palette.primary = RGBColor(211, 47, 47);
                break;
            default:
                current_theme.palette.primary = RGBColor(33, 150, 243);
                break;
        }
        
        return applyTheme(current_theme.name);
    }
    
    bool setFont(const FontSettings& font) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return false;
        }
        
        renderer_->applyFontSettings(font);
        
        // 更新当前主题的字体设置
        if (themes_.find(current_theme_) != themes_.end()) {
            themes_[current_theme_].font = font;
        }
        
        // 触发字体改变事件
        ThemeEvent event(ThemeEvent::Type::FontChanged);
        event.theme_name = current_theme_;
        notifyEventListeners(event);
        
        return true;
    }
    
    bool setIconTheme(const IconTheme& icon_theme) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return false;
        }
        
        renderer_->applyIconTheme(icon_theme);
        
        // 更新当前主题的图标设置
        if (themes_.find(current_theme_) != themes_.end()) {
            themes_[current_theme_].icons = icon_theme;
        }
        
        // 触发图标主题改变事件
        ThemeEvent event(ThemeEvent::Type::IconThemeChanged);
        event.theme_name = current_theme_;
        notifyEventListeners(event);
        
        return true;
    }
    
    bool setWindowDecoration(const WindowDecoration& decoration) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return false;
        }
        
        renderer_->applyWindowDecoration(decoration);
        
        // 更新当前主题的窗口装饰设置
        if (themes_.find(current_theme_) != themes_.end()) {
            themes_[current_theme_].window = decoration;
        }
        
        // 触发窗口装饰改变事件
        ThemeEvent event(ThemeEvent::Type::WindowDecorationChanged);
        event.theme_name = current_theme_;
        notifyEventListeners(event);
        
        return true;
    }
    
    bool setAnimation(const AnimationSettings& animation) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return false;
        }
        
        renderer_->applyAnimationSettings(animation);
        
        // 更新当前主题的动画设置
        if (themes_.find(current_theme_) != themes_.end()) {
            themes_[current_theme_].animation = animation;
        }
        
        // 触发动画设置改变事件
        ThemeEvent event(ThemeEvent::Type::AnimationChanged);
        event.theme_name = current_theme_;
        notifyEventListeners(event);
        
        return true;
    }
    
    std::vector<uint8_t> generateThemePreview(const std::string& theme_name, int width, int height) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!renderer_) {
            last_error_ = "渲染器未初始化";
            return {};
        }
        
        if (themes_.find(theme_name) == themes_.end()) {
            last_error_ = "主题不存在: " + theme_name;
            return {};
        }
        
        return renderer_->getThemePreview(width, height);
    }
    
    std::string validateThemeSettings(const ThemeSettings& settings) {
        // 验证主题名称
        if (settings.name.empty()) {
            return "主题名称不能为空";
        }
        
        // 验证字体大小
        if (settings.font.size < 8 || settings.font.size > 72) {
            return "字体大小必须在8-72之间";
        }
        
        // 验证字体权重
        if (settings.font.weight < 100 || settings.font.weight > 900) {
            return "字体权重必须在100-900之间";
        }
        
        // 验证图标尺寸
        if (settings.icons.size_small < 8 || settings.icons.size_small > 64) {
            return "小图标尺寸必须在8-64之间";
        }
        if (settings.icons.size_medium < 16 || settings.icons.size_medium > 128) {
            return "中图标尺寸必须在16-128之间";
        }
        if (settings.icons.size_large < 32 || settings.icons.size_large > 256) {
            return "大图标尺寸必须在32-256之间";
        }
        
        // 验证窗口装饰
        if (settings.window.border_width < 0 || settings.window.border_width > 10) {
            return "边框宽度必须在0-10之间";
        }
        if (settings.window.title_bar_height < 20 || settings.window.title_bar_height > 60) {
            return "标题栏高度必须在20-60之间";
        }
        if (settings.window.corner_radius < 0 || settings.window.corner_radius > 20) {
            return "圆角半径必须在0-20之间";
        }
        if (settings.window.shadow_blur < 0 || settings.window.shadow_blur > 50) {
            return "阴影模糊半径必须在0-50之间";
        }
        
        // 验证动画设置
        if (settings.animation.duration < 0 || settings.animation.duration > 2000) {
            return "动画时长必须在0-2000毫秒之间";
        }
        if (