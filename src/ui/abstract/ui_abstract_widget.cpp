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

#include "ekg/ui/abstract/ui_abstract_widget.hpp"
#include "ekg/util/gui.hpp"
#include "ekg/draw/draw.hpp"
#include "ekg/core/memory.hpp"

ekg::ui::abstract_widget::abstract_widget() {
  this->p_parent = &this->empty_parent;
  this->p_scroll = &this->empty_scroll;
}

ekg::ui::abstract_widget::~abstract_widget() {
  this->on_destroy();
}

void ekg::ui::abstract_widget::on_create() {
  this->is_just_created = true;
}

void ekg::ui::abstract_widget::on_destroy() {
  if (EKG_MEMORY_MUST_FREE_TASKS_AUTOMATICALLY) {
    for (uint64_t it {}; it < EKG_MEMORY_ACTIONS_SIZE; it++) {
      ekg::task *p_task {
        this->p_data->get_task((ekg::action)it)
      };

      if (p_task->unsafe_is_allocated) {
        delete p_task;
      }
    }
  }
}

void ekg::ui::abstract_widget::on_reload() {
  ekg::dispatch(ekg::env::redraw);
}

void ekg::ui::abstract_widget::on_pre_event(ekg::os::io_event_serial &io_event_serial) {
  if (ekg::input::pressed() || ekg::input::released() || ekg::input::motion() || ekg::input::wheel()) {
    ekg::vec4 &interact {ekg::input::interact()};
    ekg::rect &rect {this->get_abs_rect()};

    this->flag.hovered = (
      ekg::rect_collide_vec(rect, interact) && (
      this->p_data->get_level() == ekg::level::top_level ||
      this->p_data->get_parent_id() == 0 ||
      ekg::rect_collide_vec(this->scissor, interact))
    );
  }
}

void ekg::ui::abstract_widget::on_event(ekg::os::io_event_serial &io_event_serial) {

}

void ekg::ui::abstract_widget::on_post_event(ekg::os::io_event_serial &io_event_serial) {
  this->flag.hovered = false;

  #if defined(__ANDROID__)
  this->flag.highlight = (
    !(!this->flag.hovered &&
    (ekg::input::released())) &&
    this->flag.highlight
  )
  #endif
}

void ekg::ui::abstract_widget::on_update() {

}

void ekg::ui::abstract_widget::on_draw_refresh() {

}

ekg::rect ekg::ui::abstract_widget::get_static_rect() {
  return this->dimension + *this->p_parent;
}

ekg::rect &ekg::ui::abstract_widget::get_abs_rect() {
  ekg::rect &rect {
    (this->p_data->widget() = (this->dimension + *this->p_parent + *this->p_scroll))
  };

  /**
   * ~~As pointed here: ekg/layout/docknize -> ekg::layout::docknize -> fill property~~
   * 
   * ~~The pixel imperfect is easy fixed with an int32_t cast.~~
   * ~~I do not care anymore about incorrect quads aligned,
   * ~~too stupid.~~
   * 
   * I do not know what I am talking about.
   * - Rina.
   **/

  rect.x = static_cast<int32_t>(rect.x);
  rect.y = static_cast<int32_t>(rect.y);
  rect.w = static_cast<int32_t>(rect.w);
  rect.h = static_cast<int32_t>(rect.h);

  return rect;
}