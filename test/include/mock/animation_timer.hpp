#pragma once
#include <munin/animation_timer.hpp>
#include <gmock/gmock.h>

class mock_animation_timer : public munin::animation_timer
{
public:
    //* =====================================================================
    /// \brief Schedules a repaint of the passed component
    //* =====================================================================
    MOCK_METHOD3(
        repaint_component_in, 
        void (
            std::shared_ptr<munin::component> const &,
            terminalpp::rectangle,
            std::chrono::milliseconds));

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    MOCK_CONST_METHOD0(now, std::chrono::steady_clock::time_point ());
};
