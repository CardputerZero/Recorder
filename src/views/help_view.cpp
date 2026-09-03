#include "views/help_view.hpp"

#include "assets/assets.h"

#include <cstdint>

namespace recorder {
namespace {

constexpr int32_t kPanelWidth  = 286;
constexpr int32_t kPanelHeight = 122;

void configureLabel(lv_obj_t* label, const lv_font_t* font, lv_color_t color)
{
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    if (font) {
        lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    }
}

}  // namespace

HelpView::HelpView(lv_obj_t* parent)
{
    if (!parent) {
        return;
    }

    _overlay = lv_obj_create(parent);
    if (!_overlay) {
        return;
    }

    lv_obj_set_size(_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_pos(_overlay, 0, 0);
    lv_obj_set_style_bg_color(_overlay, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_overlay, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(_overlay, 0, LV_PART_MAIN);
    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* panel = lv_obj_create(_overlay);
    if (!panel) {
        lv_obj_delete(_overlay);
        _overlay = nullptr;
        return;
    }
    lv_obj_set_size(panel, kPanelWidth, kPanelHeight);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x101010), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x363636), LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(panel, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_right(panel, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_top(panel, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(panel, 8, LV_PART_MAIN);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(panel);
    lv_obj_set_width(title, kPanelWidth - 28);
    configureLabel(title, &font_chivo_medium_14, lv_color_hex(0xFFFFFF));
    lv_label_set_text(title, "Recorder");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t* body = lv_label_create(panel);
    lv_obj_set_width(body, kPanelWidth - 28);
    lv_obj_set_height(body, 66);
    configureLabel(body, &font_chivo_medium_14, lv_color_hex(0xF2F2F2));
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_label_set_text(body,
                      "Record, play, and manage audio files.\n\n"
                      "Number keys 4-8: operations");
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 0, 23);

    lv_obj_t* footer = lv_label_create(panel);
    lv_obj_set_width(footer, kPanelWidth - 28);
    configureLabel(footer, &font_chivo_mono_medium_12, lv_color_hex(0x9A9A9A));
    lv_obj_set_style_text_align(footer, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text(footer, "Fn+H / Esc: close");
    lv_obj_align(footer, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
}

HelpView::~HelpView()
{
    if (_overlay && lv_obj_is_valid(_overlay)) {
        lv_obj_delete(_overlay);
    }
    _overlay = nullptr;
}

void HelpView::show()
{
    if (!_overlay || !lv_obj_is_valid(_overlay)) {
        return;
    }
    lv_obj_remove_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(_overlay);
}

void HelpView::hide()
{
    if (_overlay && lv_obj_is_valid(_overlay)) {
        lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

void HelpView::toggle()
{
    if (visible()) {
        hide();
    } else {
        show();
    }
}

bool HelpView::visible() const
{
    return _overlay && lv_obj_is_valid(_overlay) && !lv_obj_has_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
}

}  // namespace recorder
