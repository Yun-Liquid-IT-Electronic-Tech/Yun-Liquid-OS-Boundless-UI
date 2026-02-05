/**
 * @file desktop_icon.cpp
 * @brief 桌面图标管理器实现文件
 */

#include "desktop_icon.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>
#include <json/json.h>

namespace CloudFlow {
namespace Desktop {

class DesktopIconManager::Impl {
public:
    Impl() : arrangement_(IconArrangement::AutoArrange), 
              icon_size_(IconSize::Medium),
              grid_size_(64),
              is_dragging_(false),
              drag_start_pos_{0, 0},
              last_error_("") {}
    
    ~Impl() = default;
    
    bool initialize(std::shared_ptr<IDesktopIconRenderer> renderer) {
        if (!renderer) {
            last_error_ = "渲染器不能为空";
            return false;
        }
        
        renderer_ = renderer;
        
        // 创建默认系统图标
        createDefaultIcons();
        
        return true;
    }
    
    bool addIcon(const DesktopIcon& icon) {
        // 检查图标ID是否已存在
        if (getIconById(icon.id) != nullptr) {
            last_error_ = "图标ID已存在: " + icon.id;
            return false;
        }
        
        // 检查位置是否被占用
        if (getIconAtPosition(icon.position) != nullptr) {
            // 自动寻找空闲位置
            DesktopIcon new_icon = icon;
            if (!findFreePosition(new_icon.position)) {
                last_error_ = "无法找到空闲位置";
                return false;
            }
            icons_.push_back(new_icon);
        } else {
            icons_.push_back(icon);
        }
        
        // 如果启用自动排列，重新排列图标
        if (arrangement_ == IconArrangement::AutoArrange) {
            autoArrangeIcons();
        }
        
        // 触发刷新事件
        triggerRefreshEvent();
        
        return true;
    }
    
    bool removeIcon(const std::string& icon_id) {
        auto it = std::find_if(icons_.begin(), icons_.end(), 
                              [&icon_id](const DesktopIcon& icon) { 
                                  return icon.id == icon_id; 
                              });
        
        if (it == icons_.end()) {
            last_error_ = "图标不存在: " + icon_id;
            return false;
        }
        
        icons_.erase(it);
        
        // 触发刷新事件
        triggerRefreshEvent();
        
        return true;
    }
    
    bool moveIcon(const std::string& icon_id, const IconPosition& new_position) {
        DesktopIcon* icon = getIconById(icon_id);
        if (!icon) {
            last_error_ = "图标不存在: " + icon_id;
            return false;
        }
        
        // 检查新位置是否被占用
        DesktopIcon* existing_icon = getIconAtPosition(new_position);
        if (existing_icon && existing_icon->id != icon_id) {
            last_error_ = "目标位置已被占用";
            return false;
        }
        
        icon->position = new_position;
        icon->modified_time = std::chrono::system_clock::now();
        
        // 触发刷新事件
        triggerRefreshEvent();
        
        return true;
    }
    
    bool selectIcon(const std::string& icon_id, bool multi_select) {
        DesktopIcon* icon = getIconById(icon_id);
        if (!icon) {
            last_error_ = "图标不存在: " + icon_id;
            return false;
        }
        
        if (!multi_select) {
            clearSelection();
        }
        
        icon->selected = true;
        
        // 触发选择改变事件
        DesktopIconEvent event(DesktopIconEvent::Type::SelectionChanged);
        event.icon = icon;
        notifyEventListeners(event);
        
        return true;
    }
    
    void clearSelection() {
        for (auto& icon : icons_) {
            icon.selected = false;
        }
        
        // 触发选择改变事件
        DesktopIconEvent event(DesktopIconEvent::Type::SelectionChanged);
        notifyEventListeners(event);
    }
    
    std::vector<DesktopIcon> getAllIcons() const {
        return icons_;
    }
    
    std::vector<DesktopIcon> getSelectedIcons() const {
        std::vector<DesktopIcon> selected;
        std::copy_if(icons_.begin(), icons_.end(), std::back_inserter(selected),
                    [](const DesktopIcon& icon) { return icon.selected; });
        return selected;
    }
    
    DesktopIcon* getIconAtPosition(const IconPosition& position) {
        for (auto& icon : icons_) {
            if (!icon.visible) continue;
            
            int icon_size = renderer_->getIconSize(icon_size_);
            int icon_width = icon_size;
            int icon_height = icon_size + 20; // 包含标签高度
            
            if (position.pixel_x >= icon.position.pixel_x &&
                position.pixel_x <= icon.position.pixel_x + icon_width &&
                position.pixel_y >= icon.position.pixel_y &&
                position.pixel_y <= icon.position.pixel_y + icon_height) {
                return &icon;
            }
        }
        return nullptr;
    }
    
    DesktopIcon* getIconById(const std::string& icon_id) {
        auto it = std::find_if(icons_.begin(), icons_.end(),
                              [&icon_id](const DesktopIcon& icon) {
                                  return icon.id == icon_id;
                              });
        return it != icons_.end() ? &(*it) : nullptr;
    }
    
    void setArrangement(IconArrangement arrangement) {
        arrangement_ = arrangement;
        if (arrangement_ == IconArrangement::AutoArrange) {
            autoArrangeIcons();
        }
    }
    
    void setIconSize(IconSize size) {
        icon_size_ = size;
        switch (size) {
            case IconSize::Small: grid_size_ = 48; break;
            case IconSize::Medium: grid_size_ = 64; break;
            case IconSize::Large: grid_size_ = 80; break;
            case IconSize::ExtraLarge: grid_size_ = 96; break;
        }
        triggerRefreshEvent();
    }
    
    void autoArrangeIcons() {
        int x = 0, y = 0;
        int max_icons_per_row = 8; // 每行最多图标数
        
        for (auto& icon : icons_) {
            if (!icon.visible) continue;
            
            icon.position.grid_x = x;
            icon.position.grid_y = y;
            icon.position.pixel_x = x * grid_size_;
            icon.position.pixel_y = y * grid_size_;
            
            x++;
            if (x >= max_icons_per_row) {
                x = 0;
                y++;
            }
        }
        
        triggerRefreshEvent();
    }
    
    void refreshDesktop() {
        if (renderer_) {
            // 渲染所有图标
            for (const auto& icon : icons_) {
                if (icon.visible) {
                    renderer_->renderIcon(icon, icon_size_);
                    renderer_->renderIconLabel(icon);
                }
            }
            
            // 渲染选择框
            for (const auto& icon : icons_) {
                if (icon.selected && icon.visible) {
                    IconPosition size(grid_size_, grid_size_ + 20);
                    renderer_->renderSelectionBox(icon.position, size);
                }
            }
        }
    }
    
    void handleMouseEvent(const DesktopIconEvent& event) {
        switch (event.type) {
            case DesktopIconEvent::Type::Click:
                handleClickEvent(event);
                break;
            case DesktopIconEvent::Type::DoubleClick:
                handleDoubleClickEvent(event);
                break;
            case DesktopIconEvent::Type::RightClick:
                handleRightClickEvent(event);
                break;
            case DesktopIconEvent::Type::DragStart:
                handleDragStartEvent(event);
                break;
            case DesktopIconEvent::Type::DragMove:
                handleDragMoveEvent(event);
                break;
            case DesktopIconEvent::Type::DragEnd:
                handleDragEndEvent(event);
                break;
            default:
                break;
        }
    }
    
    void handleKeyboardEvent(int key_code, bool ctrl_pressed, bool shift_pressed) {
        // 处理键盘快捷键
        switch (key_code) {
            case 65: // A (Ctrl+A全选)
                if (ctrl_pressed) {
                    for (auto& icon : icons_) {
                        icon.selected = true;
