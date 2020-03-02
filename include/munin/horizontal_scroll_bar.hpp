#pragma once

#include "munin/composite_component.hpp"

namespace munin {

//* =========================================================================
/// \brief A class that models a horizontal scroll bar
//* =========================================================================
class MUNIN_EXPORT horizontal_scroll_bar : public munin::composite_component
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    horizontal_scroll_bar();

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~horizontal_scroll_bar() override;

protected :
    //* =====================================================================
    /// \brief Called by event().  Derived classes must override this
    /// function in order to handle events in a custom manner.
    //* =====================================================================
    void do_event(boost::any const &ev) override;

private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

//* =========================================================================
/// \brief Returns a newly created horizontal scroll bar.
//* =========================================================================
MUNIN_EXPORT
std::shared_ptr<horizontal_scroll_bar> make_horizontal_scroll_bar();

}
