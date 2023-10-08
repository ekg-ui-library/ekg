/*
 * MIT License
 *
 * Copyright (c) 2022-2023 Rina Wilk / vokegpu@gmail.com
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

#include "ekg/ui/listbox/ui_listbox_widget.hpp"
#include "ekg/ui/listbox/ui_listbox.hpp"

void ekg::ui::listbox_widget::on_reload() {
    auto p_ui {(ekg::ui::listbox*) this->p_data};
    auto &rect {this->get_abs_rect()};
    auto &ui_item {p_ui->item()};

    for (uint64_t it {}; it < ui_item.size(); it++) {
        ekg::item &item {ui_item.at(it)};
    }
}

void ekg::ui::listbox_widget::on_update() {

}

void ekg::ui::listbox_widget::on_draw_refresh() {
}