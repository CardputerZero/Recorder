#include "views/volume_hud.hpp"

#include <core/easing/ease.hpp>
#include <algorithm>
#include <cmath>

namespace recorder {

namespace {

constexpr int32_t kWidth              = 134;
constexpr int32_t kHeight             = 86;
constexpr int32_t kRadius             = 14;
constexpr int32_t kPanelOffsetY       = -10;
constexpr int32_t kIconY              = 22;
constexpr int32_t kSegmentStartX      = 18;
constexpr int32_t kSegmentY           = 61;
constexpr int32_t kSegmentWidth       = 3;
constexpr int32_t kSegmentHeight      = 7;
constexpr int32_t kSegmentPitch       = 5;
constexpr uint32_t kVisibleDurationMs = 1100;
constexpr float kFadeDuration         = 0.18f;
constexpr uint32_t kPanelColor        = 0x474747;
constexpr uint32_t kTextColor         = 0xFFFFFF;

lv_opa_t toOpacity(float value)
{
    return static_cast<lv_opa_t>(std::clamp(static_cast<int>(std::round(value)), 0, 255));
}

bool deadlineReached(uint32_t now_ms, uint32_t deadline_ms)
{
    return static_cast<int32_t>(now_ms - deadline_ms) >= 0;
}

}  // namespace

VolumeHud::VolumeHud() : _opacity(0)
{
    _opacity.easingOptions().duration       = kFadeDuration;
    _opacity.easingOptions().easingFunction = smooth_ui_toolkit::ease::ease_out_quad;
}

VolumeHud::~VolumeHud()
{
    shutdown();
}

void VolumeHud::start(lv_obj_t* parent)
{
    if (!_root) {
        (void)create(parent);
    }
}

void VolumeHud::shutdown()
{
    if (_root) {
        lv_obj_delete(_root);
    }
    _root = nullptr;
    _icon = nullptr;
    _segments.fill(nullptr);
    _opacity.teleport(0);
    _waiting_to_hide = false;
    _hide_at_ms      = 0;
}

void VolumeHud::showVolume(int percent)
{
    const int clamped = std::clamp(percent, 0, 100);
    show(clamped == 0 ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MAX, clamped);
}

void VolumeHud::showMute(bool muted, int percent)
{
    show(muted ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MAX, muted ? 0 : std::clamp(percent, 0, 100));
}

void VolumeHud::tick(uint32_t now_ms)
{
    if (!_root || lv_obj_has_flag(_root, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    if (_waiting_to_hide && deadlineReached(now_ms, _hide_at_ms)) {
        _waiting_to_hide = false;
        _opacity.move(0);
    }

    _opacity.update();
    applyOpacity();

    if (!_waiting_to_hide && _opacity.done() && _opacity.directValue() <= 0.0f) {
        lv_obj_add_flag(_root, LV_OBJ_FLAG_HIDDEN);
    }
}

bool VolumeHud::create(lv_obj_t* parent)
{
    if (!parent) {
        return false;
    }

    _root = lv_obj_create(parent);
    if (!_root) {
        return false;
    }
    lv_obj_remove_style_all(_root);
    lv_obj_add_event_cb(_root, onRootDeleted, LV_EVENT_DELETE, this);
    lv_obj_set_size(_root, kWidth, kHeight);
    lv_obj_align(_root, LV_ALIGN_CENTER, 0, kPanelOffsetY);
    lv_obj_set_style_bg_color(_root, lv_color_hex(kPanelColor), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_root, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(_root, kRadius, LV_PART_MAIN);
    lv_obj_set_style_border_width(_root, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(_root, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(_root, 0, LV_PART_MAIN);
    lv_obj_clear_flag(_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_root, LV_OBJ_FLAG_IGNORE_LAYOUT);

    _icon = lv_label_create(_root);
    if (!_icon) {
        shutdown();
        return false;
    }

    lv_obj_set_size(_icon, kWidth, 26);
    lv_obj_set_pos(_icon, 0, kIconY);
    lv_obj_set_style_text_color(_icon, lv_color_hex(kTextColor), LV_PART_MAIN);
    lv_obj_set_style_text_font(_icon, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_align(_icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    for (size_t i = 0; i < _segments.size(); ++i) {
        _segments[i] = lv_obj_create(_root);
        if (!_segments[i]) {
            shutdown();
            return false;
        }
        lv_obj_remove_style_all(_segments[i]);
        lv_obj_set_size(_segments[i], kSegmentWidth, kSegmentHeight);
        lv_obj_set_pos(_segments[i], kSegmentStartX + static_cast<int32_t>(i) * kSegmentPitch, kSegmentY);
        lv_obj_set_style_bg_color(_segments[i], lv_color_hex(kTextColor), LV_PART_MAIN);
        lv_obj_set_style_radius(_segments[i], 1, LV_PART_MAIN);
        lv_obj_clear_flag(_segments[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(_segments[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    lv_obj_set_style_opa_layered(_root, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(_root, LV_OBJ_FLAG_HIDDEN);
    return true;
}

void VolumeHud::show(const char* icon, int percent)
{
    if (!_root && !create(lv_layer_top())) {
        return;
    }

    lv_label_set_text(_icon, icon);
    renderSegments(percent);

    lv_obj_remove_flag(_root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(_root);
    _opacity.move(255);
    _hide_at_ms      = lv_tick_get() + kVisibleDurationMs;
    _waiting_to_hide = true;
    applyOpacity();
}

void VolumeHud::renderSegments(int percent)
{
    const int clamped = std::clamp(percent, 0, 100);
    const size_t active =
        clamped == 0 ? 0 : static_cast<size_t>((clamped * static_cast<int>(kSegmentCount) + 99) / 100);

    for (size_t i = 0; i < _segments.size(); ++i) {
        lv_obj_set_style_bg_opa(_segments[i], i < active ? LV_OPA_COVER : LV_OPA_20, LV_PART_MAIN);
    }
}

void VolumeHud::applyOpacity()
{
    if (_root) {
        lv_obj_set_style_opa_layered(_root, toOpacity(_opacity.directValue()), LV_PART_MAIN);
    }
}

void VolumeHud::onRootDeleted(lv_event_t* event)
{
    auto* self = static_cast<VolumeHud*>(lv_event_get_user_data(event));
    if (!self || lv_event_get_target(event) != self->_root) {
        return;
    }

    self->_root = nullptr;
    self->_icon = nullptr;
    self->_segments.fill(nullptr);
    self->_waiting_to_hide = false;
    self->_hide_at_ms      = 0;
}

}  // namespace recorder
