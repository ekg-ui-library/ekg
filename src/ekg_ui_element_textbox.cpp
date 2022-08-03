/**
 * EKG-LICENSE - this software/library LICENSE can not be modified in any instance.
 *
 * --
 * ANY NON-OFFICIAL MODIFICATION IS A CRIME.
 * DO NOT SELL THIS CODE SOFTWARE, FOR USE EKG IN A COMMERCIAL PRODUCT ADD EKG-LICENSE TO PROJECT,
 * RESPECT THE COPYRIGHT TERMS OF EKG, NO SELL WITHOUT EKG-LICENSE (IT IS A CRIME).
 * DO NOT FORK THE PROJECT SOURCE WITHOUT EKG-LICENSE.
 *
 * END OF EKG-LICENSE.
 **/
#include <ekg/ekg.hpp>

ekg_textbox::ekg_textbox() {
    this->type = ekg::ui::TEXTBOX;
}

ekg_textbox::~ekg_textbox() {

}

void ekg_textbox::on_killed() {
    ekg_element::on_killed();
}

void ekg_textbox::on_sync() {
    ekg_element::on_sync();

    this->min_text_width = ekgfont::get_text_width("h");
    this->min_text_height = ekgfont::get_text_height("h");

    this->rect.w = this->rect.w < this->min_text_width ? this->min_text_width : this->rect.w;
    this->rect.h = this->rect.h < this->min_text_height ? this->min_text_height : this->rect.h;

    the_ekg_core->dispatch_todo_event(ekgutil::action::REFRESH);
}

void ekg_textbox::on_pre_event_update(SDL_Event &sdl_event) {
    ekg_element::on_pre_event_update(sdl_event);

    float mx = 0;
    float my = 0;

    if (ekgapi::motion(sdl_event, mx, my)) {
        ekgapi::set_direct(this->flag.over, this->is_hovering(mx, my));
    }
}

void ekg_textbox::on_event(SDL_Event &sdl_event) {
    ekg_element::on_event(sdl_event);

    float mx = 0;
    float my = 0;

    if (ekgapi::motion(sdl_event, mx, my)) {
        ekgapi::set(this->flag.highlight, this->flag.over);
    } else if (ekgapi::input_down_left(sdl_event, mx, my)) {
        ekgapi::set(this->flag.activy, this->flag.over);
        ekgapi::set(this->flag.focused, this->flag.over);
    } else if (ekgapi::input_up_left(sdl_event, mx, my)) {
        ekgapi::set(this->flag.activy, false);

        if (this->flag.focused && !this->flag.over) {
        	ekgapi::set(this->flag.focused, false);
        }
    }
}

void ekg_textbox::on_post_event_update(SDL_Event &sdl_event) {
    ekg_element::on_post_event_update(sdl_event);

    float mx = 0;
    float my = 0;

    if (ekgapi::motion(sdl_event, mx, my)) {
        ekgapi::set_direct(this->flag.over, false);
    }
}

void ekg_textbox::on_draw_refresh() {
    ekg_element::on_draw_refresh();
    ekggpu::rectangle(this->rect, ekg::theme().button_background);

    if (this->flag.highlight) {
        ekggpu::rectangle(this->rect, ekg::theme().button_highlight);
    }

    if (this->flag.focused) {
        ekggpu::rectangle(this->rect, ekg::theme().button_activy);
    }

    ekgfont::render(this->text, this->rect.x + this->text_offset_x, this->rect.y + this->text_offset_y, ekg::theme().string_enabled_color);
}

void ekg_textbox::set_text(const std::string &string) {
    if (this->text != string) {
        this->text = string;
        this->on_sync();
    }
}

std::string ekg_textbox::get_text() {
    return this->text;
}

void ekg_textbox::set_size(float width, float height) {
    ekg_element::set_size(width, height);

    if (this->rect.w != width || this->rect.h != height) {
        this->rect.w = width;
        this->rect.h = height;

        this->on_sync();
    }
}

void ekg_textbox::set_pos(float x, float y) {
    ekg_element::set_pos(x, y);
}

void ekg_textbox::set_width(float width) {
    this->set_size(width, this->rect.h);
}

void ekg_textbox::set_height(float height) {
    this->set_size(this->rect.x, height);
}

float ekg_textbox::get_min_text_width() {
    return this->min_text_height;
}

float ekg_textbox::get_min_text_height() {
    return this->min_text_height;
}