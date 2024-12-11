#include <std/ui_builder.h>
#include <std/render.h>

UI_Signal ui_button(String string, UI_Rect rect)
{
    auto rect_offset = renderer_font_compute_string_rect(ui_state_get()->renderer, RENDER_FONT_TYPE_PROPORTIONAL, string);
    rect.x1 = rect.x0 + rect_offset.x;
    rect.y1 = rect.y0 + rect_offset.y;
    auto* widget = ui_widget_make((UI_WidgetFlags) {
        .draw_text = 1,
        .draw_background = 1,
        .clickable = 1,
    }, string, rect);

    UI_Signal signal = ui_signal_from_widget(widget);
    return signal;
}
