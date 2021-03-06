#pragma once

namespace munin {

//* =========================================================================
/// \brief A class that describes the capabilities of a render surface.
//* =========================================================================
struct render_surface_capabilities
{
    virtual ~render_surface_capabilities() = default;

    //* =====================================================================
    /// \brief Returns true if the surface is known to support unicode.
    /// characters.  Attempting to render unicode on surfaces that do not
    /// support unicode may have unexpected results.
    //* =====================================================================
    virtual bool supports_unicode() const = 0;
};

}