#pragma once
#include <munin/component.hpp>
#include <chrono>

namespace munin {

//* =========================================================================
/// \brief An injectable interface into a render surface that allows
/// timing of animated components.
//* =========================================================================
struct MUNIN_EXPORT animation_timer
{
    //* =====================================================================
    /// \brief Schedules a repaint of the passed component
    //* =====================================================================
    virtual void repaint_component_in(
        std::shared_ptr<component> const &comp,
        terminalpp::rectangle bounds,
        std::chrono::milliseconds delay) = 0;

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    virtual std::chrono::steady_clock::time_point now() const = 0;
};

//* =========================================================================
/// \brief An animation timer that does nothing.
//* =========================================================================
class MUNIN_EXPORT null_animation_timer
  : public animation_timer
{
public:
    //* =====================================================================
    /// \brief Schedules a repaint of the passed component
    //* =====================================================================
    void repaint_component_in(
        std::shared_ptr<component> const &comp,
        terminalpp::rectangle const bounds,
        std::chrono::milliseconds const delay) override
    {
    }

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    std::chrono::steady_clock::time_point now() const override
    {
        return {};
    }
};

extern MUNIN_EXPORT null_animation_timer default_animation_timer;

}
