#pragma once

#include "munin/basic_component.hpp"
#include <terminalpp/element.hpp>
#include <functional>

namespace munin {

//* =========================================================================
/// \brief A class that models a box that is always completely filled with
/// a given element.
//* =========================================================================
class MUNIN_EXPORT filled_box : public munin::basic_component
{
public :
    using fill_function_type = terminalpp::element (render_context &);

    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    explicit filled_box(terminalpp::element const &element = ' ');

    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    explicit filled_box(std::function<fill_function_type> fill_function);
    
    //* =====================================================================
    /// \brief Sets the preferred size of this box.  The default is (1,1).
    //* =====================================================================
    void set_preferred_size(terminalpp::extent preferred_size);

protected :
    //* =====================================================================
    /// \brief Returns true if it is allowed for the component to receive
    /// focus, false otherwise.  This is used by the focus functions of
    /// basic_component.  By default, a component may receive focus at all
    /// times.  Override this function to specify different behaviour in your
    /// component.
    //* =====================================================================
    bool do_can_receive_focus() const override;

    //* =====================================================================
    /// \brief Called by get_preferred_size().  Derived classes must override
    /// this function in order to get the size of the component in a custom
    /// manner.
    //* =====================================================================
    terminalpp::extent do_get_preferred_size() const override;

    //* =====================================================================
    /// \brief Called by draw().  Derived classes must override this function
    /// in order to draw onto the passed canvas.  A component must only draw
    /// the part of itself specified by the region.
    ///
    /// \param context the context on which the component should draw itself.
    /// \param region the region relative to this component's origin that
    /// should be drawn.
    //* =====================================================================
    void do_draw(
        render_context &context,
        terminalpp::rectangle const &region) const override;

    //* =====================================================================
    /// \brief Called by to_json().  Derived classes must override this
    /// function in order to add additional data about their implementation
    /// in a custom manner.
    //* =====================================================================
    nlohmann::json do_to_json() const override;

private :
    std::function<fill_function_type> fill_function_;
    terminalpp::extent preferred_size_;
};

//* =========================================================================
/// \brief Returns a newly created filled box.
//* =========================================================================
MUNIN_EXPORT
std::shared_ptr<filled_box> make_fill(terminalpp::element const &fill);

//* =========================================================================
/// \brief Returns a newly created filled box.
//* =========================================================================
MUNIN_EXPORT
std::shared_ptr<filled_box> make_fill(
    std::function<filled_box::fill_function_type> fill_function);

}
