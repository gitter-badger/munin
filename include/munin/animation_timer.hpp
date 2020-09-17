#pragma once
#include <munin/component.hpp>
#include <chrono>

namespace munin {

//* =========================================================================
/// \brief An injectable interface into a render surface that allows
/// timing of animated components.
//* =========================================================================
class MUNIN_EXPORT animation_timer
{
public:
    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    virtual ~animation_timer() = default;

    //* =====================================================================
    /// \brief Schedules a redraw of the passed component
    //* =====================================================================
    void redraw_component_at(
        std::shared_ptr<component> const &comp,
        terminalpp::rectangle bounds,
        std::chrono::steady_clock::time_point execution_time);

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    std::chrono::steady_clock::time_point now() const;

private:
    //* =====================================================================
    /// \brief Schedules a function to be called at a certain time.
    /// This is used to schedule the requested component redraws.
    //* =====================================================================
    virtual void do_call_function_at(
        std::function<void ()> const &fn,
        std::chrono::steady_clock::time_point execution_time) = 0;

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    virtual std::chrono::steady_clock::time_point do_now() const = 0;
};

//* =========================================================================
/// \brief An animation timer that does nothing.
//* =========================================================================
class MUNIN_EXPORT null_animation_timer
  : public animation_timer
{
private:
    //* =====================================================================
    /// \brief Schedules a function to be called at a certain time.
    /// This is used to schedule the requested component redraws.
    //* =====================================================================
    void do_call_function_at(
        std::function<void ()> const &fn,
        std::chrono::steady_clock::time_point const execution_time) override
    {
    }

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    virtual std::chrono::steady_clock::time_point do_now() const override
    {
        return {};
    }
};

extern MUNIN_EXPORT null_animation_timer default_animation_timer;

}
