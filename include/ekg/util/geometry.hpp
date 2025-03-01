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

#ifndef EKG_UTIL_GEOMETRY_H
#define EKG_UTIL_GEOMETRY_H

#include <iostream>
#include <cstdint>
#include <cfloat>
#include <sstream>
#include <cmath>

#define ekg_equals_float(x, y) ((fabsf((x) - (y))) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))
#define ekg_pixel_div_2        (0.500000f)
#define ekg_pixel              (1.000000f)
#define ekg_pi                 (3.141592653589793238462643383279502884)

#define ekg_no_update_placement (0.0f)

/*
 * The STL std:::min and std::max implementation,
 * are not suitable to perform the required comparions inside of EKG
 * without making the code confuse.
 * 
 * Max STL impl:
 * ```
 * (a < b) ? b : a;
 * ```
 *
 * Max EKG impl:
 * ```
 * (a > b) ? b : a; 
 * ```
 */
#define ekg_min(a, b)      ((a < b) ? b : a)
#define ekg_max(a, b)      ((a > b) ? b : a)
#define ekg_clamp(a, b, c) ((a < b) ? b : ((a > c) ? c : a))

namespace ekg {
  enum dock {
    none   = 2 << 1,
    free   = 2 << 2,
    top    = 2 << 3,
    bottom = 2 << 4,
    right  = 2 << 5,
    left   = 2 << 6,
    center = 2 << 7,
    full   = 2 << 8,
    next   = 2 << 9,
    fill   = 2 << 10,
    resize = 2 << 11,
    bind   = 2 << 12
  };

  /*
   * Enum linked to dock.
   */
  enum axis {
    vertical = 2 << 13,
    horizontal = 2 << 14
  };

  struct vec2 {
  public:
    float x {};
    float y {};
  };

  ekg::vec2 operator/(const ekg::vec2 &, float);

  struct vec3 {
  public:
    float x {};
    float y {};
    float z {};
  };

  struct vec4 {
  public:
    float x {};
    float y {};
    float z {};
    float w {};
  };

  struct rect {
  public:
    float x {};
    float y {};
    float w {};
    float h {};
  public:
    /**
     * Logs the dimensions.
     **/
    operator std::string() {
      std::ostringstream buffer {};

      buffer <<
        " x: " << this->x <<
        " y: " << this->y <<
        " w: " << this->w <<
        " h: " << this->h;

      return buffer.str();
    }
  };

  bool operator==(const ekg::rect &, const ekg::rect &);

  bool operator!=(const ekg::rect &, const ekg::rect &);

  bool operator==(const ekg::rect &l, const ekg::vec4 &r);

  bool operator!=(const ekg::rect &l, const ekg::vec4 &r);

  ekg::rect operator-(const ekg::rect &l, const ekg::rect &r);

  ekg::rect operator+(const ekg::rect &l, const ekg::rect &r);

  ekg::rect operator-(const ekg::rect &l, const ekg::vec4 &r);

  ekg::rect operator+(const ekg::rect &l, const ekg::vec4 &r);

  ekg::vec4 operator-(const ekg::vec4 &l, const ekg::vec4 &r);

  ekg::vec4 operator+(const ekg::vec4 &l, const ekg::vec4 &r);

  ekg::vec4 operator-(const ekg::vec4 &l, const ekg::rect &r);

  ekg::vec4 operator+(const ekg::vec4 &l, const ekg::rect &r);

  ekg::vec4 operator/(const ekg::vec4 &l, float r);

  struct docker {
  public:
    ekg::rect left {};
    ekg::rect right {};
    ekg::rect top {};
    ekg::rect bottom {};
    ekg::rect center {};
    ekg::rect rect {};
  };

  struct dock_rect {
  public:
    ekg::rect *p_rect {};
    uint16_t dock {};
  };

  struct placement {
  public:
    ekg::rect rect {};
    ekg::rect rect_extent {};

    ekg::rect rect_text {};
    uint16_t text_dock_flags {};
    float offset {};
  };

  /**
   * Get a index n-size by scroll amount.
   * Note: You must have a compatible scroll-size with the n-index-size.
   **/
  uint64_t get_index_by_scroll(float scroll, float dimension, uint64_t size);

  /**
   * Get a index n-size by scroll amount.
   * Note: You must have a compatible scroll-size with the n-index-size.
   **/
  uint64_t get_index_by_normalized_scroll(float normalized, uint64_t size);

  bool rect_collide_rect(const ekg::rect &a, const ekg::rect &b);
  bool rect_collide_vec(const ekg::rect &rect, const ekg::vec4 &vec);
  bool rect_collide_vec_precisely(const ekg::rect &rect, const ekg::vec4 &vec);

  void set_rect_clamped(ekg::rect &rect, float min_size);
  void set_dock_scaled(const ekg::rect &rect, const ekg::vec2 &offset, ekg::docker &docker);
  void ortho(float *p_matrix, float left, float right, float bottom, float top);

  float find_min_offset(float width, float offset);
  float smooth(float duration, uint64_t ticks);
  float lerp(float a, float b, float dt);

  ekg::vec4 color(int32_t r, int32_t g, int32_t b, int32_t a);
  int32_t find_collide_dock(ekg::docker &docker, uint16_t flags, const ekg::vec4 &vec);
}

#endif
