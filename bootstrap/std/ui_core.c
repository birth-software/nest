#include <std/ui_core.h>
#include <std/format.h>
#include <std/string.h>

global_variable UI_State* ui_state = 0;
#define ui_stack_autopop_set(field_name, value) ui_state->stack_autopops.field_name = (value)
#define ui_stack_push_impl(field_name, value, auto_pop_value) do \
{\
    *vb_add(&ui_state->stacks.field_name, 1) = (value);\
    ui_stack_autopop_set(field_name, auto_pop_value);\
} while (0)

fn u8* ui_pop_generic(VirtualBuffer(u8)* stack, u32 element_size)
{
    auto length = stack->length;

    assert(length > 0);
    auto next_length = length - 1;
    auto index = next_length;
    auto* result = &stack->pointer[index * element_size];
    stack->length = next_length;

    return result;
}

#define ui_stack_push(field_name, value) ui_stack_push_impl(field_name, value, 0)
#define ui_stack_push_next_only(field_name, value) ui_stack_push_impl(field_name, value, 1)
#define ui_stack_pop(field_name) (typeof(ui_state->stacks.field_name.pointer)) ui_pop_generic(&ui_state->stacks.field_name, sizeof(*ui_state->stacks.field_name.pointer))
#define ui_stack_top(field_name) (ui_state->stacks.field_name.length ? ui_state->stacks.field_name.pointer[ui_state->stacks.field_name.length - 1] : ui_state->stack_nulls.field_name)

void ui_pref_width(UI_Size size)
{
    ui_stack_push(pref_width, size);
}

void ui_pref_height(UI_Size size)
{
    ui_stack_push(pref_height, size);
}

fn void ui_autopop(UI_State* state)
{
    auto* restrict stack_end = (u32*)((u8*)&state->stacks + sizeof(state->stacks));
    
    auto* restrict bitset_pointer = (u64*)&state->stack_autopops;
    u64 bitset_index = 0;
    for (auto* restrict stack_pointer = (u32*)&state->stacks; stack_pointer != stack_end; stack_pointer += sizeof(VirtualBuffer(u8)) / sizeof(u32))
    {
        auto bitset = *bitset_pointer;
        auto shift_value = 1 << bitset_index;
        auto autopop = (bitset & shift_value) != 0;
        auto mask = ~shift_value;
        *bitset_pointer = bitset & mask;
        auto* restrict length_pointer = stack_pointer + (offsetof(VirtualBuffer(u8), length) / sizeof(u32));
        auto current_length = *length_pointer;
        assert(!autopop | current_length);
        *length_pointer -= autopop;

        u64 increment_bitset_element = (bitset_index > 0) & (bitset_index % 64 == 0);
        bitset_pointer += increment_bitset_element;
        bitset_index = increment_bitset_element ? 0 : bitset_index + 1;
    }
}

void ui_state_select(UI_State* state)
{
    ui_state = state;
}

UI_State* ui_state_get()
{
    return ui_state;
}

fn Arena* ui_build_arena()
{
    auto* arena = ui_state->build_arenas[ui_state->build_count % array_length(ui_state->build_arenas)];
    return arena;
}

fn UI_Key ui_key_null()
{
    UI_Key key = {};
    return key;
}

UI_State* ui_state_allocate(Renderer* renderer, RenderWindow* window)
{
    Arena* arena = arena_init(GB(8), MB(2), MB(2));
    UI_State* state = arena_allocate(arena, UI_State, 1);
    state->renderer = renderer;
    state->render_window = window;
    state->arena = arena;
    state->widget_table.length = 4096;
    state->widget_table.pointer = arena_allocate(arena, UI_WidgetSlot, state->widget_table.length);
    
    for (u64 i = 0; i < array_length(state->build_arenas); i += 1)
    {
        state->build_arenas[i] = arena_init(GB(8), MB(2), MB(2));
    }

    state->stack_nulls = (UI_StateStackNulls){
        .parent = 0,
        .child_layout_axis = AXIS2_COUNT,
        .pref_width = {},
        .pref_height = {},
    };

    auto* stack_end = (u32*)((u8*)&state->stacks + sizeof(state->stacks));
    
    for (auto* stack_pointer = (u32*)&state->stacks; stack_pointer != stack_end; stack_pointer += sizeof(VirtualBuffer(u8)) / sizeof(u32))
    {
        auto* length_pointer = stack_pointer + (offsetof(VirtualBuffer(u8), length) / sizeof(u32));
        assert(*length_pointer == 0);
    }

    return state;
}

fn u64 ui_widget_index_from_key(UI_Key key)
{
    auto length = ui_state->widget_table.length;
    assert(is_power_of_two(length));
    return key.value & (length - 1);
}

auto text_end_delimiter = strlit("##");
auto hash_start_delimiter = strlit("###");

fn String ui_text_from_key_string(String string)
{
    String result = string;
    auto index = string_first_ocurrence(string, text_end_delimiter);
    if (index < string.length)
    {
        result.length = index;
    }
    return result;
}

fn String ui_hash_from_key_string(String string)
{
    String result = string;
    auto index = string_first_ocurrence(string, hash_start_delimiter);
    if (index < string.length)
    {
        result = s_get_slice(u8, string, index, string.length);
    }

    return result;
}

fn UI_Key ui_key_from_string(UI_Key seed, String string)
{
    UI_Key key = ui_key_null();

    if (string.length)
    {
        key = seed;

        for (u64 i = 0; i < string.length; i += 1)
        {
            key.value = ((key.value << 5) + key.value) + string.pointer[i];
        }
    }

    return key;
}

fn UI_Key ui_key_from_string_format(UI_Key seed, char* format, ...)
{
    u8 buffer[256];
    va_list args;
    va_start(args, format);
    auto string = format_string_va((String)array_to_slice(buffer), format, args);
    va_end(args);
    auto result = ui_key_from_string(seed, string);
    return result;
}

fn u8 ui_key_equal(UI_Key a, UI_Key b)
{
    return a.value == b.value;
}

UI_Widget* ui_widget_from_key(UI_Key key)
{
    UI_Widget* result = 0;

    if (!ui_key_equal(key, ui_key_null()))
    {
        auto index = ui_widget_index_from_key(key);
        for (UI_Widget* widget = ui_state->widget_table.pointer[index].first; widget; widget = widget->hash_next)
        {
            if (ui_key_equal(widget->key, key))
            {
                result = widget;
                break;
            }
        }
    }

    return result;
}

UI_Widget* ui_widget_make_from_key(UI_WidgetFlags flags, UI_Key key)
{
    auto* widget = ui_widget_from_key(key);
    static auto count = 0;
    count += 1;

    if (widget)
    {
        if (widget->last_build_touched == ui_state->build_count)
        {
            key = ui_key_null();
            widget = 0;
        }
    }

    u8 first_frame = 0;
    if (!widget)
    {
        auto index = ui_widget_index_from_key(key);
        first_frame = 1;

        widget = arena_allocate(ui_state->arena, UI_Widget, 1);

        auto* table_widget_slot = &ui_state->widget_table.pointer[index];
        if (!table_widget_slot->last)
        {
            table_widget_slot->first = widget;
            table_widget_slot->last = widget;
        }
        else
        {
            table_widget_slot->last->next = widget;
            widget->previous = table_widget_slot->last;
            table_widget_slot->last = widget;
        }
    }

    auto* parent = ui_stack_top(parent);

    if (parent)
    {
        if (!parent->last)
        {
            parent->last = widget;
            parent->first = widget;
        }
        else
        {
            auto* previous_last = parent->last;
            previous_last->next = widget;
            parent->last = widget;
        }

        widget->parent = parent;
    }
    else
    {
        ui_state->root = widget;
    }

    auto color = count % 3 == 0;
    widget->key = key;
    widget->background_color = Color4(color, color, color, 1);
    widget->text_color = Color4(!color, !color, !color, 1);
    widget->flags = flags;
    widget->first = 0;
    widget->last = 0;
    widget->last_build_touched = ui_state->build_count;
    widget->pref_size[AXIS2_X] = ui_stack_top(pref_width);
    widget->pref_size[AXIS2_Y] = ui_stack_top(pref_height);
    widget->child_layout_axis = ui_stack_top(child_layout_axis);

    ui_autopop(ui_state);

    return widget;
}

UI_Widget* ui_widget_make(UI_WidgetFlags flags, String string)
{
    // TODO:
    auto seed = ui_key_null();

    auto hash_string = ui_hash_from_key_string(string);
    auto key = ui_key_from_string(seed, hash_string);

    auto* widget = ui_widget_make_from_key(flags, key);

    if (flags.draw_text)
    {
        widget->text = ui_text_from_key_string(string);
    }

    return widget;
}

UI_Widget* ui_widget_make_format(UI_WidgetFlags flags, const char* format, ...)
{
    va_list args;
    u8 buffer[4096];
    va_start(args, format);
    auto string = format_string_va((String)array_to_slice(buffer), format, args);
    va_end(args);

    auto* result = ui_widget_make(flags, string);
    return result;
}

UI_Signal ui_signal_from_widget(UI_Widget* widget)
{
    auto rect = widget->rect;
    auto mouse_position = ui_state->mouse_position;
    if (widget->flags.mouse_clickable & (ui_state->mouse_button_events[OS_EVENT_MOUSE_LEFT].action == OS_EVENT_MOUSE_RELEASE))
    {
        print("Clicked on {u32}x{u32}. Rect ({u32}, {u32}), ({u32}, {u32})\n", (u32)mouse_position.x, (u32)mouse_position.y, (u32)rect.p0.x, (u32)rect.p0.y, (u32)rect.p1.x, (u32)rect.p1.y);
    }
    UI_Signal signal = {
        .clicked_left = 
            (widget->flags.mouse_clickable & (ui_state->mouse_button_events[OS_EVENT_MOUSE_LEFT].action == OS_EVENT_MOUSE_RELEASE)) &
            ((mouse_position.x >= rect.x0) & (mouse_position.x <= rect.x1)) &
            ((mouse_position.y >= rect.y0) & (mouse_position.y <= rect.y1)),
    };
    return signal;
}

fn void ui_stack_reset(UI_State* state)
{
    auto* stack_end = (u32*)((u8*)&state->stacks + sizeof(state->stacks));
    
    for (auto* stack_pointer = (u32*)&state->stacks; stack_pointer != stack_end; stack_pointer += sizeof(VirtualBuffer(u8)) / sizeof(u32))
    {
        auto* length_pointer = stack_pointer + (offsetof(VirtualBuffer(u8), length) / sizeof(u32));
        *length_pointer = 0;
    }
}

UI_Size ui_pixels(u32 width, f32 strictness)
{
    return (UI_Size) {
        .kind = UI_SIZE_PIXEL_COUNT,
        .strictness = strictness,
        .value = (f32)width,
    };
}

UI_Size ui_percentage(f32 percentage, f32 strictness)
{
    return (UI_Size) {
        .kind = UI_SIZE_PERCENTAGE,
        .strictness = strictness,
        .value = percentage,
    };
}

UI_Size ui_em(f32 value, f32 strictness)
{
    auto font_size = ui_stack_top(font_size);
    assert(font_size);
    return (UI_Size) {
        .kind = UI_SIZE_PIXEL_COUNT,
        .strictness = strictness,
        .value = value * font_size,
    };
}

void ui_font_size(f32 size)
{
    ui_stack_push(font_size, size);
}

u8 ui_build_begin(OSWindow os_window, f64 frame_time, OSEventQueue* event_queue)
{
    ui_state->build_count += 1;
    auto* build_arena = ui_build_arena();
    arena_reset(build_arena);
    ui_state->frame_time = frame_time;
    ui_state->os_window = os_window;

    ui_stack_reset(ui_state);

    u8 open = 1;

    for (u32 generic_event_index = 0; open & (generic_event_index < event_queue->descriptors.length); generic_event_index += 1)
    {
        auto event_descriptor = event_queue->descriptors.pointer[generic_event_index];
        u32 event_index = event_descriptor.index;

        switch (event_descriptor.type)
        {
        case OS_EVENT_TYPE_MOUSE_BUTTON:
            {
                auto button = event_queue->mouse_buttons.pointer[event_index];
                auto previous_button_event = ui_state->mouse_button_events[button.button];
                switch (button.event.action)
                {
                    case OS_EVENT_MOUSE_RELAX:
                        unreachable();
                    case OS_EVENT_MOUSE_RELEASE:
                        {
                            assert(previous_button_event.action == OS_EVENT_MOUSE_PRESS);
                        } break;
                    case OS_EVENT_MOUSE_PRESS:
                        {
                            assert(previous_button_event.action == OS_EVENT_MOUSE_RELAX);
                        } break;
                    case OS_EVENT_MOUSE_REPEAT:
                        {
                            unreachable();
                        } break;
                }

                ui_state->mouse_button_events[button.button] = button.event;
            } break;
        case OS_EVENT_TYPE_WINDOW_FOCUS:
            {
            } break;
        case OS_EVENT_TYPE_CURSOR_POSITION:
            {
                auto mouse_position = event_queue->cursor_positions.pointer[event_index];
                ui_state->mouse_position = (UI_MousePosition) {
                    .x = mouse_position.x,
                    .y = mouse_position.y,
                };
            } break;
        case OS_EVENT_TYPE_CURSOR_ENTER:
            {
                todo();
            } break;
        case OS_EVENT_TYPE_WINDOW_POSITION:
            {
                // event_queue->window_positions.pointer[event_index];
                // todo();
            } break;
        case OS_EVENT_TYPE_WINDOW_CLOSE:
            {
                open = 0;
            } break;
        }
    }

    if (open)
    {
        // for (u64 i = 0; i < ui_state->widget_table.length; i += 1)
        // {
        //     auto* widget_table_element = &ui_state->widget_table.pointer[i];
        //     for (UI_Widget* widget = widget_table_element->first, *next = 0; widget; widget = next)
        //     {
        //         next = widget->hash_next;
        //
        //         if (ui_key_equal(widget->key, ui_key_null()) || widget->last_build_touched + 1 < ui_state->build_count)
        //         {
        //             // Remove from the list
        //             if (widget->hash_previous)
        //             {
        //                 widget->hash_previous->hash_next = widget->hash_next;
        //             }
        //
        //             if (widget->hash_next)
        //             {
        //                 widget->hash_next->hash_previous = widget->hash_previous;
        //             }
        //
        //             if (widget_table_element->first == widget)
        //             {
        //                 widget_table_element->first = widget->hash_next;
        //             }
        //
        //             if (widget_table_element->last == widget)
        //             {
        //                 widget_table_element->last = widget->hash_previous;
        //             }
        //         }
        //     }
        // }

        auto framebuffer_size = os_window_framebuffer_size_get(os_window);
        ui_stack_push_next_only(pref_width, ui_pixels(framebuffer_size.width, 1.0f));
        ui_stack_push_next_only(pref_height, ui_pixels(framebuffer_size.height, 1.0f));
        ui_stack_push_next_only(child_layout_axis, AXIS2_Y);

        auto* root = ui_widget_make_format((UI_WidgetFlags) {}, "window_root_{u64}", os_window);
        assert(!ui_state->stack_autopops.child_layout_axis);

        ui_stack_push(parent, root);

        ui_stack_push(font_size, 12);
        ui_stack_push(text_color, Color4(1, 1, 1, 1));
        ui_stack_push(background_color, Color4(0, 0, 0, 1));
        ui_stack_push(pref_width, ui_percentage(1.0, 0.0));
        ui_stack_push(pref_height, ui_em(1.8, 0.0));
    }

    return open;
}

fn void ui_compute_independent_sizes(UI_Widget* widget)
{
    for (Axis2 axis = 0; axis < AXIS2_COUNT; axis += 1)
    {
        auto pref_size = widget->pref_size[axis];
        switch (pref_size.kind)
        {
            default: break; case UI_SIZE_COUNT: unreachable();
            case UI_SIZE_PIXEL_COUNT:
                {
                    widget->computed_size.v[axis] = floorf(widget->pref_size[axis].value);
                } break;
        }
    }

    for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
    {
        ui_compute_independent_sizes(child_widget);
    }
}

fn void ui_compute_upward_dependent_sizes(UI_Widget* widget)
{
    // TODO: optimize loop out if possible
    for (Axis2 axis = 0; axis < AXIS2_COUNT; axis += 1)
    {
        auto pref_size = widget->pref_size[axis];
        switch (pref_size.kind)
        {
            default: break; case UI_SIZE_COUNT: unreachable();
            case UI_SIZE_PERCENTAGE:
            {
                for (UI_Widget* ancestor = widget->parent; ancestor; ancestor = ancestor->parent)
                {
                    if (ancestor->pref_size[axis].kind != UI_SIZE_BY_CHILDREN)
                    {
                        widget->computed_size.v[axis] = floorf(ancestor->computed_size.v[axis] * widget->pref_size[axis].value);
                        break;
                    }
                }
            } break;
        }
    }

    for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
    {
        ui_compute_upward_dependent_sizes(child_widget);
    }
}

fn void ui_compute_downward_dependent_sizes(UI_Widget* widget)
{
    for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
    {
        ui_compute_downward_dependent_sizes(child_widget);
    }

    for (Axis2 axis = 0; axis < AXIS2_COUNT; axis += 1)
    {
        auto pref_size = widget->pref_size[axis];
        switch (pref_size.kind)
        {
            default: break; case UI_SIZE_COUNT: unreachable();
            case UI_SIZE_BY_CHILDREN:
            {
                todo();
            } break;
        }
    }
}

fn void ui_resolve_conflicts(UI_Widget* widget)
{
    for (Axis2 axis = 0; axis < AXIS2_COUNT; axis += 1)
    {
        auto available_space = widget->computed_size.v[axis];
        f32 taken_space = 0;
        f32 total_fixup_budget = 0;

        if (!(widget->flags.v & (UI_WIDGET_FLAG_OVERFLOW_X << axis)))
        {
            for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
            {
                if (!(child_widget->flags.v & (UI_WIDGET_FLAG_FLOATING_X << axis)))
                {
                    if (axis == widget->child_layout_axis)
                    {
                        taken_space += child_widget->computed_size.v[axis];
                    }
                    else
                    {
                        taken_space = MAX(taken_space, child_widget->computed_size.v[axis]);
                    }
                    auto fixup_budget_this_child = child_widget->computed_size.v[axis] * (1 - child_widget->pref_size[axis].strictness);
                    total_fixup_budget += fixup_budget_this_child;
                }
            }

            auto conflict = taken_space - available_space;

            if (conflict > 0 && total_fixup_budget > 0)
            {
                for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
                {
                    if (!(child_widget->flags.v & (UI_WIDGET_FLAG_FLOATING_X << axis)))
                    {
                        auto fixup_budget_this_child = child_widget->computed_size.v[axis] * (1 - child_widget->pref_size[axis].strictness);
                        f32 fixup_size_this_child = 0;

                        if (axis == widget->child_layout_axis)
                        {
                            fixup_size_this_child = fixup_budget_this_child * (conflict / total_fixup_budget);
                        }
                        else
                        {
                            fixup_size_this_child = child_widget->computed_size.v[axis] - available_space;
                        }

                        fixup_size_this_child = CLAMP(0, fixup_size_this_child, fixup_budget_this_child);
                        child_widget->computed_size.v[axis] = floorf(child_widget->computed_size.v[axis] - fixup_size_this_child);
                    }
                }
            }
        }

        if (axis == widget->child_layout_axis)
        {
            f32 p = 0;

            for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
            {
                if (!(child_widget->flags.v & (UI_WIDGET_FLAG_FLOATING_X << axis)))
                {
                    child_widget->computed_relative_position.v[axis] = p;
                    p += child_widget->computed_size.v[axis];
                }
            }
        }
        else
        {
            for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
            {
                if (!(child_widget->flags.v & (UI_WIDGET_FLAG_FLOATING_X << axis)))
                {
                    child_widget->computed_relative_position.v[axis] = 0;
                }
            }
        }

        for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
        {
            auto last_relative_rect = child_widget->relative_rect;
            child_widget->relative_rect.p0.v[axis] = child_widget->computed_relative_position.v[axis];
            child_widget->relative_rect.p1.v[axis] = child_widget->relative_rect.p0.v[axis] + child_widget->computed_size.v[axis];

            F32Vec2 last_corner_01 = { .x = last_relative_rect.x0, .y = last_relative_rect.y1 };
            F32Vec2 last_corner_10 = { .x = last_relative_rect.x1, .y = last_relative_rect.y0 };
            F32Vec2 this_corner_01 = { .x = child_widget->relative_rect.x0, .y = child_widget->relative_rect.y1 };
            F32Vec2 this_corner_10 = { .x = child_widget->relative_rect.x1, .y = child_widget->relative_rect.y0 };

            child_widget->relative_corner_delta[CORNER_00].v[axis] = child_widget->relative_rect.p0.v[axis] - last_relative_rect.p0.v[axis];
            child_widget->relative_corner_delta[CORNER_01].v[axis] = this_corner_01.v[axis] - last_corner_01.v[axis];
            child_widget->relative_corner_delta[CORNER_10].v[axis] = this_corner_10.v[axis] - last_corner_10.v[axis];
            child_widget->relative_corner_delta[CORNER_11].v[axis] = child_widget->relative_rect.p1.v[axis] - last_relative_rect.p1.v[axis];

            child_widget->rect.p0.v[axis] = widget->rect.p0.v[axis] + child_widget->relative_rect.p0.v[axis] - widget->view_offset.v[axis];
            child_widget->rect.p1.v[axis] = child_widget->rect.p0.v[axis] + child_widget->computed_size.v[axis];

            if (!(child_widget->flags.v & (UI_WIDGET_FLAG_FLOATING_X << axis)))
            {
                child_widget->rect.p0.v[axis] = floorf(child_widget->rect.p0.v[axis]);
                child_widget->rect.p1.v[axis] = floorf(child_widget->rect.p1.v[axis]);
            }
        }

        for (UI_Widget* child_widget = widget->first; child_widget; child_widget = child_widget->next)
        {
            ui_resolve_conflicts(child_widget);
        }
    }
}

void ui_build_end()
{
    // Clear release button presses
    for (u32 i = 0; i < array_length(ui_state->mouse_button_events); i += 1)
    {
        auto* event = &ui_state->mouse_button_events[i];
        if (event->action == OS_EVENT_MOUSE_RELEASE)
        {
            event->action = OS_EVENT_MOUSE_RELAX;
        }
    }

    ui_stack_pop(parent);

    ui_compute_independent_sizes(ui_state->root);
    ui_compute_upward_dependent_sizes(ui_state->root);
    ui_compute_downward_dependent_sizes(ui_state->root);
    ui_resolve_conflicts(ui_state->root);
}

fn RenderRect render_rect(F32Interval2 rect)
{
    return (RenderRect) {
        .x0 = rect.x0,
        .y0 = rect.y0,
        .x1 = rect.x1,
        .y1 = rect.y1,
    };
}

void ui_draw()
{
    UI_Widget* root = ui_state->root;

    UI_Widget* widget = root;
    RenderWindow* window = ui_state->render_window;
    Renderer* renderer = ui_state->renderer;

    while (1)
    {
        // print("Widget 0x{u64:x}. {u32} {u32} {u32} {u32}\n", widget, (u32)widget->rect.p0.x, (u32)widget->rect.p0.y, (u32)widget->rect.p1.x, (u32)widget->rect.p1.y);
        if (widget->flags.draw_background)
        {
            window_render_rect(window, (RectDraw) {
                .color = widget->background_color,
                .vertex = render_rect(widget->rect),
            });
        }

        if (widget->flags.draw_text)
        {
            window_render_text(renderer, window, widget->text, widget->text_color, RENDER_FONT_TYPE_PROPORTIONAL, widget->rect.x0, widget->rect.y0);
        }

        if (widget->first)
        {
            widget = widget->first;
        }
        else if (widget->next)
        {
            widget = widget->next;
        }
        else if (widget->parent == ui_state->root)
        {
            break;
        }
        else if (widget->parent)
        {
            widget = widget->parent;
        }
        else
        {
            break;
        }
    }
}

