/**
 * MIT License
 * 
 * Copyright (c) 2022-2024 Rina Wilk / vokegpu@gmail.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ekg/ekg.hpp"
#include "ekg/os/platform.hpp"
#include "ekg/util/geometry.hpp"

ekg::runtime *ekg::core {};
bool ekg::running {};

ekg::service::theme_scheme_t &ekg::current_theme_scheme() {
  return ekg::core->service_theme.get_current_theme_scheme();
}

ekg::service::theme &ekg::theme() {
  return ekg::core->service_theme;
}

ekg::draw::font_renderer &ekg::f_renderer(ekg::font font_size) {
  switch (font_size) {
    case ekg::font::big: {
      return ekg::core->f_renderer_big;
    }

    case ekg::font::normal: {
      return ekg::core->f_renderer_normal;
    }

    case ekg::font::small: {
      return ekg::core->f_renderer_small;
    }
  }

  return ekg::core->f_renderer_normal;
}

void ekg::init(
  ekg::runtime *p_ekg_runtime,
  ekg::runtime_property *p_ekg_runtime_property
) {
  ekg::log() << "Initializing FreeType library";

  if (FT_Init_FreeType(&ekg::draw::font_renderer::ft_library)) {
    ekg::log() << "Error: Failed to init FreeType library";
  }

  ekg::log() << "Initializing built-in OS platform-interface";

  p_ekg_runtime->p_os_platform = p_ekg_runtime_property->p_os_platform;
  p_ekg_runtime->p_os_platform->init();

  ekg::log() << "Initializing built-in GPU hardware-interface";

  p_ekg_runtime->p_gpu_api = p_ekg_runtime_property->p_gpu_api;
  p_ekg_runtime->p_gpu_api->init();
  p_ekg_runtime->p_gpu_api->log_vendor_details();

  ekg::log() << "Initializing EKG";

  ekg::core = p_ekg_runtime;
  ekg::running = true;

  ekg::core->f_renderer_small.init();
  ekg::core->f_renderer_small.set_font(p_ekg_runtime_property->p_font_path);
  ekg::core->f_renderer_small.set_font_emoji(p_ekg_runtime_property->p_font_path_emoji);

  ekg::core->f_renderer_normal.init();
  ekg::core->f_renderer_normal.set_font(p_ekg_runtime_property->p_font_path);
  ekg::core->f_renderer_normal.set_font_emoji(p_ekg_runtime_property->p_font_path_emoji);

  ekg::core->f_renderer_big.init();
  ekg::core->f_renderer_big.set_font(p_ekg_runtime_property->p_font_path);
  ekg::core->f_renderer_big.set_font_emoji(p_ekg_runtime_property->p_font_path_emoji);

  ekg::core->init();
}

void ekg::quit() {
  ekg::core->p_os_platform->quit();
  ekg::core->p_gpu_api->quit();
  ekg::core->quit();
  ekg::running = false;

  ekg::log() << "Shutdown complete - Thank you for using EKG ;) <3";
}

void ekg::update() {
  ekg::core->process_update();
  ekg::core->p_os_platform->update_cursor(ekg::cursor);
  ekg::core->io_event_serial.event_type = ekg::platform_event_type::none;
}

void ekg::render() {
  ekg::core->process_render();
}

ekg::ui::frame *ekg::frame(std::string_view tag, ekg::rect rect) {
  ekg::ui::frame *p_ui {new ekg::ui::frame()};
  ekg::flags sync_ui_flags {static_cast<ekg::flags>(ekg::ui_sync::dimension)};

  bool is_w_set {!ekg_equals_float(rect.w, ekg_no_update_placement)};
  bool is_h_set {!ekg_equals_float(rect.h, ekg_no_update_placement)};

  p_ui->set_tag(tag);
  p_ui->unsafe_set_type(ekg::type::frame);
  p_ui->set_place(ekg::dock::free);
  ekg::core->gen_widget(p_ui);

  p_ui->ui() = {
    rect.x,
    rect.y,
    rect.w,
    rect.h
  };

  sync_ui_flags |= static_cast<ekg::flags>(ekg::ui_sync::set_x) | static_cast<ekg::flags>(ekg::ui_sync::set_y);
  sync_ui_flags |= static_cast<ekg::flags>(ekg::ui_sync::set_width) * is_w_set;
  sync_ui_flags |= static_cast<ekg::flags>(ekg::ui_sync::set_height) * is_h_set;

  p_ui->unsafe_sync_ui(static_cast<ekg::flags>(sync_ui_flags));
  p_ui->set_auto_initial_dimension(
    !ekg_bitwise_contains(static_cast<ekg::flags>(sync_ui_flags), static_cast<ekg::flags>(ekg::ui_sync::set_height))
  );

  return p_ui;
}

ekg::ui::frame *ekg::frame(std::string_view tag, ekg::rect rect, ekg::flags dock) {
  ekg::ui::frame *p_ui {new ekg::ui::frame()};
  ekg::flags sync_ui_flags {static_cast<ekg::flags>(ekg::ui_sync::dimension)};

  bool is_w_set {!ekg_equals_float(rect.w, ekg_no_update_placement)};
  bool is_h_set {!ekg_equals_float(rect.h, ekg_no_update_placement)};

  if (!ekg::core->has_bind_group_flag()) {
    dock = ekg::dock::free;
  }

  p_ui->set_tag(tag);
  p_ui->unsafe_set_type(ekg::type::frame);
  p_ui->set_place(dock);
  ekg::core->gen_widget(p_ui);

  p_ui->ui() = {
    rect.x,
    rect.y,
    rect.w,
    rect.h
  };

  sync_ui_flags |= static_cast<ekg::flags>(ekg::ui_sync::set_width) * is_w_set;
  sync_ui_flags |= static_cast<ekg::flags>(ekg::ui_sync::set_height) * is_h_set;

  p_ui->unsafe_sync_ui(static_cast<ekg::flags>(sync_ui_flags));
  p_ui->set_auto_initial_dimension(
    !ekg_bitwise_contains(static_cast<ekg::flags>(sync_ui_flags), static_cast<ekg::flags>(ekg::ui_sync::set_height))
  );

  return p_ui;
}

ekg::ui::button *ekg::button(std::string_view text, ekg::flags dock) {
  ekg::ui::button *p_ui {new ekg::ui::button()};
  p_ui->registry(p_ui);
  p_ui->reset_ownership();
  p_ui->unsafe_set_type(ekg::type::button);
  ekg::core->gen_widget(p_ui);

  p_ui->set_text(text);
  p_ui->set_place(dock);
  p_ui->set_scaled_height(1);
  p_ui->set_font_size(ekg::font::normal);
  p_ui->set_text_align(ekg::dock::left);
  p_ui->set_tag(text);

  return p_ui;
}

ekg::ui::label *ekg::label(std::string_view text, ekg::flags dock) {
  ekg::ui::label *p_ui {new ekg::ui::label()};
  p_ui->registry(p_ui);
  p_ui->reset_ownership();
  p_ui->unsafe_set_type(ekg::type::label);
  ekg::core->gen_widget(p_ui);

  p_ui->set_value(std::string(text));
  p_ui->set_place(dock);
  p_ui->set_scaled_height(1);
  p_ui->set_font_size(ekg::font::normal);
  p_ui->set_text_align(ekg::dock::left);
  p_ui->set_tag(text);

  return p_ui;
}

ekg::ui::checkbox *ekg::checkbox(std::string_view text, bool value, ekg::flags dock) {
  ekg::ui::checkbox *p_ui {new ekg::ui::checkbox()};
  p_ui->registry(p_ui);
  p_ui->reset_ownership();
  p_ui->unsafe_set_type(ekg::type::checkbox);
  ekg::core->gen_widget(p_ui);

  p_ui->set_text(text);
  p_ui->set_place(dock);
  p_ui->set_scaled_height(1);
  p_ui->set_font_size(ekg::font::normal);
  p_ui->set_text_align(ekg::dock::left);
  p_ui->set_box_align(ekg::dock::left);
  p_ui->set_tag(text);
  p_ui->set_value(value);

  return p_ui;
}

ekg::ui::popup *
ekg::popup(std::string_view tag, const std::vector<std::string> &component_list, bool interact_position) {
  if (ekg::hovered.type == ekg::type::popup || ekg::hovered.down_type == ekg::type::popup) {
    return nullptr;
  }

  ekg::ui::popup *p_ui {new ekg::ui::popup()};
  p_ui->unsafe_set_type(ekg::type::popup);
  ekg::core->gen_widget(p_ui);

  if (interact_position) {
    ekg::vec4 &interact {ekg::input::interact()};
    p_ui->set_pos(interact.x, interact.y);
  }

  p_ui->set_width(100);
  p_ui->insert(component_list);
  p_ui->set_tag(tag);
  p_ui->set_scaled_height(1);
  p_ui->set_text_align(ekg::dock::center | ekg::dock::left);
  p_ui->set_level(ekg::level::top_level);

  return p_ui;
}

ekg::ui::textbox *ekg::textbox(std::string_view tag, std::string_view text, ekg::flags dock) {
  ekg::ui::textbox *p_ui {new ekg::ui::textbox()};
  p_ui->registry(p_ui);
  p_ui->reset_ownership();
  p_ui->unsafe_set_type(ekg::type::textbox);
  ekg::core->gen_widget(p_ui);

  p_ui->set_tag(tag);
  p_ui->set_place(dock);
  p_ui->set_scaled_height(1);
  p_ui->set_font_size(ekg::font::normal);
  p_ui->set_width(200);
  p_ui->p_value->emplace_back() = text;
  p_ui->set_state(ekg::state::enable);

  return p_ui;
}

ekg::ui::listbox *ekg::listbox(
  std::string_view tag,
  const ekg::item &item_list,
  ekg::flags dock
) {
  ekg::ui::listbox *p_ui {new ekg::ui::listbox()};
  p_ui->registry(p_ui);
  p_ui->reset_ownership();
  p_ui->unsafe_set_type(ekg::type::listbox);
  ekg::core->gen_widget(p_ui);

  p_ui->set_tag(tag);
  p_ui->set_place(dock);
  p_ui->set_scaled_height(6);
  p_ui->set_item_font_size(ekg::font::normal);
  p_ui->set_column_header_font_size(ekg::font::normal);
  p_ui->set_value(item_list);
  p_ui->set_column_header_align(ekg::dock::fill);

  return p_ui;
}

ekg::ui::scrollbar *ekg::scrollbar(std::string_view tag) {
  ekg::ui::scrollbar *p_ui {new ekg::ui::scrollbar()};
  p_ui->unsafe_set_type(ekg::type::scrollbar);
  ekg::core->gen_widget(p_ui);
  p_ui->set_tag(tag);

  return p_ui;
}

bool ekg::has_bind_group_flag() {
  return ekg::core->has_bind_group_flag();
}

void ekg::pop_group() {
  ekg::core->end_group_flag();
}

void ekg::pop_group_parent() {
  ekg::core->end_group_parent_flag();
}
