#include "munin/animation_timer.hpp"

namespace munin {

null_animation_timer default_animation_timer;

// ==========================================================================
// REDRAW_COMPONENT_IN
// ==========================================================================
void animation_timer::redraw_component_in(
        std::shared_ptr<component> const &comp,
        terminalpp::rectangle bounds,
        std::chrono::milliseconds delay)
{
    do_call_function_in(
        [wcomp = std::weak_ptr<component>(comp), bounds]()
        {
            auto comp = wcomp.lock();

            if (comp)
            {
                comp->on_redraw({{bounds.origin, bounds.size}});
            }
        },
        delay);
}

// ==========================================================================
// NOW
// ==========================================================================
std::chrono::steady_clock::time_point animation_timer::now() const
{
    return do_now();
}

}