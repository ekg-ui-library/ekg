/*
 * VOKEGPU EKG LICENSE
 *
 * Respect ekg license policy terms, please take a time and read it.
 * 1- Any "skidd" or "stole" is not allowed.
 * 2- Forks and pull requests should follow the license policy terms.
 * 3- For commercial use, do not sell without give credit to vokegpu ekg.
 * 4- For ekg users and users-programmer, we do not care, free to use in anything (utility, hacking, cheat, game, software).
 * 5- Malware, rat and others virus. We do not care.
 * 6- Do not modify this license under any instance.
 *
 * @VokeGpu 2022 all rights reserved.
 */

#include "ekg/core/runtime.hpp"
#include "ekg/util/util_event.hpp"
#include "ekg/ui/frame/ui_frame.hpp"
#include "ekg/ui/frame/ui_frame_widget.hpp"
#include "ekg/ui/button/ui_button_widget.hpp"
#include "ekg/ui/label/ui_label_widget.hpp"
#include "ekg/ui/checkbox/ui_checkbox_widget.hpp"
#include "ekg/ui/slider/ui_slider_widget.hpp"

ekg::stack ekg::swap::collect {};
ekg::stack ekg::swap::back {};
ekg::stack ekg::swap::front {};
std::vector<ekg::ui::abstract_widget*> ekg::swap::buffer {};

void ekg::swap::refresh() {
    ekg::swap::collect.clear();
    ekg::swap::back.clear();
    ekg::swap::front.clear();
    ekg::swap::buffer.clear();
}

void ekg::runtime::set_root(SDL_Window *sdl_win_root) {
    this->root = sdl_win_root;
}

SDL_Window *ekg::runtime::get_root() {
    return this->root;
}

void ekg::runtime::init() {
    this->allocator.init();
    this->theme_service.init();

    if (FT_Init_FreeType(&ekg::draw::font_renderer::ft_library)) {
        ekg::log("could not init FreeType");
    }

    this->prepare_tasks();
    this->prepare_ui_env();
    this->layout_service.init();
}

void ekg::runtime::quit() {
    this->allocator.quit();
    this->theme_service.quit();
    this->layout_service.quit();
}

ekg::draw::font_renderer &ekg::runtime::get_f_renderer_small() {
    return this->f_renderer_small;
}

ekg::draw::font_renderer &ekg::runtime::get_f_renderer_normal() {
    return this->f_renderer_normal;
}

ekg::draw::font_renderer &ekg::runtime::get_f_renderer_big() {
    return this->f_renderer_big;
}

void ekg::runtime::process_event(SDL_Event &sdl_event) {
    this->input_service.on_event(sdl_event);

    bool pressed {ekg::input::pressed()};
    bool released {ekg::input::released()};

    if (pressed || released || ekg::input::motion()) {
        this->widget_id_focused = 0;
    }

    ekg::ui::abstract_widget* focused_widget {nullptr};
    bool hovered {};
    bool found_absolute_widget {};

    for (ekg::ui::abstract_widget* &widgets : this->loaded_widget_list) {
        if (widgets == nullptr || !widgets->data->is_alive()) {
            continue;
        }

        widgets->on_pre_event(sdl_event);
        hovered = widgets->flag.hovered && widgets->data->get_state() == ekg::state::visible;

        if ((hovered || widgets->flag.absolute) && !found_absolute_widget) {
            this->widget_id_focused = widgets->data->get_id();
            focused_widget = widgets;
            found_absolute_widget = widgets->flag.absolute;
        }

        widgets->flag.hovered = false;

        if (!hovered || found_absolute_widget) {
            widgets->on_event(sdl_event);
        }
    }

    if (focused_widget != nullptr) {
        focused_widget->on_pre_event(sdl_event);
        focused_widget->on_event(sdl_event);
        focused_widget->flag.hovered = false;
    }

    if (pressed) {
        this->widget_id_pressed_focused = this->widget_id_focused;
    } else if (released) {
        this->widget_id_released_focused = this->widget_id_focused;
    }

    if (this->prev_widget_id_focused != this->widget_id_focused && this->widget_id_focused != 0 && (pressed || released)) {
        this->swap_widget_id_focused = this->widget_id_focused;
        this->prev_widget_id_focused = this->widget_id_focused;

        ekg::dispatch(ekg::env::swap);
        ekg::dispatch(ekg::env::redraw);
    }
}

void ekg::runtime::process_update() {    
    this->input_service.on_update();

    if (this->handler_service.should_poll()) {
        this->handler_service.on_update();
    }

    this->allocator.on_update();
    glViewport(0, 0, ekg::display::width, ekg::display::height);
}

void ekg::runtime::process_render() {
    this->allocator.draw();
}

ekg::gpu::allocator &ekg::runtime::get_gpu_allocator() {
    return this->allocator;
}

ekg::service::handler &ekg::runtime::get_service_handler() {
    return this->handler_service;
}

void ekg::runtime::prepare_tasks() {
    ekg::log("creating task events");

    /*
      The priority of tasks is organised by compilation code, means that you need
      to dispatch tasks in n sequence. This is only a feature for allocated tasks.
     */

    this->handler_service.dispatch(new ekg::cpu::event {"refresh", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_refresh_list) {
            if (widgets == nullptr) {
                continue;
            }

            if (!widgets->data->is_alive()) {
                runtime->widget_map.erase(widgets->data->get_id());
                delete widgets;
                widgets = nullptr;
            }

            runtime->loaded_widget_list.push_back(widgets);
        }

        runtime->loaded_widget_refresh_list.clear();
    }, ekg::event::alloc});

    this->handler_service.dispatch(new ekg::cpu::event {"swap", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};
        if (runtime->swap_widget_id_focused == 0) {
            return;
        }

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_list) {
            if (widgets == nullptr) {
                continue;
            }

            if (ekg::swap::collect.registry[widgets->data->get_id()] || ekg::swap::front.registry[widgets->data->get_id()]) {
                continue;
            }

            ekg::swap::collect.clear();
            ekg::push_back_stack(widgets, ekg::swap::collect);

            if (ekg::swap::collect.registry[runtime->swap_widget_id_focused]) {
                ekg::swap::front.registry.insert(ekg::swap::collect.registry.begin(), ekg::swap::collect.registry.end());
                ekg::swap::front.ordered_list.insert(ekg::swap::front.ordered_list.end(), ekg::swap::collect.ordered_list.begin(), ekg::swap::collect.ordered_list.end());
            } else {
                ekg::swap::back.registry.insert(ekg::swap::collect.registry.begin(), ekg::swap::collect.registry.end());
                ekg::swap::back.ordered_list.insert(ekg::swap::back.ordered_list.end(), ekg::swap::collect.ordered_list.begin(), ekg::swap::collect.ordered_list.end());
            }
        }

        runtime->swap_widget_id_focused = 0;
        runtime->loaded_widget_list.clear();

        for (ekg::ui::abstract_widget* &widget : ekg::swap::back.ordered_list) {
            if (widget == nullptr) {
                continue;
            }

            runtime->loaded_widget_list.push_back(widget);
        }

        for (ekg::ui::abstract_widget* &widget : ekg::swap::front.ordered_list) {
            if (widget == nullptr) {
                continue;
            }

            runtime->loaded_widget_list.push_back(widget);
        }

        ekg::swap::refresh();
    }, ekg::event::alloc});

    this->handler_service.dispatch(new ekg::cpu::event {"reload", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_reload_list) {
            if (widgets == nullptr) {
                continue;
            }

            auto &sync_flags {widgets->data->get_sync()};
            if (ekg::bitwise::contains(sync_flags, (uint16_t) ekg::uisync::reset)) {
                ekg::bitwise::remove(sync_flags, (uint16_t) ekg::uisync::reset);

                switch (widgets->data->get_type()) {
                    case ekg::type::frame: {
                        auto ui {(ekg::ui::frame *) widgets->data};
                        auto pos {ui->get_pos_initial()};
                        auto size {ui->get_size_initial()};

                        if (ui->widget() != ekg::vec4 {pos, size}) {
                            widgets->dimension.w = size.x;
                            widgets->dimension.h = size.y;

                            if (ui->get_parent_id() != 0) {
                                widgets->dimension.x = pos.x - widgets->parent->x;
                                widgets->dimension.y = pos.y - widgets->parent->y;
                            } else {
                                widgets->parent->x = pos.x;
                                widgets->parent->y = pos.y;
                            }
                        }

                        break;
                    }

                    default: {
                        break;
                    }
                }
            }

            if (ekg::bitwise::contains(sync_flags, (uint16_t) ekg::uisync::dimension)) {
                ekg::bitwise::remove(sync_flags, (uint16_t) ekg::uisync::dimension);

                auto &rect {widgets->data->ui()};
                widgets->dimension.w = rect.w;
                widgets->dimension.h = rect.h;

                if (widgets->data->get_parent_id() != 0) {
                    widgets->dimension.x = rect.x - widgets->parent->x;
                    widgets->dimension.y = rect.y - widgets->parent->y;
                } else {
                    widgets->parent->x = rect.x;
                    widgets->parent->y = rect.y;
                }
            }

            widgets->on_reload();
            runtime->processed_widget_map[widgets->data->get_id()] = true;
        }

        runtime->loaded_widget_reload_list.clear();
        runtime->processed_widget_map.clear();
    }, ekg::event::alloc});

    this->handler_service.dispatch(new ekg::cpu::event {"synclayout", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};
        // todo fix the issue with sync layout offset.

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_sync_layou_list) {
            if (widgets == nullptr || runtime->processed_widget_map[widgets->data->get_id()]) {
                continue;
            }

            runtime->layout_service.process_scaled(widgets);
            runtime->processed_widget_map[widgets->data->get_id()] = true;
        }

        runtime->loaded_widget_sync_layou_list.clear();
        runtime->processed_widget_map.clear();
    }, ekg::event::alloc});

    this->handler_service.dispatch(new ekg::cpu::event {"redraw", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};

        runtime->allocator.invoke();
        runtime->f_renderer_big.blit("Widgets count: " + std::to_string(runtime->loaded_widget_list.size()), 10, 10, {255, 255, 255, 255});

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_list) {
            if (widgets != nullptr && widgets->data->is_alive() && widgets->data->get_state() == ekg::state::visible) {
                widgets->on_draw_refresh();
            }
        }

        runtime->allocator.revoke();
    }, ekg::event::alloc});

    this->handler_service.dispatch(new ekg::cpu::event {"scissor", this, [](void* p_data) {
        auto runtime {static_cast<ekg::runtime*>(p_data)};

        float scissor[4] {}, scissor_parent_master[4] {};
        ekg::ui::abstract_widget *parent_master {nullptr};
        ekg::gpu::scissor *gpu_scissor {nullptr}, *gpu_parent_master_scissor {nullptr};
        ekg::rect widget_rect {};

        for (ekg::ui::abstract_widget* &widgets : runtime->loaded_widget_scissor_list) {
            if (runtime->processed_widget_map[widgets->data->get_id()] || (parent_master = ekg::find_absolute_parent_master(widgets)) == nullptr) {
                continue;
            }

            ekg::swap::front.clear();
            ekg::push_back_stack(parent_master, ekg::swap::front);

            /*
             Scissor is a great feature from OpenGL, but it
             does not stack, means that GL context does not
             accept scissor inside scissor.

             After 1 year studying scissor, I  built one scheme,
             compute bounds of all parent widgets with the parent
             master, obvious it takes some ticks but there is no
             other way (maybe I am wrong).

             Two things important:
             1 - This scissors scheme use scissor IDs from widgets.
             2 - Iteration collect ALL parent families and sub parent of target.
            */

            for (ekg::ui::abstract_widget* &scissor_widget : ekg::swap::front.ordered_list) {
                gpu_scissor = runtime->allocator.get_scissor_by_id(scissor_widget->data->get_id());
                if (gpu_scissor == nullptr) {
                    continue;
                }

                if (scissor_widget->data->get_parent_id() == 0) {
                    widget_rect = scissor_widget->get_abs_rect();
                    gpu_scissor->rect[0] = widget_rect.x;
                    gpu_scissor->rect[1] = widget_rect.y;
                    gpu_scissor->rect[2] = widget_rect.w;
                    gpu_scissor->rect[3] = widget_rect.h;
                    continue;
                }

                parent_master = runtime->widget_map[scissor_widget->data->get_parent_id()];
                gpu_parent_master_scissor = runtime->get_gpu_allocator().get_scissor_by_id(parent_master->data->get_id());
                if (gpu_parent_master_scissor == nullptr) {
                    continue;
                }

                widget_rect = scissor_widget->get_abs_rect();

                scissor_parent_master[0] = gpu_parent_master_scissor->rect[0];
                scissor_parent_master[1] = gpu_parent_master_scissor->rect[1];
                scissor_parent_master[2] = gpu_parent_master_scissor->rect[2];
                scissor_parent_master[3] = gpu_parent_master_scissor->rect[3];

                scissor[0] = widget_rect.x;
                scissor[1] = widget_rect.y;
                scissor[2] = widget_rect.w;
                scissor[3] = widget_rect.h;

                if (scissor[0] < scissor_parent_master[0]) {
                    scissor[0] = scissor_parent_master[0];
                }

                if (scissor[1] < scissor_parent_master[1]) {
                    scissor[1] = scissor_parent_master[1];
                }

                if (scissor[0] + scissor[2] > scissor_parent_master[0] + scissor_parent_master[2]) {
                    scissor[2] -= (scissor[0] + scissor[2]) - (scissor_parent_master[0] + scissor_parent_master[2]);
                }

                if (scissor[1] + scissor[3] > scissor_parent_master[1] + scissor_parent_master[3]) {
                    scissor[3] -= (scissor[1] + scissor[3]) - (scissor_parent_master[1] + scissor_parent_master[3]);
                }

                gpu_scissor->rect[0] = scissor[0];
                gpu_scissor->rect[1] = scissor[1];
                gpu_scissor->rect[2] = scissor[2];
                gpu_scissor->rect[3] = scissor[3];
            }

            runtime->processed_widget_map[widgets->data->get_id()] = true;
        }

        runtime->processed_widget_map.clear();
        runtime->loaded_widget_scissor_list.clear();
        ekg::swap::front.clear();
    }, ekg::event::alloc});
}

ekg::ui::abstract_widget *ekg::runtime::get_fast_widget_by_id(int32_t id) {
    /* widget ID 0 is defined as none, or be, ID token accumulation start with 1 and not 0 */
    return id ? this->widget_map[id] : nullptr;
}

void ekg::runtime::do_task_reload(ekg::ui::abstract_widget* widget) {
    if (widget != nullptr) {
        this->loaded_widget_reload_list.push_back(widget);
        ekg::dispatch(ekg::env::reload);
    }
}

ekg::service::input &ekg::runtime::get_service_input() {
    return this->input_service;
}

ekg::service::theme &ekg::runtime::get_service_theme() {
    return this->theme_service;
}

ekg::service::layout &ekg::runtime::get_service_layout() {
    return this->layout_service;
}

void ekg::runtime::prepare_ui_env() {
    ekg::log("creating widget fonts");

    this->f_renderer_small.font_size = 16;
    this->f_renderer_small.reload();
    this->f_renderer_small.bind_allocator(&this->allocator);

    this->f_renderer_normal.font_size = 18;
    this->f_renderer_normal.reload();
    this->f_renderer_normal.bind_allocator(&this->allocator);

    this->f_renderer_big.font_size = 24;
    this->f_renderer_big.reload();
    this->f_renderer_big.bind_allocator(&this->allocator);

    ekg::log("creating widget binds");

    /* start of frame input binds */
    this->input_service.bind("frame-drag-activy", "mouse-left");
    this->input_service.bind("frame-drag-activy", "finger-click");
    this->input_service.bind("frame-resize-activy", "mouse-left");
    this->input_service.bind("frame-resize-activy", "finger-click");
    /* end of frame input binds */

    /* start of button input binds */
    this->input_service.bind("button-activy", "mouse-left");
    this->input_service.bind("button-activy", "finger-click");
    /* end of button input binds */

    /* start of checkbox input binds */
    this->input_service.bind("checkbox-activy", "mouse-left");
    this->input_service.bind("checkbox-activy", "finger-click");
    this->input_service.bind("popup-activy", "mouse-right");
    this->input_service.bind("popup-activy", "finger-hold");
    /* end of checkbox input binds */
    
    /* start of popup input binds */
    this->input_service.bind("popup-component-activy", "mouse-left");
    this->input_service.bind("popup-component-activy", "finger-click");
    /* end of popup input binds */

    /* start of text input binds */
    this->input_service.bind("textbox-action-delete-left", "backspace");
    this->input_service.bind("textbox-action-select-all", "lctrl+a");
    this->input_service.bind("textbox-action-select-all", "finger-hold");
    this->input_service.bind("textbox-action-select-all", "mouse-left-double");
    this->input_service.bind("textbox-action-delete-right", "delete");
    this->input_service.bind("textbox-action-break-line", "return");
    this->input_service.bind("textbox-action-up", "up");
    this->input_service.bind("textbox-action-down", "down");
    this->input_service.bind("textbox-action-down", "right");
    this->input_service.bind("textbox-action-down", "left");
    /* end of textbox input binds */

    /* start of slider input binds */
    this->input_service.bind("slider-activy", "mouse-left");
    this->input_service.bind("slider-activy", "finger-click");
    this->input_service.bind("slider-bar-increase", "mouse-wheel-up");
    this->input_service.bind("slider-bar-decrease", "mouse-wheel-down");
    /* end of slider input binds */
}

void ekg::runtime::gen_widget(ekg::ui::abstract* ui) {
    ui->set_id(++this->token_id);

    this->swap_widget_id_focused = ui->get_id();
    ekg::ui::abstract_widget* created_widget {nullptr};
    bool is_group {};

    switch (ui->get_type()) {
        case type::abstract: {
            auto widget {new ekg::ui::abstract_widget {}};
            widget->data = ui;
            created_widget = widget;
            break;
        }

        case type::frame: {
            auto widget {new ekg::ui::frame_widget {}};
            widget->is_scissor_refresh = true;
            widget->data = ui;

            is_group = true;
            created_widget = widget;
            this->current_bind_group = created_widget;
            ui->reset();

            break;
        }

        case type::button: {
            auto widget {new ekg::ui::button_widget {}};
            widget->data = ui;
            created_widget = widget;
            break;
        }

        case type::label: {
            auto widget {new ekg::ui::label_widget {}};
            widget->data = ui;
            created_widget = widget;
            break;
        }

        case type::checkbox: {
            auto widget {new ekg::ui::checkbox_widget {}};
            widget->data = ui;
            created_widget = widget;
            break;
        }

        case type::slider: {
            auto widget {new ekg::ui::slider_widget {}};
            widget->data = ui;
            created_widget = widget;
            break;
        }
    }

    this->loaded_widget_refresh_list.push_back(created_widget);
    this->widget_map[ui->get_id()] = created_widget;
    this->do_task_reload(created_widget);

    if (!is_group && this->current_bind_group != nullptr) {
        this->current_bind_group->data->add_child(ui->get_id());
    } else if (is_group) {
        this->do_task_synclayout(created_widget);
        this->do_task_scissor(created_widget);
    }

    ekg::log("created widget " + std::to_string(ui->get_id()));
    ekg::dispatch(ekg::env::refresh);
    ekg::dispatch(ekg::env::swap);
}

void ekg::runtime::do_task_scissor(ekg::ui::abstract_widget *widget) {
    if (widget != nullptr) {
        this->loaded_widget_scissor_list.push_back(widget);
        ekg::dispatch(ekg::env::scissor);
    }
}

void ekg::runtime::do_task_synclayout(ekg::ui::abstract_widget *widget) {
    if (widget == nullptr) {
        return;
    }

    bool is_group {widget->data->get_type() == ekg::type::frame};
    bool check_parent {is_group == false && widget->data->has_parent()};

    if (check_parent) {
        widget = this->get_fast_widget_by_id(widget->data->get_parent_id());
    }

    if (widget != nullptr) {
        this->loaded_widget_sync_layou_list.push_back(widget);
        ekg::dispatch(ekg::env::synclayout);
    }
}

void ekg::runtime::end_group_flag() {
    this->current_bind_group = nullptr;
}
