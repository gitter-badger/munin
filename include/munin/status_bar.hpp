#pragma once

#include "munin/basic_component.hpp"

namespace munin {

class MUNIN_EXPORT status_bar : public basic_component
{
public:
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    status_bar();
    
    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~status_bar() override;

private:
    //* =====================================================================
    /// \brief Called by get_preferred_size().  Derived classes must override
    /// this function in order to get the size of the component in a custom
    /// manner.
    //* =====================================================================
    terminalpp::extent do_get_preferred_size() const override;

    //* =====================================================================
    /// \brief Called by draw().  Derived classes must override this function
    /// in order to draw onto the passed context.  A component must only draw
    /// the part of itself specified by the region.
    ///
    /// \param context the context on which the component should draw itself.
    /// \param region the region relative to this component's origin that
    /// should be drawn.
    //* =====================================================================
    void do_draw(
        render_context &context,
        terminalpp::rectangle const &region) const override;

    struct impl;
    std::unique_ptr<impl> pimpl_;
};

//* =========================================================================
/// \brief Creates a new status bar
//* =========================================================================
MUNIN_EXPORT
std::shared_ptr<status_bar> make_status_bar();

}
