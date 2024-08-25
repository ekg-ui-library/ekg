#include "ekg/layout/docknize.hpp"
#include "ekg/ui/display.hpp"
#include "ekg/ekg.hpp"
#include "ekg/ui/frame/ui_frame_widget.hpp"

float ekg::layout::offset {2.0f};
ekg::rect ekg::layout::extent::h {};
ekg::rect ekg::layout::extent::v {};

void ekg::layout::extentnize(
  float &extent,
  ekg::ui::abstract_widget *p_widget,
  ekg::flags flag_ok,
  ekg::flags flag_stop,
  int64_t &begin_and_count,
  ekg::axis axis
) {
  extent = 0.0f;
  if (p_widget == nullptr) {
    return;
  }

  switch (axis) {
    case ekg::axis::horizontal: {
      ekg::flags flags {};
      int32_t ids {};
      int64_t flag_ok_count {};
      int64_t it {begin_and_count};

      if (
          begin_and_count > static_cast<int64_t>(ekg::layout::extent::h.x) &&
          begin_and_count < static_cast<int64_t>(ekg::layout::extent::h.y)
        ) {
        begin_and_count = static_cast<int64_t>(ekg::layout::extent::h.h);
        extent = ekg::layout::extent::h.w;
        return;
      }

      ekg::layout::extent::h.x = static_cast<float>(it);
      ekg::ui::abstract_widget *p_widgets {};
      std::vector<int32_t> &child_id_list {p_widget->p_data->get_child_id_list()};

      uint64_t size {child_id_list.size()};
      uint64_t latest_index {size - (!child_id_list.empty())};

      bool is_scrollbar {};
      bool is_last_index {};
      bool is_ok_flag {};

      /**
       * The last index does not check if contains a next flag,
       * so it is needed to brute-check to stop at end of index. 
       *
       * The extent data store the previous bounding indices,
       * in simply words, prevent useless iteration.
       *
       * The min offset is added for extent, because we need count
       * the offset position when split the fill width, but the
       * last extent space is not necessary, so we need to subtract.
       **/
      for (it = it; it < size; it++) {
        if ((p_widgets = ekg::core->get_fast_widget_by_id(child_id_list.at(it))) == nullptr) {
          continue;
        }

        flags = p_widgets->p_data->get_place_dock();
        is_scrollbar = p_widgets->p_data->get_type() == ekg::type::scrollbar;
        is_last_index = it == latest_index;

        if (
            (ekg_bitwise_contains(flags, flag_stop) && it != begin_and_count) || is_last_index || is_scrollbar
          ) {
          extent -= ekg::layout::offset;
          flag_ok_count += (
            (is_ok_flag = (!ekg_bitwise_contains(flags, flag_stop) && ekg_bitwise_contains(flags, flag_ok) && is_last_index))
          );

          /**
           * Basically if the container/frame mother ends with any non flag ok (ekg::dock::fill)
           * it MUST add the width size to extend.
           * But the point is the scrollbar, he is not docknized here, then just bypass with `!is_scrollbar`.
           *
           * :blush:
           **/
          extent += (
            (p_widgets->dimension.w + ekg::layout::offset) * (is_last_index && !ekg_bitwise_contains(flags, flag_ok) && !is_scrollbar)
          );

          ekg::layout::extent::h.y = static_cast<float>(it + is_last_index);
          ekg::layout::extent::h.w = extent;
          ekg::layout::extent::h.h = static_cast<float>(flag_ok_count == 0 ? 1 : flag_ok_count);
          break;
        }

        if (ekg_bitwise_contains(flags, flag_ok)) {
          flag_ok_count++;
          continue;
        }

        extent += p_widgets->dimension.w + ekg::layout::offset;
      }

      begin_and_count = flag_ok_count == 0 ? 1 : flag_ok_count;
      break;
    }

    case ekg::axis::vertical: {
      break;
    }
  }
}

void ekg::layout::docknize(ekg::ui::abstract_widget *p_widget_parent) {
  if (p_widget_parent == nullptr || p_widget_parent->p_data == nullptr) {
    return;
  }

  ekg::type type {p_widget_parent->p_data->get_type()};
  bool is_group {type == ekg::type::frame};
  ekg::rect &abs_parent_rect {p_widget_parent->get_abs_rect()};

  if (!is_group || abs_parent_rect.w == 0 || abs_parent_rect.h == 0) {
    return;
  }

  if (p_widget_parent->is_targeting_absolute_parent) {
    p_widget_parent->is_targeting_absolute_parent = false;

    if (p_widget_parent->p_abs_parent_widget) {
      ekg::layout::docknize(p_widget_parent->p_abs_parent_widget);
    } else {
      ekg::layout::docknize(p_widget_parent);
    }

    return;
  }

  float group_top_offset {ekg::layout::offset};
  float initial_offset {};
 
  bool has_scroll_embedded {};
  bool is_vertical_enabled {};

  ekg::rect group_rect {p_widget_parent->dimension};

  switch (type) {
    case ekg::type::frame: {
      ekg::service::theme_scheme_t &theme_scheme {ekg::current_theme_scheme()};
      ekg::ui::frame_widget *p_frame {(ekg::ui::frame_widget *) p_widget_parent};

      initial_offset = static_cast<float>(theme_scheme.scrollbar_pixel_thickness);
      has_scroll_embedded = p_frame->p_scroll_embedded != nullptr;

      if (has_scroll_embedded) {
        p_frame->p_scroll_embedded->check_axis_states();
        is_vertical_enabled = p_frame->p_scroll_embedded->is_vertical_enabled;
        initial_offset *= static_cast<float>(!theme_scheme.symmetric_layout);

        group_rect.w -= initial_offset * static_cast<float>(is_vertical_enabled);
        group_rect.h -= initial_offset * static_cast<float>(p_frame->p_scroll_embedded->is_horizontal_enabled);
      }

      initial_offset = static_cast<float>(theme_scheme.scrollbar_pixel_thickness) * static_cast<float>(theme_scheme.symmetric_layout);
      break;
    }

    default: {
      break;
    }
  }

  float total_offset {(initial_offset + ekg::layout::offset) * 2.0f};

  group_rect.w -= total_offset;
  group_rect.h -= total_offset;

  ekg::ui::abstract_widget *p_widgets {};
  ekg::flags flags {};
  int64_t it {};
  float dimensional_extent {};
  int64_t count {};

  ekg::rect widget_rect {};
  ekg::rect bottom_rect {};
  ekg::rect top_rect {ekg::layout::offset + initial_offset, ekg::layout::offset + initial_offset, 0.0f, 0.0f};
  ekg::rect prev_widget_layout {};

  ekg::flags prev_flags {};
  bool should_reload_widget {};
  bool skip_widget {};
  float max_previous_height {};

  ekg::layout::extent::h = {};
  ekg::rect h_extent_backup {};
  ekg::rect v_extent_backup {};

  for (int32_t &ids: p_widget_parent->p_data->get_child_id_list()) {
    if (ids == 0 || (p_widgets = ekg::core->get_fast_widget_by_id(ids)) == nullptr) {
      continue;
    }

    ekg::rect &layout {p_widgets->dimension};
    flags = p_widgets->p_data->get_place_dock();

    // @TODO Prevent useless scrolling reload.
    p_widgets->on_reload();
    type = p_widgets->p_data->get_type();

    if (type == ekg::type::scrollbar) {
      it++;
      continue;
    }

    if (ekg_bitwise_contains(flags, ekg::dock::fill)) {
      if (ekg_bitwise_contains(flags, ekg::dock::next)) {
        top_rect.h += max_previous_height + ekg::layout::offset;
        top_rect.w = 0.0f;
        max_previous_height = 0.0f;
      }

      layout.x = top_rect.x + top_rect.w;
      layout.y = top_rect.y + top_rect.h;

      count = it;
      ekg::layout::extentnize(
        dimensional_extent,
        p_widget_parent,
        ekg::dock::fill,
        ekg::dock::next,
        count,
        ekg::axis::horizontal
      );

      dimensional_extent = ekg_min(
        ekg_layout_get_dimensional_extent(
          group_rect.w,
          dimensional_extent,
          ekg::layout::offset,
          count
        ),
        p_widgets->min_size.x
      );

      top_rect.w += dimensional_extent + ekg::layout::offset;
      layout.w = dimensional_extent;
      should_reload_widget = true;
    } else if (ekg_bitwise_contains(flags, ekg::dock::next)) {
      top_rect.h += max_previous_height + ekg::layout::offset;
      top_rect.w = 0.0f;

      layout.x = top_rect.x;
      layout.y = top_rect.y + top_rect.h;
      top_rect.w += layout.w + ekg::layout::offset;

      count = it;
      ekg::layout::extentnize(
        dimensional_extent,
        p_widget_parent,
        ekg::dock::fill,
        ekg::dock::next,
        count,
        ekg::axis::horizontal
      );

      max_previous_height = 0.0f;
    } else if (flags == ekg::dock::none) {
      layout.x = top_rect.x + top_rect.w;
      layout.y = top_rect.y + top_rect.h;
      top_rect.w += layout.w + ekg::layout::offset;

      count = it;
      ekg::layout::extentnize(
        dimensional_extent,
        p_widget_parent,
        ekg::dock::fill,
        ekg::dock::next,
        count,
        ekg::axis::horizontal
      );
    }

    if (ekg_bitwise_contains(flags, ekg::dock::resize)) {
      should_reload_widget = true;
    }

    max_previous_height = layout.h > max_previous_height ? layout.h : max_previous_height;
    if (should_reload_widget) {
      p_widgets->on_reload();
    }

    h_extent_backup = ekg::layout::extent::h;

    if (p_widgets->p_data->has_children()) {
      ekg::layout::docknize(p_widgets);
    }

    ekg::layout::extent::h = h_extent_backup;
    prev_widget_layout = layout;
    prev_flags = flags;
    it++;
  }

  if (has_scroll_embedded && !is_vertical_enabled && type == ekg::type::frame) {
    ekg::ui::frame_widget *p_frame {(ekg::ui::frame_widget *) p_widget_parent};
    has_scroll_embedded = p_frame->p_scroll_embedded != nullptr;

    if (has_scroll_embedded && !is_vertical_enabled && p_frame->p_scroll_embedded->is_vertical_enabled) {
      ekg::layout::docknize(p_widget_parent);
    }
  }
}

void ekg::layout::mask::extentnize(
  float &extent,
  ekg::flags flag_ok,
  ekg::flags flag_stop,
  int64_t &begin_and_count,
  ekg::axis axis
) {
  extent = 0.0f;
  switch (axis) {
    case ekg::axis::horizontal: {
      ekg::flags flags {};
      int32_t ids {};
      int64_t flag_ok_count {};
      int64_t it {begin_and_count};

      if (
          begin_and_count > static_cast<int64_t>(ekg::layout::extent::h.x) &&
          begin_and_count < static_cast<int64_t>(ekg::layout::extent::h.y)
        ) {
        begin_and_count = static_cast<int64_t>(ekg::layout::extent::h.h);
        extent = ekg::layout::extent::h.w;
        return;
      }

      ekg::layout::extent::h.x = static_cast<float>(it);
      uint64_t size {this->dock_rect_list.size()};
      uint64_t latest_index {size - (!this->dock_rect_list.empty())};

      bool is_last_index {};
      bool is_ok_flag {};

      /**
       * The last index does not check if contains a next flag,
       * so it is needed to brute-check to stop at end of index. 
       *
       * The extent data store the previous bounding indices,
       * in simply words, prevent useless iteration.
       *
       * The min offset is added for extent, because we need count
       * the offset position when split the fill width, but the
       * last extent space is not necessary, so we need to subtract.
       **/
      for (it = it; it < size; it++) {
        ekg::layout::mask::rect &dock_rect {this->dock_rect_list.at(it)};
        if (dock_rect.p_rect == nullptr) {
          continue;
        }

        is_last_index = it == latest_index;

        if (
            (ekg_bitwise_contains(dock_rect.flags, flag_stop) && it != begin_and_count) || is_last_index
          ) {
          extent -= this->offset.x;
          flag_ok_count += (
            (is_ok_flag = (!ekg_bitwise_contains(dock_rect.flags, flag_stop) && ekg_bitwise_contains(dock_rect.flags, flag_ok) && is_last_index))
          );

          /**
           * Basically if the container/frame mother ends with any non flag ok (ekg::dock::fill)
           * it MUST add the width size to extend.
           *
           * :blush:
           **/
          extent += (
            (dock_rect.p_rect->w + this->offset.x) * (is_last_index && !ekg_bitwise_contains(dock_rect.flags, flag_ok))
          );

          ekg::layout::extent::h.y = static_cast<float>(it + is_last_index);
          ekg::layout::extent::h.w = extent;
          ekg::layout::extent::h.h = static_cast<float>(flag_ok_count == 0 ? 1 : flag_ok_count);
          break;
        }

        if (ekg_bitwise_contains(dock_rect.flags, flag_ok)) {
          flag_ok_count++;
          continue;
        }

        extent += dock_rect.p_rect->w + this->offset.x;
      }

      begin_and_count = flag_ok_count == 0 ? 1 : flag_ok_count;
      break;
    }

    case ekg::axis::vertical: {
      break;
    }
  }
}

void ekg::layout::mask::preset(const ekg::vec3 &mask_offset, ekg::axis mask_axis, float initial_respective_size) {
  this->axis = mask_axis;
  this->offset = mask_offset;
  this->respective_all = initial_respective_size;
}

void ekg::layout::mask::insert(const ekg::layout::mask::rect &dock_rect) {
  if (
      (static_cast<int32_t>(dock_rect.p_rect->w == 0) || static_cast<int32_t>(dock_rect.p_rect->h) == 0) ||
      (ekg_bitwise_contains(dock_rect.flags, ekg::dock::none))
    ) {
    return;
  }

  this->dock_rect_list.push_back(dock_rect);
}

void ekg::layout::mask::docknize() {
  float left_or_right {};
  float centered_dimension {this->offset.z / 2};
  float opposite {};
  float uniform {};
  float clamped_offset {};

  int64_t count {};
  float dimensional_extent {};

  switch (this->axis) {
  case ekg::axis::horizontal:
    if (this->dock_rect_list.empty()) {
      this->mask.w = this->respective_all;
      this->mask.h = this->offset.z;
      return;
    }

    this->mask.w = this->offset.x;
    this->mask.h = this->offset.z;

    for (uint64_t it {}; it < this->dock_rect_list.size(); it++) {
      ekg::layout::mask::rect &dock_rect {this->dock_rect_list.at(it)};
      if (dock_rect.p_rect == nullptr) {
        continue;
      }

      clamped_offset = ekg_clamp(
        (dock_rect.p_rect->h + this->offset.y) - this->offset.z,
        0.0f,
        this->offset.y
      );

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::fill)) {
        dock_rect.p_rect->x = this->mask.w;

        count = it;
        this->extentnize(
          dimensional_extent,
          ekg::dock::fill,
          ekg::dock::none,
          count,
          ekg::axis::horizontal
        );

        dimensional_extent = ekg_min(
          ekg_layout_get_dimensional_extent(
            this->respective_all,
            dimensional_extent,
            this->offset.x,
            count
          ),
          1.0f
        );

        dock_rect.p_rect->w = dimensional_extent;
        opposite = 0;
        uniform = dock_rect.p_rect->w + this->offset.x;
        this->mask.w += dimensional_extent + this->offset.x + opposite;

        std::cout << "dimensional_extent: " << dimensional_extent << std::endl;
      }

      left_or_right = (
        ekg_bitwise_contains(dock_rect.flags, ekg::dock::left) ||
        ekg_bitwise_contains(dock_rect.flags, ekg::dock::right)
      );

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::center) && !left_or_right) {
        dock_rect.p_rect->x = (this->respective_all / 2) - (dock_rect.p_rect->w / 2);
        dock_rect.p_rect->y = centered_dimension - (dock_rect.p_rect->h / 2);
        this->mask.w += dock_rect.p_rect->w + this->offset.x;
      } else if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::center)) {
        dock_rect.p_rect->y = centered_dimension - (dock_rect.p_rect->h / 2);
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::left)) {
        if (static_cast<int32_t>(opposite) != 0) {
          this->mask.w -= opposite;
        }

        dock_rect.p_rect->x = this->mask.w;
        this->mask.w += dock_rect.p_rect->w + this->offset.x + opposite;
        opposite = 0;
        uniform = dock_rect.p_rect->w + this->offset.x;
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::right)) {
        if (static_cast<int32_t>(uniform) != 0) {
          this->mask.w -= uniform;
        }

        this->mask.w += dock_rect.p_rect->w;
        dock_rect.p_rect->x = this->respective_all - this->mask.w;
        this->mask.w += uniform + this->offset.x;
        opposite = dock_rect.p_rect->w + this->offset.x;
        uniform = 0;
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::top)) {
        dock_rect.p_rect->y = clamped_offset;
      } else if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::bottom)) {
        dock_rect.p_rect->y = this->offset.z - clamped_offset - dock_rect.p_rect->h;
      }
    }

    this->mask.w = ekg_min(this->respective_all, this->mask.w);
    break;
  case ekg::axis::vertical:
    if (this->dock_rect_list.empty()) {
      this->mask.w = this->offset.z;
      this->mask.h = this->respective_all;
      return;
    }

    this->mask.w = this->offset.z;
    this->mask.h = this->offset.y;

    for (uint64_t it {}; it < this->dock_rect_list.size(); it++) {
      ekg::layout::mask::rect &dock_rect {this->dock_rect_list.at(it)};

      if (dock_rect.p_rect == nullptr) {
        continue;
      }

      clamped_offset = ekg_clamp(
        (dock_rect.p_rect->w + this->offset.x) - this->offset.z,
        0.0f,
        this->offset.x
      );

      left_or_right = (
        ekg_bitwise_contains(dock_rect.flags, ekg::dock::left) ||
        ekg_bitwise_contains(dock_rect.flags, ekg::dock::right)
      );

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::center) && !left_or_right) {
        dock_rect.p_rect->y = (this->respective_center) + (dock_rect.p_rect->h / 2);
        dock_rect.p_rect->x = centered_dimension - (dock_rect.p_rect->w / 2);
      } else if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::center)) {
        dock_rect.p_rect->x = centered_dimension - (dock_rect.p_rect->w / 2);
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::top)) {
        if (static_cast<int32_t>(opposite) != 0) {
          this->mask.h -= opposite;
        }

        dock_rect.p_rect->y = this->mask.h;
        this->mask.h += dock_rect.p_rect->h + this->offset.y + opposite;
        opposite = 0;
        uniform = dock_rect.p_rect->h + this->offset.y;
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::bottom)) {
        if (static_cast<int32_t>(uniform) != 0) {
          this->mask.h -= uniform;
        }

        this->mask.h += dock_rect.p_rect->h;
        dock_rect.p_rect->y = this->respective_all - this->mask.h;
        this->mask.h += uniform + this->offset.y;
        opposite = dock_rect.p_rect->h + this->offset.y;
        uniform = 0;
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::left)) {
        dock_rect.p_rect->x = clamped_offset;
      }

      if (ekg_bitwise_contains(dock_rect.flags, ekg::dock::right)) {
        dock_rect.p_rect->x = this->offset.z - clamped_offset - dock_rect.p_rect->w;
      }
    }

    this->mask.h = ekg_min(this->respective_all, this->mask.h);
    break;
  }

  this->dock_rect_list.clear();
}

ekg::rect &ekg::layout::mask::get_rect() {
  return this->mask;
}

float ekg::layout::mask::get_respective_size() {
  if (this->dock_rect_list.empty()) {
    return 0;
  }

  float respective_size {(this->axis == ekg::axis::horizontal) ? this->offset.x : this->offset.y};
  float respective_center_size {respective_size};
  float size {};

  int32_t only_center_count {};

  for (ekg::layout::mask::rect &dock_rect : this->dock_rect_list) {
    if (dock_rect.p_rect == nullptr) {
      continue;
    }

    size = (
      this->axis == ekg::axis::horizontal ?
      (dock_rect.p_rect->w + this->offset.x) : (dock_rect.p_rect->h + this->offset.y)
    );

    if (
        ekg_bitwise_contains(dock_rect.flags, ekg::dock::center) &&
        !(ekg_bitwise_contains(dock_rect.flags, ekg::dock::left) || ekg_bitwise_contains(dock_rect.flags, ekg::dock::right))
      ) {
      respective_center_size += size;
      only_center_count++;
    }

    respective_size += size;
  }

  this->respective_center = (only_center_count != 0 ? (respective_center_size / only_center_count) : 0.0f);
  this->respective_all = ekg_min(this->respective_all, respective_size);

  return this->respective_all;
}