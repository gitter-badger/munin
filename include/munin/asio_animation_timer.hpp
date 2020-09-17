#pragma once
#include "munin/animation_timer.hpp"
#include <boost/asio/io_context.hpp>

namespace munin {

class asio_animation_timer : public animation_timer
{
public:
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    asio_animation_timer(boost::asio::io_context &ctx);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~asio_animation_timer() override;

private:
    //* =====================================================================
    /// \brief Schedules a function to be called in an amount of time.
    /// This is used to schedule the requested component redraws.
    //* =====================================================================
    void do_call_function_in(
        std::function<void ()> const &fn,
        std::chrono::milliseconds delay) override;

    //* =====================================================================
    /// \brief Returns the current time
    //* =====================================================================
    std::chrono::steady_clock::time_point do_now() const override;

    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}