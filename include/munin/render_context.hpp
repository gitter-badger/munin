#pragma once
#include "munin/animation_timer.hpp"
#include "munin/render_surface.hpp"

namespace munin {

//* =========================================================================
/// \brief A container for tools that may be required when rendering a
/// component onto a surface.
//* =========================================================================
struct MUNIN_EXPORT render_context
{
public:
    using size_type = render_surface::size_type;

    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    render_context(render_surface& surface, animation_timer& timer);

    //* =====================================================================
    /// \brief Returns true if the surface is known to support unicode.
    /// characters.  Attempting to render unicode on surfaces that do not
    /// support unicode may have unexpected results.
    //* =====================================================================
    bool supports_unicode() const;

    //* =====================================================================
    /// \brief Offsets the canvas by a certain amount, causing it to become
    /// a view with the offset location as a basis.
    //* =====================================================================
    void offset_by(terminalpp::extent offset);

    //* =====================================================================
    /// \brief Returns the size of the canvas.
    //* =====================================================================
    terminalpp::extent size() const;

    //* =====================================================================
    /// \brief A subscript operator into a column
    //* =====================================================================
    render_surface::column_proxy operator[](terminalpp::coordinate_type column);

    //* =====================================================================
    /// \brief Returns the current time in order to allow animations.
    //* =====================================================================
    std::chrono::steady_clock::time_point now() const;

private:
    render_surface& surface_;
    animation_timer& animation_timer_;
};

}