#include "munin/horizontal_scroll_bar.hpp"

namespace munin {

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
horizontal_scroll_bar::horizontal_scroll_bar()
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
horizontal_scroll_bar::~horizontal_scroll_bar()
{
}

// ==========================================================================
// DO_EVENT
// ==========================================================================
void horizontal_scroll_bar::do_event(boost::any const &ev)
{
}

// ==========================================================================
// MAKE_HORIZONTAL_SCROLL_BAR
// ==========================================================================
std::shared_ptr<horizontal_scroll_bar> make_horizontal_scroll_bar()
{
    return std::make_shared<horizontal_scroll_bar>();
}

}


