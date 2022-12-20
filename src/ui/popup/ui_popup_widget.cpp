<<<<<<< HEAD
/*
 * VOKEGPU EKG LICENSE
 *
 * Respect ekg license policy terms, please take a time and read it.
 * 1- Any "skidd" or "stole" is not allowed.
 * 2- Forks and pull requests should follow the license policy terms.
 * 3- For commercial use, do not sell without give credit to vokegpu ekg.
 * 4- For ekg users and users-programmer, we do not care, free to use in anything (utility, hacking, cheat, game, software).
 * 5- Malware, rat and others virus. We do not care.
 * 6- Do not modify this license under any instance.
 *
 * @VokeGpu 2022 all rights reserved.
 */

#include "ekg/ui/popup/ui_popup_widget.hpp"

void ekg::ui::popup_widget::destroy() {
    abstract_widget::destroy();

    
}

void ekg::ui::popup_widget::on_reload() {
    abstract_widget::on_reload();
}

void ekg::ui::popup_widget::on_pre_event(SDL_Event &sdl_event) {
    abstract_widget::on_pre_event(sdl_event);
}

void ekg::ui::popup_widget::on_event(SDL_Event &sdl_event) {
    abstract_widget::on_event(sdl_event);
}

void ekg::ui::popup_widget::on_post_event(SDL_Event &sdl_event) {
    abstract_widget::on_post_event(sdl_event);
=======
#include "ekg/ui/popup/ui_popup_widget.hpp"
#include "ekg/ui/popup/ui_popup.hpp"
#include "ekg/ekg.hpp"

void ekg::ui::popup_widget::destroy() {

}

void ekg::ui::popup_widget::on_reload() {
	auto ui {(ekg::ui::popup*) this->data};
	auto &theme {ekg::theme()};
	auto &font_renderer {ekg::f_renderer(ui->get_font_size())};
}

void ekg::ui::popup_widget::on_pre_event(SDL_Event &sdl_event) {

}

void ekg::ui::popup_widget::on_event(SDL_Event &sdl_event) {

}

void ekg::ui::popup_widget::on_post_event(SDL_Event &sdl_event) {

>>>>>>> ed6d07b (Refactored base structure UI methods.)
}

void ekg::ui::popup_widget::on_update() {

}

void ekg::ui::popup_widget::on_draw_refresh() {
<<<<<<< HEAD
=======

>>>>>>> ed6d07b (Refactored base structure UI methods.)
}