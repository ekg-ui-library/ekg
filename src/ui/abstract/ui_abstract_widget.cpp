#include "ekg/ui/abstract/ui_abstract_widget.hpp"
#include "ekg/util/util_event.hpp"

ekg::ui::abstract_widget::abstract_widget() {
    this->parent = &this->ghost;
}

ekg::ui::abstract_widget::~abstract_widget() {

}

void ekg::ui::abstract_widget::destroy() {

}

void ekg::ui::abstract_widget::on_reload() {
    ekg::dispatch(ekg::env::redraw);
}

void ekg::ui::abstract_widget::on_pre_event(SDL_Event &sdl_event) {
    if (ekg::input::pressed() || ekg::input::released() || ekg::input::motion()) {
        auto interact {ekg::interact()};
        auto &rect = (this->data->widget() = this->layout + *this->parent);
        this->flag.hovered = ekg::rect_collide_vec(rect, interact) && (this->data->get_parent_id() == 0 || ekg::rect_collide_vec(*this->parent, interact));
    }
}

void ekg::ui::abstract_widget::on_event(SDL_Event &sdl_event) {

}

void ekg::ui::abstract_widget::on_post_event(SDL_Event &sdl_event) {
    this->flag.hovered = false;
}

void ekg::ui::abstract_widget::on_update() {

}

void ekg::ui::abstract_widget::on_draw_refresh() {

}
