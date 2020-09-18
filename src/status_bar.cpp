#include "munin/status_bar.hpp"
#include <terminalpp/element.hpp>
#include <terminalpp/algorithm/for_each_in_region.hpp>
#include <munin/render_context.hpp>

namespace munin {

// ==========================================================================
// STATUS_BAR::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct status_bar::impl
{
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
status_bar::status_bar()
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
status_bar::~status_bar() = default;

// ==========================================================================
// DO_GET_PREFERRED_SIZE
// ==========================================================================
terminalpp::extent status_bar::do_get_preferred_size() const
{
    return {};
}

// ==========================================================================
// DO_DRAW
// ==========================================================================
void status_bar::do_draw(
    render_context &ctx,
    terminalpp::rectangle const &region) const
{
    terminalpp::for_each_in_region(
        ctx,
        region,
        [](terminalpp::element &elem,
           terminalpp::coordinate_type column,
           terminalpp::coordinate_type row)
        {
            elem = ' ';
        });
}

// ==========================================================================
// MAKE_STATUS_BAR
// ==========================================================================
std::shared_ptr<status_bar> make_status_bar()
{
    return std::make_shared<status_bar>();
}

}
