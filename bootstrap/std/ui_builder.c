#include <std/ui_builder.h>

UI_Signal ui_button(String string, UI_Rect rect)
{
    auto* widget = ui_widget_make((UI_WidgetFlags) {
        .draw_text = 1,
        .draw_background = 1,
        .clickable = 1,
    }, string, rect);

    UI_Signal signal = ui_signal_from_widget(widget);
    return signal;
}
