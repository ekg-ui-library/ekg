#ifndef EKG_MATH_GEOMETRY_HPP
#define EKG_MATH_GEOMETRY_HPP

#include <cstdint>

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

  enum axis {
    vertical   = 2 << 13,
    horizontal = 2 << 14
  };

  template<typename t>
  struct vec2_t {
    union {
      struct {
        t x {};
        t y {};
      };
    };
  public:
    inline vec2_t() = default;

    inline vec2_t(t _x, _y) {
      this->x = _x;
      this->y = _y;
    }

    // TODO: add non-useless vector 2 properties operators
  
    template<typename t>
    operator ekg::vec2_t<t>() {
      return ekg::vec2_t<t>{
        static_cast<t>(this->x),
        static_cast<t>(this->y)
      };
    }
  };

  template<typename t>
  struct vec3_t {
    union {
      struct {
        t x {};
        t y {};
        t z {};
      };
    };
  public:
    inline vec3_t() = default;

    inline vec3_t(t _x, t _y, t _z) {
      this->x = _x;
      this->y = _y;
      this->z = _z;
    }

    // TODO: add non-useless vector 3 properties operators

    template<typename t>
    operator ekg::vec3_t<t>() {
      return ekg::vec3_t<t>{
        static_cast<t>(this->x),
        static_cast<t>(this->y),
        static_cast<t>(this->z)
      };
    }
  };

  template<typename t>
  struct vec4_t {
    union {
      struct {
        t x {};
        t y {};
        t z {};
        t w {};
      };
    };
  public:
    inline vec4_t() = default;

    inline vec4_t(t _x, t _y, t _z, t _w) {
      this->x = _x;
      this->y = _y;
      this->z = _z;
      this->w = _w;
    }

    // TODO: add non-useless vector 4 properties operators

    template<typename t>
    operator ekg::vec4_t<t>() {
      return ekg::vec4_t<t>{
        static_cast<t>(this->x),
        static_cast<t>(this->y),
        static_cast<t>(this->z),
        static_cast<t>(this->w)
      };
    }

    template<typename s>
    ekg::vec4_t<t> operator / (s div_by) {
      return ekg::vec4_t<t> {
        this->x / div_by,
        this->y / div_by,
        this->z / div_by,
        this->w / div_by
      };
    }

    template<typename s>
    ekg::vec4_t<t> operator / (ekg::vec4_t<s> div_by) {
      return ekg::vec4_t<t> {
        this->x / div_by.x,
        this->y / div_by.y,
        this->z / div_by.z,
        this->w / div_by.w
      };
    }
  };

  template<typename t>
  struct rect_t {
  public: 
    union {
      struct {
        t x {};
        t y {};
        t w {};
        t h {};
      };
    };
  public:
    inline rect_t() = default;

    inline rect_t(t _x, t _y, t _w, t _h) {
      this->x = _x;
      this->y = _y;
      this->w = _w;
      this->h = _h;
    }

    // TODO: add non-useless rect (aka vector 4 properties) operators

    template<typename t>
    operator ekg::rect_t<t>() {
      return ekg::rect_t<t>{
        static_cast<t>(this->x),
        static_cast<t>(this->y),
        static_cast<t>(this->w),
        static_cast<t>(this->h)
      };
    }
    
    template<typename t>
    operator ekg::vec4_t<t>() {
      return ekg::vec4_t<t>{
        static_cast<t>(this->x),
        static_cast<t>(this->y),
        static_cast<t>(this->w),
        static_cast<t>(this->h)
      };
    }
  };

  struct rect_descriptor_t {
  public:
    ekg::rect_t<float> *p_rect {};
    ekg::flags_t flags {};
  };

  template<typename t>
  ekg::vec4_t<float> color(
    t r,
    t g,
    t b,
    t a
  ) {
    return ekg::vec4_t<t>(r, g, b, a) / 255;
  }

  void ortho(
    float *p_mat4x4,
    float left,
    float right,
    float top,
    float bottom
  );
}

#endif
