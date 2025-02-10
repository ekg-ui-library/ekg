#include "ekg/io/design.hpp"

std::map<std::string_view, ekg::theme_t> &ekg::themes() {
  return ekg::core->service_theme.get_theme_map();
}

ekg::theme_t ekg::theme(std::string_view name) {
  if (name.empty()) {
    return ekg::core->service_theme.get_current_theme();
  }

  return ekg::themes()[name];
}

ekg::theme_t &ekg::theme(ekg::theme_t theme) {
  return ekg::core->service_theme.add(theme.name, theme);
}

ekg::flags_t ekg::set_current_theme(std::string_view name) {
  return ekg::core->service_theme.set_current_theme(name);
}
