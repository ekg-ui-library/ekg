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
 * @VokeGpu 2023 all rights reserved.
 */

#ifndef EKG_CORE_H
#define EKG_CORE_H

#include "ekg/ui/abstract/ui_abstract_widget.hpp"
#include "ekg/service/handler.hpp"
#include "ekg/draw/font.hpp"
#include "ekg/service/theme.hpp"
#include "ekg/service/input.hpp"
#include "ekg/core/feature.hpp"
#include "ekg/service/layout.hpp"
#include "ekg/util/util_ui.hpp"

namespace ekg {
    /* The swap maps for prevent copies of hashes and vector. */
    struct swap {
        static ekg::stack collect;
        static ekg::stack back;
        static ekg::stack front;
        static std::vector<ekg::ui::abstract_widget*> buffer;

        static void refresh();
    };

    /* The main runtime for run ekg. */
    class runtime : public ekg::feature {
    private:
        SDL_Window* root {};

        /* Widget env lists and maps for tasks. */

        std::vector<ekg::ui::abstract_widget*> loaded_widget_list {};
        std::vector<ekg::ui::abstract*> loaded_interface_list {};

        std::vector<ekg::ui::abstract_widget*> loaded_widget_reload_list {};
        std::vector<ekg::ui::abstract_widget*> loaded_widget_refresh_list {};
        std::vector<ekg::ui::abstract_widget*> loaded_widget_scissor_list {};
        std::vector<ekg::ui::abstract_widget*> loaded_widget_sync_layou_list {};

        std::map<int32_t, ekg::ui::abstract_widget*> widget_map {};
        std::map<int32_t, ekg::ui::abstract> interface_map {};
        std::map<int32_t, bool> processed_widget_map {};

        /* Core services and instances. */

        ekg::ui::abstract_widget* current_bind_group {};
        ekg::gpu::allocator allocator {};

        ekg::draw::font_renderer f_renderer_small {};
        ekg::draw::font_renderer f_renderer_normal {};
        ekg::draw::font_renderer f_renderer_big {};

        ekg::service::input input_service {};
        ekg::service::theme theme_service {};
        ekg::service::handler handler_service {};
        ekg::service::layout layout_service {};

        /* Tokens for use in creation of elements. */

        int32_t token_id {},
                widget_id_focused {},
                prev_widget_id_focused {},
                swap_widget_id_focused {},
                widget_id_pressed_focused {},
                widget_id_released_focused {};
        
        /* Security methods of ekg. */

        void prepare_tasks();
        void prepare_ui_env();
    public:
        void set_root(SDL_Window* sdl_win_root);
        SDL_Window* get_root();

        ekg::gpu::allocator &get_gpu_allocator();
        ekg::draw::font_renderer &get_f_renderer_small();
        ekg::draw::font_renderer &get_f_renderer_normal();
        ekg::draw::font_renderer &get_f_renderer_big();

        ekg::service::input &get_service_input();
        ekg::service::theme &get_service_theme();
        ekg::service::handler &get_service_handler();
        ekg::service::layout &get_service_layout();
        ekg::ui::abstract_widget* get_fast_widget_by_id(int32_t id);

        void gen_widget(ekg::ui::abstract* ui);
        void do_task_reload(ekg::ui::abstract_widget* widget);
        void do_task_scissor(ekg::ui::abstract_widget* widget);
        void do_task_synclayout(ekg::ui::abstract_widget* widget);
        void end_group_flag();

        void init();
        void quit();

        void process_event(SDL_Event &sdl_event);
        void process_update();
        void process_render();
    };
};

#endif