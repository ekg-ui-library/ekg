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

#include "ekg/draw/font_renderer.hpp"
#include "ekg/util/text.hpp"
#include "ekg/util/io.hpp"
#include "ekg/ekg.hpp"

FT_Library ekg::draw::font_renderer::ft_library {};

ekg::gpu::sampler_t *ekg::draw::font_renderer::get_atlas_texture_sampler() {
  return &this->sampler_texture;
}

float ekg::draw::font_renderer::get_text_width(std::string_view text, int32_t &lines) {
  if (text.empty() || (!this->faces[ekg::draw::font_face_type::text].was_loaded && !this->faces[ekg::draw::font_face_type::emojis].was_loaded)) {
    return 0.0f;
  }

  FT_Face ft_face {};
  FT_UInt ft_uint_previous {};
  FT_Vector ft_vector_previous_char {};
  FT_GlyphSlot ft_glyph_slot {};

  float text_width {};
  float largest_text_width {};

  uint64_t lines_count {};
  uint64_t text_size {text.size()};
  
  char32_t char32 {};
  uint8_t char8 {};
  std::string utf_string {};

  bool break_text {};
  bool r_n_break_text {};

  for (uint64_t it {}; it < text_size; it++) {
    char8 = static_cast<uint8_t>(text.at(it));
    it += ekg::utf_check_sequence(char8, char32, utf_string, text, it);
    break_text = char8 == '\n';
    if (break_text || (r_n_break_text = (char8 == '\r' && it < text_size && text.at(it + 1) == '\n'))) {
      it += static_cast<uint64_t>(r_n_break_text);
      largest_text_width = ekg_min(largest_text_width, text_width);
      text_width = 0.0f;
      lines_count++;
      continue;
    }

    switch (char32 < 256 || !this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
      case true: {
        ft_face = this->faces[ekg::draw::font_face_type::text].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::text].ft_face->glyph;
        break;
      }

      default: {
        ft_face = this->faces[ekg::draw::font_face_type::emojis].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::emojis].ft_face->glyph;
        break;
      }
    }

    if (this->ft_bool_kerning && ft_uint_previous) {
      FT_Get_Kerning(ft_face, ft_uint_previous, char32, 0, &ft_vector_previous_char);
      text_width += static_cast<float>(ft_vector_previous_char.x >> 6);
    }

    ekg::draw::glyph_char_t &char_data {this->mapped_glyph_char_data[char32]};
    ekg_validate_sample_state_and_get_wsize();

    ft_uint_previous = char32;
    text_width += this->mapped_glyph_char_data[char32].wsize;
  }

  lines = ekg_min(lines, lines_count);
  largest_text_width = ekg_min(largest_text_width, text_width);

  return largest_text_width;
}

float ekg::draw::font_renderer::get_text_width(std::string_view text) {
  if (text.empty() || (!this->faces[ekg::draw::font_face_type::text].was_loaded && !this->faces[ekg::draw::font_face_type::emojis].was_loaded)) {
    return 0.0f;
  }

  FT_Face ft_face {};
  FT_UInt ft_uint_previous {};
  FT_Vector ft_vector_previous_char {};
  FT_GlyphSlot ft_glyph_slot {};

  float text_width {};
  float largest_text_width {};
  char32_t char32 {};

  uint64_t text_size {text.size()};
  std::string utf_string {};
  uint8_t char8 {};

  bool break_text {};
  bool r_n_break_text {};

  for (uint64_t it {}; it < text_size; it++) {
    char8 = static_cast<uint8_t>(text.at(it));
    it += ekg::utf_check_sequence(char8, char32, utf_string, text, it);

    break_text = char8 == '\n';
    if (break_text || (r_n_break_text = (char8 == '\r' && it < text_size && text.at(it + 1) == '\n'))) {
      it += static_cast<uint64_t>(r_n_break_text);
      largest_text_width = ekg_min(largest_text_width, text_width);
      text_width = 0.0f;
      continue;
    }

    switch (char32 < 256 || !this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
      case true: {
        ft_face = this->faces[ekg::draw::font_face_type::text].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::text].ft_face->glyph;
        break;
      }

      default: {
        ft_face = this->faces[ekg::draw::font_face_type::emojis].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::emojis].ft_face->glyph;
        break;
      }
    }

    if (this->ft_bool_kerning && ft_uint_previous) {
      FT_Get_Kerning(ft_face, ft_uint_previous, char32, 0, &ft_vector_previous_char);
      text_width += static_cast<float>(ft_vector_previous_char.x >> 6);
    }

    ekg::draw::glyph_char_t &char_data {this->mapped_glyph_char_data[char32]};
    ekg_validate_sample_state_and_get_wsize();

    ft_uint_previous = char32;
    text_width += char_data.wsize;
  }

  largest_text_width = ekg_min(largest_text_width, text_width);
  return largest_text_width;
}

float ekg::draw::font_renderer::get_text_height() {
  return this->text_height;
}

void ekg::draw::font_renderer::set_font(std::string_view path) {
  if (!path.empty() && this->faces[ekg::draw::font_face_type::text].font_path != path) {
    this->faces[ekg::draw::font_face_type::text].font_path = path;
    this->faces[ekg::draw::font_face_type::text].was_changed = true;
    this->reload();
  }
}

void ekg::draw::font_renderer::set_font_emoji(std::string_view path) {
  if (!path.empty() && this->faces[ekg::draw::font_face_type::emojis].font_path != path) {
    this->faces[ekg::draw::font_face_type::emojis].font_path = path;
    this->faces[ekg::draw::font_face_type::emojis].was_changed = true;
    this->reload();
  }
}

void ekg::draw::font_renderer::set_size(uint32_t size) {
  if (this->font_size != size) {
    this->font_size = size;
    this->font_size_changed = true;
    this->reload();
  }
}

void ekg::draw::font_renderer::reload() {
  if (this->font_size == 0 || (this->faces[ekg::draw::font_face_type::text].font_path.empty() && this->faces[ekg::draw::font_face_type::emojis].font_path.empty())) {
    return;
  }

  if (
      (
        !this->faces[ekg::draw::font_face_type::text].font_path.empty()
        &&
        ekg::draw::reload_font_face(&this->faces[ekg::draw::font_face_type::text], this->font_size_changed, this->font_size)
      )
      ||
      (
        !this->faces[ekg::draw::font_face_type::emojis].font_path.empty()
        &&
        ekg::draw::reload_font_face(&this->faces[ekg::draw::font_face_type::emojis], this->font_size_changed, this->font_size)
      )
    ) {
    ekg::log() << "Failed to load font face, text: " << this->faces[ekg::draw::font_face_type::text].was_loaded << " , emoji: " << this->faces[ekg::draw::font_face_type::emojis].was_loaded;
    return;
  }

  this->font_size_changed = false;

  this->atlas_rect.w = 0;
  this->atlas_rect.h = 0;

  this->faces[ekg::draw::font_face_type::text].highest_glyph_size = FT_Vector {};
  this->faces[ekg::draw::font_face_type::emojis].highest_glyph_size = FT_Vector {};

  this->ft_bool_kerning = FT_HAS_KERNING(this->faces[ekg::draw::font_face_type::text].ft_face);
  this->faces[ekg::draw::font_face_type::text].ft_glyph_slot = this->faces[ekg::draw::font_face_type::text].ft_face->glyph;

  if (this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
    this->faces[ekg::draw::font_face_type::emojis].ft_glyph_slot = this->faces[ekg::draw::font_face_type::emojis].ft_face->glyph;
  }

  FT_GlyphSlot ft_glyph_slot {};
  FT_Face ft_face {};
  ekg::flags flags {};

  ekg::draw::font_face_t *p_font_face_picked {};

  for (char32_t &char32 : this->loaded_sampler_generate_list) {
    switch (char32 < 256 || !this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
      case true: {
        ft_face = this->faces[ekg::draw::font_face_type::text].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::text].ft_face->glyph;
        flags = FT_LOAD_RENDER;
        p_font_face_picked = &this->faces[ekg::draw::font_face_type::text];
        break;
      }

      default: {
        ft_face = this->faces[ekg::draw::font_face_type::emojis].ft_face;
        ft_glyph_slot = this->faces[ekg::draw::font_face_type::emojis].ft_face->glyph;
        flags = FT_LOAD_RENDER | FT_LOAD_COLOR;
        p_font_face_picked = &this->faces[ekg::draw::font_face_type::emojis];
        break;
      }
    }

    if (FT_Load_Char(ft_face, char32, flags)) {
      continue;
    }

    ekg::draw::glyph_char_t &char_data {
      this->mapped_glyph_char_data[char32]
    };

    char_data.w = static_cast<float>(ft_glyph_slot->bitmap.width);
    char_data.h = static_cast<float>(ft_glyph_slot->bitmap.rows);

    char_data.left = static_cast<float>(ft_glyph_slot->bitmap_left);
    char_data.top = static_cast<float>(ft_glyph_slot->bitmap_top);
    char_data.wsize = static_cast<float>(static_cast<int32_t>(ft_glyph_slot->advance.x >> 6));

    this->atlas_rect.w += static_cast<int32_t>(char_data.w);
    this->atlas_rect.h = ekg_min(this->atlas_rect.h, static_cast<int32_t>(char_data.h));

    p_font_face_picked->highest_glyph_size.x = ekg_min(
      p_font_face_picked->highest_glyph_size.x,
      static_cast<int32_t>(char_data.w)
    );

    p_font_face_picked->highest_glyph_size.y = ekg_min(
      p_font_face_picked->highest_glyph_size.y,
      static_cast<int32_t>(char_data.h)
    );
  }

  this->text_height = static_cast<float>(this->font_size);
  this->offset_text_height = static_cast<int32_t>(this->text_height / 6) / 2;

  ekg::p_core->p_gpu_api->gen_font_atlas_and_map_glyph(
    &this->sampler_texture,
    &this->faces[ekg::draw::font_face_type::text],
    &this->faces[ekg::draw::font_face_type::emojis],
    nullptr, // must impl kanjis
    this->atlas_rect,
    this->loaded_sampler_generate_list,
    this->mapped_glyph_char_data,
    this->non_swizzlable_range
  );
}

void ekg::draw::font_renderer::bind_allocator(ekg::gpu::allocator *p_allocator_bind) {
  this->p_allocator = p_allocator_bind;
}

void ekg::draw::font_renderer::blit(
  std::string_view text,
  float x,
  float y,
  const ekg::vec4_t<float> &color
) {
  if (this->p_allocator == nullptr || color.w < 0.1f || text.empty()) {
    return;
  }

  x = static_cast<float>(static_cast<int32_t>(x));
  y = static_cast<float>(static_cast<int32_t>(y - this->offset_text_height));

  ekg::gpu::data_t &data {this->p_allocator->bind_current_data()};

  data.buffer_content[0] = x;
  data.buffer_content[1] = y;
  data.buffer_content[2] = static_cast<float>(-this->non_swizzlable_range);
  data.buffer_content[3] = static_cast<float>(ekg::gpu::allocator::concave);

  data.buffer_content[4] = color.x;
  data.buffer_content[5] = color.y;
  data.buffer_content[6] = color.z;
  data.buffer_content[7] = color.w;

  ekg::rect_t<float> vertices {};
  ekg::rect_t<float> coordinates {};

  x = 0.0f;
  y = 0.0f;

  data.factor = 1;
  char32_t char32 {};
  uint8_t char8 {};

  std::string utf_string {};
  uint64_t text_size {text.size()};

  bool break_text {};
  bool r_n_break_text {};

  FT_Face ft_face {};
  FT_Vector ft_vector_previous_char {};

  for (uint64_t it {}; it < text_size; it++) {
    char8 = static_cast<uint8_t>(text.at(it));
    it += ekg::utf_check_sequence(char8, char32, utf_string, text, it);

    break_text = char8 == '\n';
    if (
        break_text
        ||
        (
          r_n_break_text = (
            char8 == '\r' && it < text_size && text.at(it + 1) == '\n'
          )
        )
      ) {
      ekg::draw::glyph_char_t &char_data {this->mapped_glyph_char_data[char32]};

      it += static_cast<uint64_t>(r_n_break_text);
      data.factor += ekg_generate_factor_hash(y, char32, char_data.x);

      y += this->text_height;
      x = 0.0f;
      continue;
    }

    if (this->ft_bool_kerning && ft_uint_previous) {
      switch (char32 < 256 || !this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
        case true: {
          ft_face = this->faces[ekg::draw::font_face_type::text].ft_face;
          break;
        }

        default: {
          ft_face = this->faces[ekg::draw::font_face_type::emojis].ft_face;
          break;
        }
      }

      FT_Get_Kerning(ft_face, ft_uint_previous, char32, 0, &ft_vector_previous_char);
      x += static_cast<float>(ft_vector_previous_char.x >> 6);
    }

    ekg::draw::glyph_char_t &char_data {this->mapped_glyph_char_data[char32]};

    if (!char_data.was_sampled) {
      this->loaded_sampler_generate_list.emplace_back(char32);
      char_data.was_sampled = true;
    }

    vertices.x = x + char_data.left;
    vertices.y = y + this->atlas_rect.h - char_data.top;

    vertices.w = char_data.w;
    vertices.h = char_data.h;

    coordinates.x = char_data.x;
    coordinates.w = vertices.w / this->atlas_rect.w;
    coordinates.h = vertices.h / this->atlas_rect.h;

    this->p_allocator->push_back_geometry(
      vertices.x,
      vertices.y,
      coordinates.x,
      coordinates.y
    );

    this->p_allocator->push_back_geometry(
      vertices.x,
      vertices.y + vertices.h,
      coordinates.x,
      coordinates.y + coordinates.h
    );

    this->p_allocator->push_back_geometry(
      vertices.x + vertices.w,
      vertices.y + vertices.h,
      coordinates.x + coordinates.w,
      coordinates.y + coordinates.h
    );

    this->p_allocator->push_back_geometry(
      vertices.x + vertices.w,
      vertices.y + vertices.h,
      coordinates.x + coordinates.w,
      coordinates.y + coordinates.h
    );

    this->p_allocator->push_back_geometry(
      vertices.x + vertices.w,
      vertices.y,
      coordinates.x + coordinates.w,
      coordinates.y
    );

    this->p_allocator->push_back_geometry(
      vertices.x,
      vertices.y,
      coordinates.x,
      coordinates.y
    );

    x += char_data.wsize;
    ft_uint_previous = char32;

    data.factor += ekg_generate_factor_hash(x, char32, char_data.x);
  }

  this->flush();
  this->p_allocator->bind_texture(&this->sampler_texture);
  this->p_allocator->dispatch();
}

void ekg::draw::font_renderer::flush() {
  uint64_t size {this->loaded_sampler_generate_list.size()};
  if (this->last_sampler_generate_list_size != size) {
    ekg::log() << "Sampler updated from-to: " << this->last_sampler_generate_list_size << " " << size;

    this->reload();
    this->last_sampler_generate_list_size = size;
    ekg::ui::redraw = true;
  }
}

void ekg::draw::font_renderer::init() {
  if (this->was_initialized) {
    return;
  }

  this->was_initialized = true;

  this->loaded_sampler_generate_list.resize(256);
  for (char32_t char32 {}; char32 < 256; char32++) {
    this->loaded_sampler_generate_list.at(char32) = char32;
  }

  ekg::log() << "Initializing 256 default chars!";
}

void ekg::draw::font_renderer::quit() {
  if (this->faces[ekg::draw::font_face_type::text].was_loaded) {
    FT_Done_Face(this->faces[ekg::draw::font_face_type::text].ft_face);
    this->faces[ekg::draw::font_face_type::text].was_loaded = false;
  }

  if (this->faces[ekg::draw::font_face_type::emojis].was_loaded) {
    FT_Done_Face(this->faces[ekg::draw::font_face_type::emojis].ft_face);
    this->faces[ekg::draw::font_face_type::emojis].was_loaded = false;
  }
}
