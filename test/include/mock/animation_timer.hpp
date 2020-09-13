#pragma once
#include <munin/animation_timer.hpp>
#include <gmock/gmock.h>

class mock_animation_timer : public munin::animation_timer
{
public:
    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    MOCK_CONST_METHOD0(now, std::chrono::steady_clock::time_point ());
};
