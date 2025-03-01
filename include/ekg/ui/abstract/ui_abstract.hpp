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

#ifndef EKG_UI_ABSTRACT_H
#define EKG_UI_ABSTRACT_H

#include <vector>
#include <array>

#include "ekg/util/geometry.hpp"
#include "ekg/core/task.hpp"
#include "ekg/gpu/api.hpp"
#include "ekg/util/io.hpp"

namespace ekg {
  enum class type {
    abstract,
    frame,
    button,
    label,
    slider,
    slider2d,
    checkbox,
    textbox,
    combobox,
    listbox,
    tab,
    popup,
    scrollbar,
  };

  enum class level {
    bottom_level,
    top_level,
  };

  enum class ui_sync {
    dimension  = 2 << 1,
    set_x      = 2 << 2,
    set_y      = 2 << 3,
    set_width  = 2 << 4,
    set_height = 2 << 5,
  };

  enum class action {
    press = 0,
    release = 1,
    hover = 2,
    drag = 3,
    motion = 4,
    focus = 5,
    resize = 6,
    activity = 7,
  };

  enum class layer {
    background = 0,
    sub_background = 1,
    highlight = 2,
    activity = 3
  };

  enum class state {
    enable,
    disable
  };

  enum class mode {
    singlecolumn,
    multicolumn
  };

  namespace ui {
    struct item {
    public:
      std::string name {};
      bool value {};
      uint32_t id {};
      uint32_t linked_id {};
      ekg::flags attributes {};
    public:
      ekg::rect rect_dimension {};
      ekg::rect rect_content {};
      ekg::rect rect_box {};
    };

    class abstract {
    protected:
      std::array<ekg::task*, 8> action_register {};
      std::array<ekg::gpu::sampler_t*, 4> layer_surfaces {};
    protected:
      int32_t id {};
      int32_t parent_id {};
      int32_t owner_id {};
      std::vector<int32_t> child_id_list {};

      bool alive {true};
      bool visible {true};
      bool auto_initial_dimension {true};

      ekg::flags dock_flags {};
      ekg::flags sync_flags {};
      std::string tag {};

      ekg::state state {};
      ekg::type type {ekg::type::abstract};
      ekg::level level {};
      ekg::rect rect_widget {};
      ekg::rect sync_ui {};

      int32_t scaled_height {};
    public:
      ekg::ui::abstract *unsafe_set_scaled_height_layout(int32_t scaled_size);

      /**
       * Set the element ID, which is unsafe due
       * to collisions between others IDs.
       */
      ekg::ui::abstract *unsafe_set_id(int32_t id);

      /**
       * Destroy the element childs,
       * which is unsafe due the performance overhead. 
       */
      void unsafe_destroy_childs();

      /**
       * Set the element type,
       * not recommend to use, due the object origin be incorrect with the current set.
       */
      ekg::ui::abstract *unsafe_set_type(ekg::type enum_type);

      /**
       * Make sync,
       * not recommend to use unless you know if it is necessary, due the no-effect directly.
       **/
      ekg::ui::abstract *unsafe_sync_ui(ekg::flags flags);
    public:
      /**
       * The abstract constructor.
       */
      abstract();

      /**
       * The abstract desconstructor.
       */
      ~abstract();

      /**
       * Set general-purpose use tag.
       */
      ekg::ui::abstract *set_tag(std::string_view tag);

      /**
       * Returns the general-purpose use tag.
       */
      std::string_view get_tag();

      /**
       * Add an UI element as child.
       */
      ekg::ui::abstract *add_child(int32_t id);

      /**
       * Returns the list of child elements, containing all UI element IDs.
       */
      std::vector<int32_t> &get_child_id_list();

      /**
       * Remove an element from child list.
       */
      ekg::ui::abstract *remove_child(int32_t id);

      /**
       * Returns the element ID.
       */
      int32_t get_id();

      /**
       * Set parent element ID.
       */
      ekg::ui::abstract *set_parent_id(int32_t parent_id);

      /**
       * Returns the parent element ID.
       */
      int32_t get_parent_id();

      /**
       * Set current alive state,
       * but the element is destroyed when
       * GC task runs.
       */
      ekg::ui::abstract *set_alive(bool state);

      /**
       * Returns current element alive state.
       */
      bool is_alive();

      /**
       * Set the visibility of element.
       */
      ekg::ui::abstract *set_visible(bool state);

      /**
       * Returns element visible state.
       */
      bool is_visible();

      /**
       * Destroy the element.
       */
      void destroy();

      /**
       * Set the current element state, possibles states:
       * enable, disable.
       * 
       * An eneble element is interactive by IO inputs,
       * while an disable element not.
       */
      ekg::ui::abstract *set_state(const ekg::state &_state);

      /**
       * Returns the current element state.
       */
      ekg::state get_state();

      ekg::ui::abstract *set_level(const ekg::level &_level);

      ekg::level get_level();

      ekg::type get_type();

      ekg::flags get_place_dock();

      ekg::flags &get_sync();

      /**
       * Set a task to an action, possibles actions:
       * press, release, hover, drag, focus, resize, activity.
       * 
       * Actions supports allocated PTR and PTR reference.
       */
      ekg::ui::abstract *set_task(ekg::task *p_task, ekg::action action);
      
      /**
       * Return a task from an action.
       */
      ekg::task *get_task(ekg::action action);

      /**
       * Set one drawing layer with a sampler.
       * You must initialize and fill the sampler with pixels data before to use.
       */
      ekg::ui::abstract *set_layer(ekg::gpu::sampler_t *p_sampler, ekg::layer layer);

      /**
       * Get a sampler from the element layers.
       */
      ekg::gpu::sampler_t *get_layer(ekg::layer layer);

      /**
       * Get a reference to the widget position.
       **/
      ekg::rect &widget();

      /**
       * Get a refence to the UI sync position (at latest the same as widget position but used internaly different).
       **/
      ekg::rect &ui();

      /**
       * Returns true if the UI element has parent, else False if not.
       **/
      bool has_parent() const;

      /**
       * Returns true if the UI element has children, else False if not.
       **/
      bool has_children();

      /**
       * Widgets can use this for different behaviors, as the name says when the widget
       * is created or arbitrary updated, the size is auto-calculated by the widget.
       **/
      ekg::ui::abstract *set_auto_initial_dimension(bool is_auto);

      /**
       * Get the auto initial dimension state, used internally by the widget.
       **/
      bool is_auto_initial_dimension();
    public:
      /**
       * Returns the widget ID when cast to `int32_t`.
       **/
      operator int32_t();
    };
  }
}

#endif