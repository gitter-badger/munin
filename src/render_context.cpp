#include "munin/render_context.hpp"

namespace munin {

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
render_context::render_context(render_surface& surface, animation_timer& timer)
  : surface_(surface),
    animation_timer_(timer)
{
}

// ==========================================================================
// SUPPORTS_UNICODE
// ==========================================================================
bool render_context::supports_unicode() const
{
    return surface_.supports_unicode();
}

// ==========================================================================
// OFFSET_BY
// ==========================================================================
void render_context::offset_by(terminalpp::extent offset)
{
    surface_.offset_by(offset);
}

// ==========================================================================
// SIZE
// ==========================================================================
terminalpp::extent render_context::size() const
{
    return surface_.size();
}

// ==========================================================================
// OPERATOR[]
// ==========================================================================
render_surface::column_proxy render_context::operator[](
    terminalpp::coordinate_type column)
{
    return surface_[column];
}

// ==========================================================================
// NOW
// ==========================================================================
std::chrono::steady_clock::time_point render_context::now() const
{
    return animation_timer_.now();
}

}
