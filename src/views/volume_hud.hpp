#pragma once

#include <core/animation/animate_value/animate_value.hpp>
#include <lvgl.h>
#include <array>
#include <cstdint>

namespace recorder {

class VolumeHud {
public:
    VolumeHud();
    ~VolumeHud();

    VolumeHud(const VolumeHud&)            = delete;
    VolumeHud& operator=(const VolumeHud&) = delete;

    void start(lv_obj_t* parent);
    void shutdown();
    void showVolume(int percent);
    void showMute(bool muted, int percent);
    void tick(uint32_t now_ms);

private:
    static constexpr size_t kSegmentCount = 20;

    lv_obj_t* _root = nullptr;
    lv_obj_t* _icon = nullptr;
    std::array<lv_obj_t*, kSegmentCount> _segments{};
    smooth_ui_toolkit::AnimateValue _opacity;
    uint32_t _hide_at_ms  = 0;
    bool _waiting_to_hide = false;

    bool create(lv_obj_t* parent);
    void show(const char* icon, int percent);
    void renderSegments(int percent);
    void applyOpacity();
    static void onRootDeleted(lv_event_t* event);
};

}  // namespace recorder
