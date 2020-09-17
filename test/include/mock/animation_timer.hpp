#pragma once
#include <munin/animation_timer.hpp>
#include <gmock/gmock.h>

class mock_animation_timer : public munin::animation_timer
{
public:
    //* =====================================================================
    /// \brief Schedules a function to be called in an amount of time.
    /// This is used to schedule the requested component redraws.
    //* =====================================================================
    MOCK_METHOD2(
        do_call_function_at, 
        void (
            std::function<void ()> const &,
            std::chrono::steady_clock::time_point));

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    MOCK_CONST_METHOD0(do_now, std::chrono::steady_clock::time_point ());
};
